#include "zx_window.hpp"

#include <stdexcept>

namespace zx {

ZxWindow::ZxWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
  initWindow();
}

ZxWindow::~ZxWindow() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void ZxWindow::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void ZxWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
  if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
    panic("Failed to create window surface");
  }
}

void ZxWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
  auto zxWindow = reinterpret_cast<ZxWindow *>(glfwGetWindowUserPointer(window));
  zxWindow->framebufferResized = true;
  zxWindow->width = width;
  zxWindow->height = height;
}

}
