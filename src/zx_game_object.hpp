#pragma once

#include "defines.hpp"
#include "zx_model.hpp"
#include "zx_texture.hpp"

#include "chunk.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace zx {

struct TransformComponent {
  glm::vec3 translation{};
  glm::vec3 scale{1.f, 1.f, 1.f};
  glm::vec3 rotation{};

  // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
  // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
  // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
  glm::mat4 mat4();

  glm::mat3 normalMatrix();
};

struct PointLightComponent {
  float lightIntensity = 1.0f;
};

class ZxGameObject {
 public:
  using id_t = unsigned int;
  using Map = std::unordered_map<id_t, ZxGameObject>;
  static ZxGameObject createGameObject() {
    static id_t currentId = 0;
    return ZxGameObject{currentId++};
  }

  static ZxGameObject makePointLight(
      float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));
      
  static std::unique_ptr<ZxGameObject> create_chunk_object(ZxDevice& zxDevice, glm::vec3 position);

  ZxGameObject(const ZxGameObject &) = delete;
  ZxGameObject &operator=(const ZxGameObject &) = delete;
  ZxGameObject(ZxGameObject &&) = default;
  ZxGameObject &operator=(ZxGameObject &&) = default;

  id_t getId() { return id; }

  glm::vec3 color{};
  TransformComponent transform{};
  // Optional pointer components
  std::shared_ptr<ZxModel> model{};
  std::unique_ptr<Chunk> chunk = nullptr;
  std::unique_ptr<PointLightComponent> pointLight = nullptr;

 private:
  ZxGameObject(id_t objId) : id{objId} {}

  id_t id;
};
}
