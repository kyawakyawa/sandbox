#include "dog.h"

Dog::Dog(const std::string& name) : Pet(name) {}

std::string Dog::Bark() const { return "woof!"; }
