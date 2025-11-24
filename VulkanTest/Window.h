#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class Window {
public:
	struct WindowExtent { uint32_t width; uint32_t height; };

	Window(int w, int h, std::string name);
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	bool shouldClose() { return glfwWindowShouldClose(mWindow); };
	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	VkExtent2D getExtent() { return { mWidth, mHeight }; };
	float getScale() { return mMainScale; };
	GLFWwindow* getGLFWWindow() const { return mWindow; };
	bool wasWindowResized() { return mFramebufferResized; };
	void resetWindowResizedFlag() { mFramebufferResized = false; };

private:
	static void framebufferResizeCallback(GLFWwindow* window, uint32_t width, uint32_t height);
	void initWindow();
	void onMouseButton(int button, int action, int mods);
	void onMouseMove(double xpos, double ypos);
	void onMouseScroll(double xoffset, double yoffset);

	uint32_t mWidth  = 600;
	uint32_t mHeight = 800;
	float mMainScale;
	double mLastMouseX = 0.0;
	double mLastMouseY = 0.0;
	bool mIsDragging = false;

	bool mFramebufferResized = false;
	std::string mWindowName;
	GLFWwindow* mWindow;
};

