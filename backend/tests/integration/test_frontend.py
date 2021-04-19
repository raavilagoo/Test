"""Test the functionality of protocols.frontend classes."""

from typing import List

import betterproto

import hypothesis as hp
import hypothesis.strategies as st

from ventserver.protocols import frontend
from ventserver.protocols.protobuf import mcu_pb as pb


example_messages = [
    pb.AlarmLimits(fio2=pb.Range(lower=21, upper=100)),
    pb.SensorMeasurements(paw=20),
    pb.CycleMeasurements(ve=300),
    pb.Parameters(fio2=80),
    pb.Ping(id=256),
    pb.Announcement(announcement=b'hello!'),
]


@hp.given(input_bytes=st.binary(max_size=300))
@hp.example(input_bytes=b'')
@hp.example(input_bytes=bytes([1 for i in range(260)]))
def test_frontend_receive_random(input_bytes: bytes) -> None:
    """Test ReceiveFilter behavior with random inputs."""
    receiver = frontend.ReceiveFilter()
    receiver.input(input_bytes)
    for _ in receiver.output_all():  # no exceptions should be raised
        pass


@hp.given(message_sequence=st.lists(
    st.sampled_from(example_messages), max_size=300
))
def test_frontend_roundtrip(
        message_sequence: List[betterproto.Message]
) -> None:
    """Test ReceiveFilter+SendFilter round-tripping."""
    sender = frontend.SendFilter()
    receiver = frontend.ReceiveFilter()
    for message in message_sequence:
        sender.input(message)
        receiver.input(sender.output())
        assert receiver.output() == message
