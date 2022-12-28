#pragma once

#include "defines.hpp"
#include "zx_game_object.hpp"
#include "zx_window.hpp"

namespace zx {
class KeyboardMovementController {
 public:
  struct KeyMappings {
    int moveLeft = GLFW_KEY_A;
    int moveRight = GLFW_KEY_D;
    int moveForward = GLFW_KEY_W;
    int moveBackward = GLFW_KEY_S;
    int moveUp = GLFW_KEY_Q;
    int moveDown = GLFW_KEY_E;
    int lookLeft = GLFW_KEY_LEFT;
    int lookRight = GLFW_KEY_RIGHT;
    int lookUp = GLFW_KEY_DOWN;
    int lookDown = GLFW_KEY_UP;
  };

  void moveInPlaneXZ(GLFWwindow* window, float dt, ZxGameObject& gameObject);

  KeyMappings keys{};
  float moveSpeed{3.f*8.f};
  float lookSpeed{1.5f};
};
}