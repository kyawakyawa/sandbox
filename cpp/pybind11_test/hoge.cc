#include <memory>

#include "add.h"
#include "array.h"
#include "dog.h"
#include "pet.h"
#include "polymorphic_dog.h"
#include "polymorphic_pet.h"
#include "world.h"

// pybind11
#include "pybind11/pybind11.h"

namespace py = pybind11;

PYBIND11_MODULE(hoge, m) {
  m.doc() = "hoge";  // optional module docstring

  // Function
  using namespace pybind11::literals;
  m.def("add", &add, "i"_a = 1, "j"_a = 2, "A function that adds two numbers");

  // variable
  py::object world = py::cast(gWorld);
  m.attr("What")   = world;

  // class
  // 先に定義してからdefするとStubの生成が上手くいく
  // [参考]
  // https://pybind11.readthedocs.io/en/latest/advanced/misc.html#avoiding-c-types-in-docstrings
  py::class_<Pet> pet(m, "Pet");
  py::enum_<Pet::Kind> kind(pet, "Kind");
  py::class_<Pet::Attributes> attributes(pet, "Attributes");

  pet.def(py::init<const std::string &>(), "name"_a)
      .def(py::init<const std::string &, Pet::Kind>())
      .def("SetName", &Pet::SetName)
      .def("GetName", &Pet::GetName)
      .def("__repr__",
           [](const Pet &a) { return "hoge.Pet named '" + a.GetName() + "'>"; })
      .def_property("name", &Pet::GetName, &Pet::SetName)
      .def("Set", static_cast<void (Pet::*)(int)>(&Pet::Set),
           "Set the pet's age")
      .def("Set", static_cast<void (Pet::*)(const std::string &)>(&Pet::Set),
           "Set the pet's name")
      // For C++14
      // .def("Set", py::overload_cast<int>(&Pet::Set), "Set the pet's age")
      // .def("Set", py::overload_cast<const std::string &>(&Pet::Set), "Set the
      // pet's name");
      .def_property("type", &Pet::GetType, &Pet::SetType);

  kind.value("Dog", Pet::Kind::Dog)
      .value("Cat", Pet::Kind::Cat)
      .export_values();

  attributes.def(py::init<>()).def_readwrite("age", &Pet::Attributes::age);

  // Method 2: pass parent class_ object:
  py::class_<Dog>(m, "Dog", pet /* <- specify Python parent type */)
      .def(py::init<const std::string &>(), "name"_a)
      .def("Bark", &Dog::Bark);

  m.def("pet_store", []() { return std::unique_ptr<Pet>(new Dog("Molly")); });

  // Same binding code
  py::class_<PolymorphicPet>(m, "PolymorphicPet");
  py::class_<PolymorphicDog, PolymorphicPet>(m, "PolymorphicDog")
      .def(py::init<>())
      .def("Bark", &PolymorphicDog::Bark);

  // Again, return a base pointer to a derived instance
  m.def("pet_store2",
        []() { return std::unique_ptr<PolymorphicPet>(new PolymorphicDog); });

  // array
  m.def("show_shape", &show_shape, "a"_a);
  m.def("fill_two", &fill_two, "a"_a);
}
