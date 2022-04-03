#include "add.h"
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
}
