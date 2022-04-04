"""hoge"""
from __future__ import annotations
import hoge
import typing
import numpy
_Shape = typing.Tuple[int, ...]

__all__ = [
    "Dog",
    "Pet",
    "PolymorphicDog",
    "PolymorphicPet",
    "What",
    "add",
    "fill_two",
    "pet_store",
    "pet_store2",
    "show_shape"
]


class Pet():
    class Attributes():
        def __init__(self) -> None: ...
        @property
        def age(self) -> float:
            """
            :type: float
            """
        @age.setter
        def age(self, arg0: float) -> None:
            pass
        pass
    class Kind():
        """
        Members:

          Dog

          Cat
        """
        def __eq__(self, other: object) -> bool: ...
        def __getstate__(self) -> int: ...
        def __hash__(self) -> int: ...
        def __index__(self) -> int: ...
        def __init__(self, value: int) -> None: ...
        def __int__(self) -> int: ...
        def __ne__(self, other: object) -> bool: ...
        def __repr__(self) -> str: ...
        def __setstate__(self, state: int) -> None: ...
        @property
        def name(self) -> str:
            """
            :type: str
            """
        @property
        def value(self) -> int:
            """
            :type: int
            """
        Cat: hoge.Pet.Kind # value = <Kind.Cat: 1>
        Dog: hoge.Pet.Kind # value = <Kind.Dog: 0>
        __members__: dict # value = {'Dog': <Kind.Dog: 0>, 'Cat': <Kind.Cat: 1>}
        pass
    def GetName(self) -> str: ...
    @typing.overload
    def Set(self, arg0: int) -> None: 
        """
        Set the pet's age

        Set the pet's name
        """
    @typing.overload
    def Set(self, arg0: str) -> None: ...
    def SetName(self, arg0: str) -> None: ...
    @staticmethod
    @typing.overload
    def __init__(*args, **kwargs) -> typing.Any: ...
    @typing.overload
    def __init__(self, name: str) -> None: ...
    def __repr__(self) -> str: ...
    @property
    def name(self) -> str:
        """
        :type: str
        """
    @name.setter
    def name(self, arg1: str) -> None:
        pass
    @property
    def type(self) -> Pet::Kind:
        """
        :type: Pet::Kind
        """
    @type.setter
    def type(self, arg1: Pet::Kind) -> None:
        pass
    Cat: hoge.Pet.Kind # value = <Kind.Cat: 1>
    Dog: hoge.Pet.Kind # value = <Kind.Dog: 0>
    pass
class Dog(Pet):
    def Bark(self) -> str: ...
    def __init__(self, name: str) -> None: ...
    pass
class PolymorphicPet():
    pass
class PolymorphicDog(PolymorphicPet):
    def Bark(self) -> str: ...
    def __init__(self) -> None: ...
    pass
def add(i: int = 1, j: int = 2) -> int:
    """
    A function that adds two numbers
    """
def fill_two(a: numpy.ndarray) -> None:
    pass
def pet_store() -> Pet:
    pass
def pet_store2() -> PolymorphicPet:
    pass
def show_shape(a: numpy.ndarray) -> None:
    pass
What = 'World'
