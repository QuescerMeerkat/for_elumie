#include "defines.hpp"
#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "zx_buffer.hpp"
#include "zx_camera.hpp"
#include "zx_game_object.hpp"
#include "zx_texture.hpp"
#include "zx_utils.hpp"
#include "systems/simple_render_system.hpp"
#include "systems/voxel_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <memory>
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>
#include <iostream>
#include <bit>

namespace zx {
      std::vector<float> heightValues;
FirstApp::FirstApp() {
  globalPool =
      ZxDescriptorPool::Builder(zxDevice)
          .setMaxSets(ZxSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ZxSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, ZxSwapChain::MAX_FRAMES_IN_FLIGHT)
          .build();
  loadGameObjects();
}

FirstApp::~FirstApp() {}

void FirstApp::run() {
  std::vector<std::unique_ptr<ZxBuffer>> uboBuffers(ZxSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < uboBuffers.size(); i++) {
    uboBuffers[i] = std::make_unique<ZxBuffer>(
        zxDevice,
        sizeof(GlobalUbo),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uboBuffers[i]->map();
  }
  auto globalSetLayout =
    ZxDescriptorSetLayout::Builder(zxDevice)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
          .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  Texture texture = Texture(zxDevice, "../textures/voronoi.png");

  VkDescriptorImageInfo imageInfo{};
  imageInfo.sampler = texture.getSampler();
  imageInfo.imageView = texture.getImageView();
  imageInfo.imageLayout = texture.getImageLayout();

  std::vector<VkDescriptorSet> globalDescriptorSets(ZxSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto uboInfo = uboBuffers[i]->descriptorInfo();
    ZxDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &uboInfo)
        .writeImage(1, &imageInfo)
        .build(globalDescriptorSets[i]);
  }

  SimpleRenderSystem simple_render_system{
      zxDevice,
      zxRenderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()};

  VoxelRenderSystem voxel_render_system{
      zxDevice,
      zxRenderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()};

  ZxCamera camera{};

  auto viewerObject = ZxGameObject::createGameObject();
  viewerObject.transform.translation = {0.f, 0.f, 0.f};
  viewerObject.transform.rotation = {0.f, 0.f, 0.f};
  KeyboardMovementController cameraController{};
  float dt = 0.f;
  auto currentTime = std::chrono::high_resolution_clock::now();
  while (!zxWindow.shouldClose()) {
    glfwPollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;

    cameraController.moveInPlaneXZ(zxWindow.getGLFWwindow(), frameTime, viewerObject);
    camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

    float aspect = zxRenderer.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(60.0f), (float)zxWindow.getExtent().width / (float)zxWindow.getExtent().height, 0.1f, 512.0f);
    
    if (auto commandBuffer = zxRenderer.beginFrame()) {
      int frameIndex = zxRenderer.getFrameIndex();
      FrameInfo frameInfo{
          frameIndex,
          frameTime,
          commandBuffer,
          camera,
          globalDescriptorSets[frameIndex],
          gameObjects};
      dt += frameTime/10.f;
      // update

      GlobalUbo ubo{};
      ubo.projection = camera.getProjection();
      ubo.inverseProjection = camera.getInverseProjection();
      ubo.view = camera.getView();
      ubo.inverseView = camera.getInverseView();
      ubo.cameraPosition = camera.getPosition();
      vec3_info("Camera", camera.getPosition());
      ubo.dt = dt;
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      zxRenderer.beginSwapChainRenderPass(commandBuffer);

      simple_render_system.renderGameObjects(frameInfo);
      voxel_render_system.renderChunks(frameInfo);

      zxRenderer.endSwapChainRenderPass(commandBuffer);
      zxRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(zxDevice.device());
}


void FirstApp::loadGameObjects() {
  World world{zxDevice};
  worlds.emplace_back(std::move(world));
  glm::vec2 pos = {0.f, 0.f};;
  worlds[0].generateTerrain(pos, 1);
}

}