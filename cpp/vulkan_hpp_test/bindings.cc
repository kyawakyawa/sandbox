#include "app.h"
#include "instance.h"
// pybind11
#include "pybind11/pybind11.h"

namespace py = pybind11;

PYBIND11_MODULE(vulkan_hpp_test, m) {
  m.doc() = "vulkan_hpp_test";

  // py::class_<vulkan_hpp_test::Instance> instance(m, "Instance");

  // instance.def(py::init<>());

  py::class_<vulkan_hpp_test::App> app(m, "App");

  using namespace pybind11::literals;
  app.def(py::init<>())
      .def("get_num_devices", &vulkan_hpp_test::App::GetNumDevices,
           "A function that get number of devices");
}
