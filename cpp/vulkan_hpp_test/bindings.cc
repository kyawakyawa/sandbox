#include "instance.h"
// pybind11
#include "pybind11/pybind11.h"

namespace py = pybind11;

PYBIND11_MODULE(vulkan_hpp_test, m) {
  m.doc() = "vulkan_hpp_test";

  py::class_<vulkan_hpp_test::Instance> instance(m, "Instance");

  instance.def(py::init<>());
}
