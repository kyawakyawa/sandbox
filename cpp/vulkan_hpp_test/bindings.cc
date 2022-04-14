#include "app.h"
#include "buffer.h"
#include "device.h"
#include "instance.h"
// pybind11
#include "pybind11/pybind11.h"

namespace py = pybind11;

PYBIND11_MODULE(vulkan_hpp_test, m) {
  m.doc() = "vulkan_hpp_test";

  // py::class_<vulkan_hpp_test::Instance> instance(m, "Instance");

  // instance.def(py::init<>());

  py::class_<vulkan_hpp_test::App> app(m, "App");
  py::class_<vulkan_hpp_test::Buffer, std::shared_ptr<vulkan_hpp_test::Buffer>>
      buffer(m, "Buffer");

  using namespace pybind11::literals;
  app.def(py::init<>())
      .def("get_num_devices", &vulkan_hpp_test::App::GetNumDevices,
           "A function that get number of devices")
      .def("create_buffer", &vulkan_hpp_test::App::CreateBuffer, "device_id"_a,
           "size_byte"_a, "A function that create buffer on device");

  buffer.def(py::init<>());
}
