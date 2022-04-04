#include "array.h"

namespace py = pybind11;

template <typename T>
std::string to_string(T *ptr, size_t n) {
  std::ostringstream ss;

  ss << "(";
  for (size_t i = 0; i < n; i++) {
    ss << ptr[i];
    if (i != (n - 1)) {
      ss << ", ";
    }
  }
  ss << ")";

  return ss.str();
}

void show_shape(const pybind11::array &a) {
  py::print("shape = ", to_string(a.shape(), a.ndim()));
}

template <typename T>
void fill_two_(pybind11::array &a) {
  py::buffer_info b = a.request();

  T *ptr = reinterpret_cast<T *>(b.ptr);

  ssize_t n = 1;
  for (ssize_t i = 0; i < a.ndim(); ++i) {
    n *= a.shape()[i];
  }

  // TODO(anyone): Consider strides

  for (ssize_t i = 0; i < n; ++i) {
    ptr[i] = static_cast<T>(2);
  }
}

void fill_two(pybind11::array &a) {
  auto f32ty = py::dtype::of<float>();
  auto f64ty = py::dtype::of<double>();
  if (f32ty.is(a.dtype())) {
    fill_two_<float>(a);
  } else if (f64ty.is(a.dtype())) {
    fill_two_<double>(a);
  }
}
