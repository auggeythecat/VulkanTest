#include "Window.h"

#include <stdexcept>
#include "imgui/backends/imgui_impl_glfw.h"




void Window::initWindow() {
	if (!glfwInit()) {
		throw std::runtime_error("failed to initialize GLFW");
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	mMainScale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
	mWidth = static_cast<uint32_t>(mWidth * mMainScale);
	mHeight = static_cast<uint32_t>(mHeight * mMainScale);


	mWindow = glfwCreateWindow(mWidth, mHeight, "Fractal garbage", nullptr, nullptr);

	glfwSetWindowUserPointer(mWindow, this);
	glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods) {
		auto app = static_cast<Window*>(glfwGetWindowUserPointer(window));
		app->onMouseButton(button, action, mods);
		});

	glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double xpos, double ypos) {
		auto app = static_cast<Window*>(glfwGetWindowUserPointer(window));
		app->onMouseMove(xpos, ypos);
		});

	glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double xoffset, double yoffset) {
		auto app = static_cast<Window*>(glfwGetWindowUserPointer(window));
		app->onMouseScroll(xoffset, yoffset);
		});
}


void Window::onMouseButton(int button, int action, int mods) {
	if (ImGui::GetIO().WantCaptureMouse) {
		return;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		mIsDragging = true;
	glfwGetCursorPos(mWindow, &mLastMouseX, &mLastMouseY);
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		mIsDragging = false;
	}
}

void Window::onMouseMove(double xpos, double ypos) {
	if (ImGui::GetIO().WantCaptureMouse) {
		mIsDragging = false;
	}

	if (!mIsDragging) {
		return;
	}

	double deltaX = xpos - mLastMouseX;
	double deltaY = ypos - mLastMouseY;

	double complexWidth = 2.0f * mPushConstants.ZoomLevel * (mPushConstants.ScreenSize.x / mPushConstants.ScreenSize.y);
	double complexHeight = 2.0f * mPushConstants.ZoomLevel;

	double unitsPerPixelX = complexWidth / mPushConstants.ScreenSize.x;
	double unitsPerPixelY = complexHeight / mPushConstants.ScreenSize.y;

	mPushConstants.ScreenCenter.x -= deltaX * unitsPerPixelX;
	mPushConstants.ScreenCenter.y -= deltaY * unitsPerPixelY;

	mLastMouseX = xpos;
	mLastMouseY = ypos;
}

void Window::onMouseScroll(double xoffset, double yoffset) {

	double xpos, ypos;

	glfwGetCursorPos(mWindow, &xpos, &ypos);

	float oldZoom = mPushConstants.ZoomLevel;

	float zoomFactor = 1.1;


	if (yoffset > 0) {
		mPushConstants.ZoomLevel /= zoomFactor;
	}
	else if (yoffset < 0) {
		mPushConstants.ZoomLevel *= zoomFactor;
	}

	float newZoom = mPushConstants.ZoomLevel;


	float x_norm = (float)((xpos / WIDTH) * 2.0 - 1.0);
	float y_norm = (float)((ypos / HEIGHT) * 2.0 - 1.0);

	float aspectRatio = (float)WIDTH / (float)HEIGHT;

	float deltaZoom = oldZoom - newZoom;

	mPushConstants.ScreenCenter.x += x_norm * deltaZoom * aspectRatio;
	mPushConstants.ScreenCenter.y += y_norm * deltaZoom;
}
