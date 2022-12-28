#version 450

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec3 frag_normal;

layout (location = 0) out vec4 out_color;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 inverseProjection;
  mat4 view;
  mat4 inverseView;
  vec3 cameraPositon;
  float dt;
} ubo;

layout(set = 0, binding = 2) uniform sampler2D image;

void main() {
  if(abs((vec3(0.8f, 0.2f, 0.25f) - frag_color).x) < 0.05f) {
    //discard;
  }
  out_color = vec4(max(dot(normalize(vec3(-1.f, -1.f, -1.f)), frag_normal), 0.f) * frag_color, 1.f);
}