#pragma once

#include <string>

#include "pet.h"

struct Dog : Pet {
  Dog(const std::string &name);
  std::string Bark() const;
};
