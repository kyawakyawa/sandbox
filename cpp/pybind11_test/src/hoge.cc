#include "add.h"

// pybind11
#include "pybind11/pybind11.h"

PYBIND11_MODULE(hoge, m) {
  m.doc() = "hoge";  // optional module docstring

  m.def("add", &add, "A function that adds two numbers");
}
