"""vulkan_hpp_test"""
from __future__ import annotations
import vulkan_hpp_test
import typing
import numpy
_Shape = typing.Tuple[int, ...]

__all__ = [
    "App",
    "Buffer",
    "CpuBuffer"
]


class App():
    def __init__(self) -> None: ...
    def create_buffer(self, device_id: int, size_byte: int) -> Buffer: 
        """
        A function that create buffer on device
        """
    def create_cpu_buffer(self, size_byte: int) -> CpuBuffer: 
        """
        A function that create cpu buffer on device
        """
    def get_num_devices(self) -> int: 
        """
        A function that get number of devices
        """
    pass
class Buffer():
    def __init__(self) -> None: ...
    def to_cpu_buffer(self) -> CpuBuffer: 
        """
        A function this copy device buffer to cpu buffer
        """
    pass
class CpuBuffer():
    def __init__(self) -> None: ...
    def from_numpy(self, a: numpy.ndarray) -> bool: 
        """
        A function that receive numpy array
        """
    def to_device_buffer(self, device_id: int) -> Buffer: 
        """
        A function that create device buffer.
        """
    def to_numpy_f32(self) -> numpy.ndarray[numpy.float32]: 
        """
        A function that create numpy array.
        """
    pass
