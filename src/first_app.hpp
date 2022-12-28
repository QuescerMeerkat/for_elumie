#pragma once

#include "defines.hpp"
#include "zx_descriptors.hpp"
#include "zx_device.hpp"
#include "zx_game_object.hpp"
#include "zx_renderer.hpp"
#include "zx_window.hpp"
#include "zx_utils.hpp"
#include "world.hpp"

#include <memory>
#include <vector>

namespace zx {
class FirstApp {
 public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  FirstApp();
  ~FirstApp();

  FirstApp(const FirstApp &) = delete;
  FirstApp &operator=(const FirstApp &) = delete;

  void run();

 private:
  void loadGameObjects();

  ZxWindow zxWindow{WIDTH, HEIGHT, "Zenix"};
  ZxDevice zxDevice{zxWindow};
  ZxRenderer zxRenderer{zxWindow, zxDevice};

  // note: order of declarations matters
  std::unique_ptr<ZxDescriptorPool> globalPool{};
  ZxGameObject::Map gameObjects;

  std::vector<World> worlds;
};
}
