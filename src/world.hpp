#pragma once

#include "chunk.hpp"
#include "defines.hpp"
#include "zx_device.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include <vector>
#include <memory> // for std::shared_ptr<>
#include <array>
#include <functional> // for std::max()
#include <cstring> // for std::memcpy()

namespace zx {

struct NoiseSettings {
  int octaves;
  float amplitude;
  float smoothness;
  float roughness;
  float offset;
};

class World {
public:
  World(ZxDevice& zxDevice);

  ~World();

  void createChunkHeightMap(const glm::vec3& position, int worldSize, int seed);
  void createTerrain(const glm::vec2& chunk_pos);
  void generateTerrain(glm::vec2& chunk_pos, uint32_t worldSize);

  std::vector<std::unique_ptr<Chunk>> chunks;
  std::array<uint32_t, CHUNK_AREA> height_map;

  ZxDevice& zxDevice;
};
}