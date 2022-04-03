#pragma once

#include <string>

class Pet {
public:
  enum Kind { Dog = 0, Cat };
  struct Attributes {
    float age = 0;
  };

  Pet(const std::string &name);
  Pet(const std::string &name, Kind type);
  void SetName(const std::string &name);
  const std::string &GetName(void) const;

  void Set(int age);
  void Set(const std::string &name);

  void SetType(const Kind type);
  Kind GetType() const;

private:
  std::string name_;
  // int age_;
  Kind type_;
  Attributes attr_;
};
