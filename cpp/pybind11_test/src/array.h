#pragma once

#include <sstream>

#include "pybind11/numpy.h"

void show_shape(const pybind11::array &a);

void fill_two(pybind11::array &b);
