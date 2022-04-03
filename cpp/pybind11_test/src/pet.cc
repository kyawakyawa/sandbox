#include "pet.h"

Pet::Pet(const std::string& name) : name_(name) {
  attr_.age = 5;
  type_      = Kind::Dog;
}

Pet::Pet(const std::string& name, Kind type) : name_(name), type_(type) {}

void Pet::SetName(const std::string& name) { name_ = name; }
const std::string& Pet::GetName() const { return name_; }

void Pet::Set(int age) { attr_.age = age; }
void Pet::Set(const std::string& name) { name_ = name; }

void Pet::SetType(const Kind type) { type_ = type; }
Pet::Kind Pet::GetType() const { return type_; }
