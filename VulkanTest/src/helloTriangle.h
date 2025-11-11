#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
//#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"

#include <vector>
#include <optional>


class helloTriangle {
public:

	float MAIN_SCALE = 1.0f;
	uint32_t WIDTH  = static_cast<uint32_t>(1920 * MAIN_SCALE);
	uint32_t HEIGHT = static_cast<uint32_t>(1080 * MAIN_SCALE);


	const std::vector<char const*> validationLayers{
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> RequiredDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifndef NDEBUG
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif // !NDEBUG

	void run();


private:
	struct Vec2 {
		float x, y;
	};

	struct mQueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		bool isComplete() const {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct mSwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct mFractalPushConstants {
		float ZoomLevel;
		uint32_t MaxIterations;
		uint32_t PlaneMode;
		float _padding0;
		Vec2 C_Const;
		Vec2 Z0_Const;
		Vec2 X_Const;
		Vec2 ScreenCenter;
		Vec2 ScreenSize;
	};

	void mainLoop();
	void initVulkan();
	void initWindow();
	void drawFrame();
	void createInstance();
	void setupDebugMessenger();
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void ImGuiWindowSetup();
	void createImGuiPool();
	void createSurface();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffer();
	void createSyncObjects();
	void createImGui();
	void cleanUp();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	mSwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availibleFormats);
	VkPresentModeKHR choseSwapPresentMode(const std::vector<VkPresentModeKHR>& availiblePresetModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	helloTriangle::mQueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void createLogicalDevice();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkDevice mDevice;
	GLFWwindow* mWindow;
	VkInstance mInstance;
	VkSurfaceKHR mSurface;
	VkQueue mPresentQueue;
	VkQueue mGraphicsQueue;
	VkExtent2D mSwapExtent;
	size_t mCurrentFrame = 0;
	VkRenderPass mRenderPass;
	VkFormat mSwapChainFormat;
	VkSwapchainKHR mSwapChain;
	uint32_t mMemoryTypeIndex;
	VkCommandPool mCommandPool;
	VkPipeline mGraphicsPipeline;
	uint32_t mMaxFlightFrames = 2;
	VkPhysicalDevice mPhysicalDevice;
	VkPipelineLayout mPipelineLayout;
	std::vector<VkFence> mInFlightFences;
	VkDescriptorPool mImGuiDescriptorPool;
	std::vector<VkImage> mSwapChainImages;
	VkDebugUtilsMessengerEXT mDebugMessenger;
	std::vector<VkCommandBuffer> mCommandBuffers;
	std::vector<VkImageView> mSwapChainImageViews;
	std::vector<VkFramebuffer> mSwapChainFramebuffers;
	std::vector<VkSemaphore> mImageAvailableSemaphores;
	std::vector<VkSemaphore> mImageRenderFinishedSemaphores;

	mFractalPushConstants mPushConstants = {
		.ZoomLevel = 2.5,
		.MaxIterations = 1000,
		.PlaneMode = 0,
		._padding0 = 0.0f,
		.C_Const = { -0.75, 0.0 },
		.Z0_Const = { 0.0, 0.0 },
		.X_Const = { 2.0, 0.0 },
		.ScreenCenter = { -0.75, 0.0 },
		.ScreenSize = { (float) WIDTH, (float) HEIGHT }
	};
};
