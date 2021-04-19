"""Integration of Trio I/O drivers with Sans-I/O protocols."""

import functools
import logging
import time
from typing import Callable, Optional, TypeVar, Tuple, List, Type

import trio
import betterproto

from ventserver.io.trio import channels as triochannels
from ventserver.io.trio import endpoints
from ventserver.io.trio import fileio
from ventserver.io.trio import websocket as websocket_io
from ventserver.protocols import server
from ventserver.protocols import file
from ventserver.protocols import exceptions
from ventserver.sansio import channels
from ventserver.sansio import protocols
from ventserver.sansio import streams


logger = logging.getLogger(__name__)


# Types


_InputEvent = TypeVar('_InputEvent')


# Protocol send outputs


async def send_all_serial(
        serial: endpoints.IOEndpoint[bytes, bytes],
        send_stream: protocols.Filter[_InputEvent, bytes]
) -> None:
    """Send all data in the stream to the serial I/O endpoint, once.

    This completes once all data has been sent, so backpressure can be applied
    by awaiting this function.
    """
    send_bytes = b''.join(send_stream.output_all())
    if send_bytes is None:
        return

    if not serial.is_open:
        logger.debug(
            'Discarding because serial I/O endpoint is not open: %s', send_bytes
        )
        return

    try:
        await serial.send(send_bytes)
    except BrokenPipeError:
        logger.warning('Serial connection was lost during send: %s', send_bytes)


async def send_all_websocket(
        websocket: endpoints.IOEndpoint[bytes, bytes],
        send_channel: protocols.Filter[_InputEvent, bytes]
) -> None:
    """Send all data in the stream to the websocket I/O endpoint, once.

    This completes once all data has been sent, so backpressure can be applied
    by awaiting this function.
    """
    for send_event in send_channel.output_all():
        if not websocket.is_open:
            logger.debug(
                'Discarding because websocket I/O endpoint is not open: %s',
                send_event
            )
            await trio.sleep(0)
            continue

        try:
            await websocket.send(send_event)
        except BrokenPipeError:
            logger.warning(
                'Websocket connection was lost during send: %s', send_event
            )
        except exceptions.ProtocolDataError as err:
            logger.warning(
                'Illegal data type: %s', err
            )


async def send_all_file(
        filehandler: fileio.Handler,
        write_channel:protocols.Filter[_InputEvent, file.StateData]
) -> None:
    """Saves protobuf data to files (prototype)"""

    for message in write_channel.output_all():
        filehandler.set_props(str(message.state_type), "wb")
        try:
            await filehandler.open()
            async with filehandler:
                await filehandler.send(message.data)
        except OSError as err:
            logger.error("Handler: %s", err)


async def process_protocol_send_output(
        protocol: server.Protocol,
        serial: Optional[endpoints.IOEndpoint[bytes, bytes]],
        websocket: Optional[endpoints.IOEndpoint[bytes, bytes]],
        filehandler: fileio.Handler
) -> None:
    """Process all send outputs from the protocol and write them to I/O, once.

    This completes once all data has been sent to both serial and websocket, so
    backpressure can be applied by awaiting this function and only using this
    function for getting send outputs.
    """
    # Consume all outputs into respective stream/channel
    serial_send = streams.BytearrayStream()
    websocket_send: channels.DequeChannel[bytes] = channels.DequeChannel()
    file_send: channels.DequeChannel[file.StateData] = channels.DequeChannel()
    for output in protocol.send.output_all():
        if output.serial_send is not None:
            serial_send.input(output.serial_send)
        if output.websocket_send is not None:
            websocket_send.input(output.websocket_send)
        if output.file_send is not None:
            file_send.input(output.file_send)
    # Write stream/channel to I/O
    async with trio.open_nursery() as nursery:
        if serial is not None:
            nursery.start_soon(send_all_serial, serial, serial_send)
        if websocket is not None:
            nursery.start_soon(send_all_websocket, websocket, websocket_send)
        if filehandler is not None:
            nursery.start_soon(send_all_file, filehandler, file_send)


# Protocol send


async def process_protocol_send(
        send_event: Optional[server.SendEvent], protocol: server.Protocol,
        serial: Optional[endpoints.IOEndpoint[bytes, bytes]],
        websocket: Optional[endpoints.IOEndpoint[bytes, bytes]],
        filehandler: fileio.Handler
) -> None:
    """Process a send event and write out any data to I/O endpoints.

    This completes once all data has been sent to the I/O endpoints, so
    backpressure can be applied by awaiting this function and only using this
    function for sending events to the protocol and thus to the I/O. This should
    only be called from within one task, to avoid issues synchronizing over a
    shared I/O device.
    """
    if not send_event:
        return
    protocol.send.input(send_event)
    await process_protocol_send_output(protocol, serial, websocket, filehandler)

# Receive frontend connection event


def receive_frontend_connection(
        protocol: server.Protocol,
        websocket: websocket_io.Driver
) -> None:
    """Process frontend connection status."""
    protocol.receive.input(
        server.FrontendConnectionEvent(
            last_connection_time=websocket.connection_time,
            is_frontend_connected=websocket.is_open
        )
    )


# Protocol Receive Outputs


async def process_protocol_receive_output(
        protocol: server.Protocol,
        channel: triochannels.TrioChannel[server.ReceiveOutputEvent]
) -> None:
    """Process all currently-available receive outputs from the protocol.

    Pushes all server protocol receive output events into the provided channel.
    Stops and returns after no more processing can be done by the server
    protocol from past inputs; this is equivalent to the condition that no more
    outputs are generated by the server protocol.
    It is the responsibility of the external caller to pass the server_send
    field of the received event back into the send direction so that the
    protocol can send any resulting outputs to serial and websocket before
    continuing to generate the next event.
    Note that there is backpressure from the channel - another task must be
    ready to consume the receive event emitted on the provided channel before
    this will continue processing.
    """
    while True:
        try:
            receive_output = protocol.receive.output()
        except Exception:
            logger.exception('Uncaught exception')
            raise

        if receive_output is None:
            break

        await channel.input(receive_output)


# Protocol receive

_ReceiveInputType = TypeVar("_ReceiveInputType")
_ReceiveOutputType = TypeVar("_ReceiveOutputType")

async def process_io_receive(
        io_endpoint: endpoints.IOEndpoint[
            _ReceiveInputType, _ReceiveOutputType
        ],
        protocol: server.Protocol,
        receive_event_maker: Callable[
            [_ReceiveOutputType, float], server.ReceiveEvent
        ],
        channel: triochannels.TrioChannel[server.ReceiveOutputEvent],
        push_endpoint: 'trio.MemorySendChannel[server.ReceiveOutputEvent]'
) -> None:
    """Process all receives from the I/O endpoint, forever.

    Pushes all server protocol receive output events emitted after each I/O
    receive into the provided channel, and then gets and processes the next I/O
    receive.
    It is the responsibility of the external caller to pass the server_send
    field of the received event back into the send direction so that the
    protocol can send any resulting outputs to serial and websocket before
    continuing to generate the next event.
    Note that there is backpressure from the channel - another task must be
    ready to consume the receive event emitted on the provided channel before
    this will continue processing.
    """
    async with push_endpoint:
        async for receive in io_endpoint.receive_all():
            protocol.receive.input(receive_event_maker(receive, time.time()))
            await process_protocol_receive_output(protocol, channel)


# Overall integration

async def process_io_persistently(
        io_endpoint: endpoints.IOEndpoint[
            _ReceiveInputType, _ReceiveOutputType
        ],
        protocol: server.Protocol,
        receive_event_maker: Callable[
            [_ReceiveOutputType, float], server.ReceiveEvent
        ],
        nursery: trio.Nursery,
        channel: triochannels.TrioChannel[server.ReceiveOutputEvent],
        push_endpoint: 'trio.MemorySendChannel[server.ReceiveOutputEvent]',
        reconnect_interval: float = 0.01
) -> None:
    """Process all traffic on the I/O endpoint and reconnect on broken pipes.

    Pushes all server protocol receive output events emitted after each I/O
    receive into the provided channel. It is the responsibility of the external
    caller to pass the server_send field of the received event back into the
    send direction so that the protocol can send any resulting outputs to serial
    and websocket before continuing to generate the next event.
    Note that there is backpressure from the channel - another task must be
    ready to consume the receive event emitted on the provided channel before
    this will continue processing.
    """
    async with push_endpoint:
        while True:
            await io_endpoint.persistently_open(nursery=nursery)
            if isinstance(io_endpoint, websocket_io.Driver):
                receive_frontend_connection(
                    protocol, io_endpoint
                )

            try:
                async with io_endpoint:
                    await process_io_receive(
                        io_endpoint, protocol, receive_event_maker,
                        channel, push_endpoint.clone()
                    )
            except BrokenPipeError:
                logger.warning(
                    'Lost I/O endpoint, reconnecting: %s', io_endpoint
                )
                if isinstance(io_endpoint, websocket_io.Driver):
                    receive_frontend_connection(
                        protocol, io_endpoint
                    )

                await trio.sleep(reconnect_interval)


async def process_clock(
        protocol: server.Protocol,
        channel: triochannels.TrioChannel[server.ReceiveOutputEvent],
        push_endpoint: 'trio.MemorySendChannel[server.ReceiveOutputEvent]',
        clock_update_interval: float = 0.001
) -> None:
    """Process all clock updates, forever."""
    async with push_endpoint:
        while True:
            protocol.receive.input(server.ReceiveEvent(time=time.time()))
            await process_protocol_receive_output(protocol, channel)
            await trio.sleep(clock_update_interval)


async def process_all(
        protocol: server.Protocol,
        serial: Optional[endpoints.IOEndpoint[bytes, bytes]],
        websocket: Optional[endpoints.IOEndpoint[bytes, bytes]],
        rotary_encoder: Optional[
            endpoints.IOEndpoint[bytes, Tuple[int, bytes]]
        ],
        channel: triochannels.TrioChannel[server.ReceiveOutputEvent],
        push_endpoint: 'trio.MemorySendChannel[server.ReceiveOutputEvent]'
) -> None:
    """Process all data asynchronously.

    Pushes all server protocol receive output events emitted after each I/O
    receive into the provided channel. It is the responsibility of the external
    caller to pass the server_send field of the received event back into the
    send direction so that the protocol can send any resulting outputs to serial
    and websocket before continuing to generate the next event.
    Note that there is backpressure from the channel - another task must be
    ready to consume the receive event emitted on the provided channel before
    this will continue processing.
    """
    async with push_endpoint:
        async with trio.open_nursery() as nursery:
            # Process all I/O devices with reconnection upon disconnect
            if serial is not None:
                nursery.start_soon(
                    # mypy only supports <= 5 args with trio-typing
                    functools.partial(
                        process_io_persistently, serial, protocol,
                        server.make_serial_receive, nursery, channel,
                        push_endpoint.clone()
                    )
                )
            if websocket is not None:
                nursery.start_soon(
                    # mypy only supports <= 5 args with trio-typing
                    functools.partial(
                        process_io_persistently, websocket, protocol,
                        server.make_websocket_receive, nursery, channel,
                        push_endpoint.clone()
                    )
                )
            if rotary_encoder is not None:
                nursery.start_soon(
                    # mypy only supports <= 5 args with trio-typing
                    functools.partial(
                        process_io_receive, rotary_encoder, protocol,
                        server.make_rotary_encoder_receive, channel,
                        push_endpoint.clone()
                    )
                )
            nursery.start_soon(
                process_clock, protocol, channel, push_endpoint.clone()
            )


async def load_file_states(
        states: List[Type[betterproto.Message]],
        protocol: server.Protocol,
        filehandler: fileio.Handler
) -> None:
    """Initialize state values from state store or default values."""
    for state in states:
        try:  # Handle fileio errors
            filehandler.set_props(state.__name__, "rb")
            await filehandler.open()
            async with filehandler:
                message = await filehandler.receive()
                logger.info("State initialized from file: %s", state.__name__)
                protocol.receive.input(
                    server.ReceiveEvent(
                        file_receive=file.StateData(
                            state_type=state.__name__, data=message
                        )
                    )
                )
        except OSError as err:
            logger.error(err)
        except exceptions.ProtocolDataError as err:
            logger.error(err)

    while True:
        output = protocol.receive.output()
        if not output:
            break
