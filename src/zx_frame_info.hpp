#pragma once

#include "defines.hpp"
#include "zx_camera.hpp"
#include "zx_game_object.hpp"

#include <vulkan/vulkan.h>

namespace zx {

struct GlobalUbo {
  glm::mat4 projection{1.f};
  glm::mat4 inverseProjection{1.f};
  glm::mat4 view{1.f};
  glm::mat4 inverseView{1.f};
  glm::vec3 cameraPosition{1.f};
  float dt;
};

struct FrameInfo {
  int frameIndex;
  float frameTime;
  VkCommandBuffer commandBuffer;
  ZxCamera &camera;
  VkDescriptorSet globalDescriptorSet;
  ZxGameObject::Map &gameObjects;
};
}
