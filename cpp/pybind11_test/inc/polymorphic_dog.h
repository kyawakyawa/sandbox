#pragma once
#include <string>

#include "polymorphic_pet.h"

class PolymorphicDog : public PolymorphicPet {
public:
  PolymorphicDog();
  std::string Bark() const;
};
