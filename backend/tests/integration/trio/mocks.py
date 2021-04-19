"""Trio I/O with sans-I/O protocol demo, running on mock I/O."""

import logging
import functools

import trio

from tests.mocks.trio import endpoints

from ventserver.integration import _trio
from ventserver.io.trio import channels
from ventserver.io.trio import fileio
from ventserver.protocols import backend
from ventserver.protocols import server



logger = logging.getLogger()
handler = logging.StreamHandler()
formatter = logging.Formatter(
    '%(asctime)s %(name)-12s %(levelname)-8s %(message)s'
)
handler.setFormatter(formatter)
logger.addHandler(handler)
logger.setLevel(logging.INFO)


async def handle_receive_outputs(
        channel: channels.TrioChannel[server.ReceiveOutputEvent],
        pull_endpoint: 'trio.MemoryReceiveChannel[server.ReceiveOutputEvent]',
        protocol: server.Protocol,
        serial: endpoints.SerialDriver, websocket: endpoints.WebSocketDriver,
        filehandler: fileio.Handler
) -> None:
    """Consume the channel of server protocol receive output events."""
    async with pull_endpoint:
        while True:
            receive_output = await channel.output()
            await _trio.process_protocol_send(
                receive_output.server_send, protocol, serial,
                websocket, filehandler
            )


async def main() -> None:
    """Set up wiring between subsystems and process until completion."""
    # Sans-I/O Protocols
    protocol = server.Protocol()

    # I/O Endpoints
    serial = endpoints.SerialDriver()
    websocket = endpoints.WebSocketDriver()
    filehandler = fileio.Handler()

    # Server Receive Outputs
    channel: channels.TrioChannel[
        server.ReceiveOutputEvent
    ] = channels.TrioChannel()

    async with channel.pull_endpoint:
        async with trio.open_nursery() as nursery:
            nursery.start_soon(
                functools.partial(_trio.process_all,
                                  channel=channel,
                                  push_endpoint=channel.push_endpoint),
                protocol, serial, websocket, None
            )
            nursery.start_soon(
                functools.partial(
                    handle_receive_outputs, channel,
                    channel.pull_endpoint.clone(),
                ), protocol, serial, websocket, filehandler
            )

            for i in range(20):
                await trio.sleep(1)
                await _trio.process_protocol_send(
                    backend.Announcement(message=bytes(
                        'Quitting in {} s!' .format(20 - i - 1), 'utf-8'
                    )), protocol, serial, websocket, filehandler
                )
            logger.info('Finished, quitting!')
            nursery.cancel_scope.cancel()


if __name__ == '__main__':
    try:
        trio.run(main)
    except KeyboardInterrupt:
        logger.info('Quitting!')
