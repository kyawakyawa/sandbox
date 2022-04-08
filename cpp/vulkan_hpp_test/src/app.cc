#include "app.h"

namespace vulkan_hpp_test {

App::App() : instance_(new Instance) {
  // CreateDevices(&devices_);
}

App::~App() {
  devices_.clear();
  instance_.reset();
}

}  // namespace vulkan_hpp_test
