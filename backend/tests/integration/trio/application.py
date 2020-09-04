"""Trio I/O with sans-I/O protocol demo, running application demo."""

import logging
import time

import trio

from ventserver.integration import _trio
from ventserver.io.trio import _serial
from ventserver.io.trio import channels
from ventserver.io.trio import websocket
from ventserver.protocols import server
from ventserver.protocols.protobuf import mcu_pb as pb


logger = logging.getLogger()
handler = logging.StreamHandler()
formatter = logging.Formatter(
    '%(asctime)s %(name)-12s %(levelname)-8s %(message)s'
)
handler.setFormatter(formatter)
logger.addHandler(handler)
logger.setLevel(logging.INFO)


async def main() -> None:
    """Set up wiring between subsystems and process until completion."""
    # Sans-I/O Protocols
    protocol = server.Protocol()

    # I/O Endpoints
    serial_endpoint = _serial.Driver()
    websocket_endpoint = websocket.Driver()

    # Server Receive Outputs
    channel: channels.TrioChannel[
        server.ReceiveOutputEvent
    ] = channels.TrioChannel()

    # Initialize State
    protocol.receive.backend.all_states[pb.Parameters] = pb.Parameters(
        pip=30, peep=10
    )

    report_interval = 0.03
    last_report_time = time.time()
    try:
        async with channel.push_endpoint:
            async with trio.open_nursery() as nursery:
                nursery.start_soon(
                    _trio.process_all, serial_endpoint, protocol,
                    websocket_endpoint, channel, channel.push_endpoint
                )

                while True:
                    receive_output = await channel.output()
                    if time.time() - last_report_time >= report_interval:
                        logger.info(protocol.receive.backend.all_states[
                            pb.SensorMeasurements
                        ])
                        last_report_time = time.time()
                    await _trio.process_protocol_send(
                        receive_output.server_send, protocol,
                        serial_endpoint, websocket_endpoint
                    )
                nursery.cancel_scope.cancel()
    except trio.EndOfChannel:
        logger.info('Finished, quitting!')


if __name__ == '__main__':
    try:
        trio.run(main)
    except KeyboardInterrupt:
        logger.info('Quitting!')