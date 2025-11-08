#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>


class helloTriangle {
public:

	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;


	const std::vector<char const*> validationLayers{
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> RequiredDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifndef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
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
		VkSurfaceCapabilitiesKHR capabilities;
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
	void initWindow();
	void initVulkan();
	void drawFrame();
	void createInstance();
	void setupDebugMessenger();
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void createSurface();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffer();
	void createSyncObjects();
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

	VkExtent2D mSwapExtent;
	VkFormat mSwapChainFormat;
	VkDevice mDevice;
	GLFWwindow* mWindow;
	VkInstance mInstance;
	VkSurfaceKHR mSurface;
	VkQueue mPresentQueue;
	VkQueue mGraphicsQueue;
	size_t mCurrentFrame = 0;
	VkRenderPass mRenderPass;
	VkSwapchainKHR mSwapChain;
	uint32_t mMemoryTypeIndex;
	VkCommandPool mCommandPool;
	VkPipeline mGraphicsPipeline;
	uint32_t mMaxFlightFrames = 2;
	VkPhysicalDevice mPhysicalDevice;
	VkPipelineLayout mPipelineLayout;
	std::vector<VkFence> mInFlightFences;
	std::vector<VkImage> mSwapChainImages;
	VkDebugUtilsMessengerEXT mDebugMessenger;
	std::vector<VkFence> mImageInFlightFences;
	std::vector<VkCommandBuffer> mCommandBuffers;
	std::vector<VkImageView> mSwapChainImageViews;
	std::vector<VkFramebuffer> mSwapChainFramebuffers;
	std::vector<VkSemaphore> mRenderFinishedSemaphores;
	std::vector<VkSemaphore> mImageAvailableSemaphores;
	std::vector<VkSemaphore> mImageRenderFinishedSemaphores;
};
