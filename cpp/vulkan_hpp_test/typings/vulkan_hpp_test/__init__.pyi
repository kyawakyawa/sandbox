"""vulkan_hpp_test"""
from __future__ import annotations
import vulkan_hpp_test
import typing

__all__ = [
    "App",
    "Buffer"
]


class App():
    def __init__(self) -> None: ...
    def create_buffer(self, device_id: int, size_byte: int) -> Buffer: 
        """
        A function that create buffer on device
        """
    def get_num_devices(self) -> int: 
        """
        A function that get number of devices
        """
    pass
class Buffer():
    def __init__(self) -> None: ...
    pass
