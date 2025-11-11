#include "src/helloTriangle.h"

#include "src/FileHelpers.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"


#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <map>
#include <optional>
#include <set>
#include <algorithm>

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void helloTriangle::run() {
	initWindow();
	initVulkan();
	mainLoop();
	cleanUp();
}

void helloTriangle::initWindow() {
	if (!glfwInit()) {
		throw std::runtime_error("failed to initialize GLFW");
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	
	MAIN_SCALE = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
	WIDTH = static_cast<uint32_t>(WIDTH * MAIN_SCALE);
	HEIGHT = static_cast<uint32_t>(HEIGHT * MAIN_SCALE);

	mWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

	glfwSetWindowUserPointer(mWindow, this);
	glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods) {
		auto app = static_cast<helloTriangle*>(glfwGetWindowUserPointer(window));
		app->onMouseButton(button, action, mods);
	});

	glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double xpos, double ypos) {
		auto app = static_cast<helloTriangle*>(glfwGetWindowUserPointer(window));
		app->onMouseMove(xpos, ypos);
	});

	glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double xoffset, double yoffset) {
		auto app = static_cast<helloTriangle*>(glfwGetWindowUserPointer(window));
		app->onMouseScroll(xoffset, yoffset);
	});
}

void helloTriangle::onMouseButton(int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			mIsDragging = true;
			glfwGetCursorPos(mWindow, &mLastMouseX, &mLastMouseY);
		} else if (action == GLFW_RELEASE) {
			mIsDragging = false;
		}
	}
}

void helloTriangle::onMouseMove(double xpos, double ypos) {
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

void helloTriangle::onMouseScroll(double xoffset, double yoffset) {
	double zoomFactor = 1.1;
	if (yoffset > 0) {
		mPushConstants.ZoomLevel /= zoomFactor;
	}
	else if (yoffset < 0) {
		mPushConstants.ZoomLevel *= zoomFactor;
	}
}

void helloTriangle::mainLoop() {
	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();
		int fbWidth, fbHeight;
		glfwGetFramebufferSize(mWindow, &fbWidth, &fbHeight);
		
		if (glfwGetWindowAttrib(mWindow, GLFW_ICONIFIED) != 0)
		{
			ImGui_ImplGlfw_Sleep(10);
			continue;
		}

		

		drawFrame();

	}

	vkDeviceWaitIdle(mDevice);
}

void helloTriangle::ImGuiWindowSetup() {
	bool cosineInterpolation = mPushConstants.ColorMode == 0;


	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::SliderFloat("Zoom", &mPushConstants.ZoomLevel, 3.0f, 0.0001f);
	ImGui::SliderInt("Max Iterations", (int*)&mPushConstants.MaxIterations, 10, 10000);
	ImGui::DragFloat2("Screen Center", (float*)&mPushConstants.ScreenCenter, 0.001f);
	ImGui::DragFloat2("C_Const", (float*)&mPushConstants.C_Const, 0.001f);
	ImGui::DragFloat2("Z0_Const", (float*)&mPushConstants.Z0_Const, 0.001f);
	ImGui::DragFloat2("X_Const", (float*)&mPushConstants.X_Const, 0.001f);

	const char* PlaneModes[] = { "Mandelbrot (C Plane)", "Julia (Z Plane)", "Exponent (X Plane)" };
	ImGui::Combo("Plane Mode", (int*)&mPushConstants.PlaneMode, PlaneModes, 3);

	const char* ColorModes[] = { "Cosine interpolation", "HSV interpolation", "Pallete lerp"};
	ImGui::Combo("Color Mode", (int*)&mPushConstants.ColorMode, ColorModes, 3);

	ImGui::SliderFloat("ColorScaler", &mPushConstants.colorScaler, 0.1f, 0.001f, "%1.5f", ImGuiSliderFlags_Logarithmic);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::EndFrame();
	ImGui::Render();
}

void helloTriangle::drawFrame() {
	
	VkCommandBuffer currentCommandBuffer = mCommandBuffers[mCurrentFrame];

	vkWaitForFences(mDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

	ImGuiWindowSetup();

	uint32_t imageIndex;
	vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);

	vkResetFences(mDevice, 1, &mInFlightFences[mCurrentFrame]);
	
	vkResetCommandBuffer(currentCommandBuffer, 0);

	recordCommandBuffer(currentCommandBuffer, imageIndex);
	
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;


	VkSemaphore activeRenderSemaphore = mImageRenderFinishedSemaphores[imageIndex];
	VkSemaphore waitSemaphores[]      = { mImageAvailableSemaphores[mCurrentFrame] };
	VkSemaphore signalSemaphors[]     = { activeRenderSemaphore };
	
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount     = 1;
	submitInfo.pWaitSemaphores        = waitSemaphores;
	submitInfo.pWaitDstStageMask      = waitStages;
	submitInfo.commandBufferCount     = 1;
	submitInfo.pCommandBuffers        = &currentCommandBuffer;

	submitInfo.signalSemaphoreCount   = 1;
	submitInfo.pSignalSemaphores      = signalSemaphors;

	//vkEndCommandBuffer(currentCommandBuffer);

	if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType                 = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount    = 1;
	presentInfo.pWaitSemaphores       = signalSemaphors;
	VkSwapchainKHR swapChains[]       = { mSwapChain };
	presentInfo.swapchainCount        = 1;
	presentInfo.pSwapchains           = swapChains;
	presentInfo.pImageIndices         = &imageIndex;
	presentInfo.pResults              = nullptr;

	vkQueuePresentKHR(mPresentQueue, &presentInfo);
	mCurrentFrame = (mCurrentFrame + 1) % mMaxFlightFrames;
}

void helloTriangle::cleanUp() {
	vkDeviceWaitIdle(mDevice);
	for (size_t i = 0; i < mMaxFlightFrames; i++) {
	vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
	vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
	}
	for (size_t i = 0; i < mSwapChainImages.size(); i++) {
	vkDestroySemaphore(mDevice, mImageRenderFinishedSemaphores[i], nullptr);
	}
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	vkDestroyDescriptorPool(mDevice, mImGuiDescriptorPool, nullptr);
	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
	for (auto framebuffer : mSwapChainFramebuffers) { vkDestroyFramebuffer(mDevice, framebuffer, nullptr); }
	vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
	for (auto imageView : mSwapChainImageViews) { vkDestroyImageView(mDevice, imageView, nullptr); }
	vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
	vkDestroyDevice(mDevice, nullptr);
	if (enableValidationLayers) { DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr); }
	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr);
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void helloTriangle::initVulkan() {
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createImGuiPool();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandPool();
	createCommandBuffer();
	createSyncObjects();
	createImGui();
}

void helloTriangle::createImGui() {

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(MAIN_SCALE);
	style.FontScaleDpi = MAIN_SCALE;

	ImGui_ImplGlfw_InitForVulkan(mWindow, true);
	
	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = mInstance;
	initInfo.PhysicalDevice = mPhysicalDevice;
	initInfo.Device = mDevice;
	initInfo.QueueFamily = findQueueFamilies(mPhysicalDevice).graphicsFamily.value();
	initInfo.Queue = mGraphicsQueue;
	initInfo.DescriptorPool = mImGuiDescriptorPool;
	initInfo.MinImageCount = mSwapChainImages.size();
	initInfo.ImageCount = mSwapChainImages.size();
	initInfo.PipelineInfoMain.RenderPass = mRenderPass;
	initInfo.PipelineInfoMain.Subpass = 0;
	initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&initInfo);

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
}

void helloTriangle::createImGuiPool() {
	
	VkDescriptorPoolSize poolSize[] =
	{
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE },
	};
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 0;
	for (VkDescriptorPoolSize& pool_size : poolSize)
		poolInfo.maxSets += pool_size.descriptorCount;
	poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSize);
	poolInfo.pPoolSizes = poolSize;
	
	if (vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mImGuiDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create ImGui Descriptor pool!");
	}
	
}

void helloTriangle::pickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			mPhysicalDevice = device;
			break;
		}
	}

	if (mPhysicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitible GPU!");
	}

}

void helloTriangle::createInstance() {
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void helloTriangle::setupDebugMessenger() {
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void helloTriangle::createSurface() {
	if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

void helloTriangle::createSwapChain() {
	mSwapChainSupportDetails swapChainSupport = querySwapChainSupport(mPhysicalDevice);

	VkExtent2D         extent        =        chooseSwapExtent(swapChainSupport.capabilities);
	VkPresentModeKHR   presentMode   =    choseSwapPresentMode(swapChainSupport.presentModes);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface          = mSurface;
	createInfo.minImageCount    = imageCount;
	createInfo.imageFormat      = surfaceFormat.format;
	createInfo.imageColorSpace  = surfaceFormat.colorSpace;
	createInfo.imageExtent      = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	mQueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, nullptr);
	mSwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, mSwapChainImages.data());

	mSwapChainFormat = surfaceFormat.format;
	mSwapExtent = extent;
}

void helloTriangle::createImageViews() {
	mSwapChainImageViews.resize(mSwapChainImages.size());

	for (size_t i = 0; i < mSwapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image                           = mSwapChainImages[i];
		createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format                          = mSwapChainFormat;
		createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel   = 0;
		createInfo.subresourceRange.levelCount     = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount     = 1;
		if (vkCreateImageView(mDevice, &createInfo, nullptr, &mSwapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
	}
}

void helloTriangle::createRenderPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format         = mSwapChainFormat;
	colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment  = 0;
	colorAttachmentRef.layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint      = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount   = 1;
	subpass.pColorAttachments      = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass          = 0;
	dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask       = 0;
	dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments    = &colorAttachment;
	renderPassInfo.subpassCount    = 1;
	renderPassInfo.pSubpasses      = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies   = &dependency;


	if (vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void helloTriangle::createGraphicsPipeline() {
	auto vertShaderCode                             = FileHelpers::readFile("shaders/shader.vert.spv");
	auto fragShaderCode                             = FileHelpers::readFile("shaders/shader.frag.spv");

	VkShaderModule vertShaderModule                 = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule                 = createShaderModule(fragShaderCode);
	
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType                       = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage                       = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module                      = vertShaderModule;
	vertShaderStageInfo.pName                       = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType                       = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage                       = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module                      = fragShaderModule;
	fragShaderStageInfo.pName                       = "main";

	VkPipelineShaderStageCreateInfo shaderStages[]  = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount   = 0;
	vertexInputInfo.pVertexBindingDescriptions      = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions    = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType                             = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology                          = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable            = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount                     = 1;
	viewportState.scissorCount                      = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType                                = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable                     = VK_FALSE;
	rasterizer.rasterizerDiscardEnable              = VK_FALSE;
	rasterizer.polygonMode                          = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth                            = 1.0f;
	rasterizer.cullMode                             = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace                            = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable                      = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType                             = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable               = VK_FALSE;
	multisampling.rasterizationSamples              = VK_SAMPLE_COUNT_1_BIT;
	
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask             = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable                = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType                             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable                     = VK_FALSE;
	colorBlending.logicOp                           = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount                   = 1;
	colorBlending.pAttachments                      = &colorBlendAttachment;
	colorBlending.blendConstants[0]                 = 0.0f;
	colorBlending.blendConstants[1]                 = 0.0f;
	colorBlending.blendConstants[2]                 = 0.0f;
	colorBlending.blendConstants[3]                 = 0.0f;

	std::vector<VkDynamicState> dynamicStates       = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType                              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount                  = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates                     = dynamicStates.data();

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(mFractalPushConstants);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = mPipelineLayout;
	pipelineInfo.renderPass = mRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(mDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(mDevice, fragShaderModule, nullptr);
}

void helloTriangle::createFramebuffers() {
	mSwapChainFramebuffers.resize(mSwapChainImageViews.size());

	for (size_t i = 0; i < mSwapChainImages.size(); i++) {
		VkImageView attachments[] = {
			mSwapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass      = mRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments    = attachments;
		framebufferInfo.width           = mSwapExtent.width;
		framebufferInfo.height          = mSwapExtent.height;
		framebufferInfo.layers          = 1;

		if (vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void helloTriangle::createCommandPool() {
	mQueueFamilyIndices queueFamilyIndices = findQueueFamilies(mPhysicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void helloTriangle::createCommandBuffer() {
	mCommandBuffers.resize(mMaxFlightFrames);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = mMaxFlightFrames;
	
	if (vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffer!");
	}
}

void helloTriangle::createSyncObjects() {
	mInFlightFences.resize(mMaxFlightFrames);
	mImageAvailableSemaphores.resize(mMaxFlightFrames);
	mImageRenderFinishedSemaphores.resize(mSwapChainImages.size());

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


	for (size_t i = 0; i < mSwapChainImages.size(); i++) {
		if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageRenderFinishedSemaphores[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image semaphores!");
		}
	}

	
	for (size_t i = 0; i < mMaxFlightFrames; i++) {
		if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(mDevice, &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create semaphores!");
		}
	}
}

void helloTriangle::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {


	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType                  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags                  = 0;
	beginInfo.pInheritanceInfo       = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}


	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass        = mRenderPass;
	renderPassInfo.framebuffer       = mSwapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = mSwapExtent;


	VkClearValue clearColor          = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	renderPassInfo.clearValueCount   = 1;
	renderPassInfo.pClearValues      = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);


	VkViewport viewport{};
	viewport.x                       = 0.0f;
	viewport.y                       = 0.0f;
	viewport.width                   = static_cast<float>(mSwapExtent.width);
	viewport.height                  = static_cast<float>(mSwapExtent.height);
	viewport.minDepth                = 0.0f;
	viewport.maxDepth                = 1.0f;

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset                   = {0, 0};
	scissor.extent                   = mSwapExtent;

	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdPushConstants(commandBuffer, mPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(mFractalPushConstants), &mPushConstants);


	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
	
	vkCmdEndRenderPass(commandBuffer);
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

std::vector<const char*> helloTriangle::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	return extensions;
}

bool helloTriangle::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

bool helloTriangle::isDeviceSuitable(VkPhysicalDevice device) {
	helloTriangle::mQueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		helloTriangle::mSwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	return indices.isComplete() && extensionsSupported && swapChainAdequate;

}

bool helloTriangle::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;

	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availibleExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availibleExtensions.data());
	std::set<std::string> requiredExtensions(RequiredDeviceExtensions.begin(), RequiredDeviceExtensions.end());
	for (const auto& extension : availibleExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}
	
	return requiredExtensions.empty();
}

void helloTriangle::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT & createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

helloTriangle::mQueueFamilyIndices helloTriangle::findQueueFamilies(VkPhysicalDevice device) {
	helloTriangle::mQueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);

			if (presentSupport) { indices.presentFamily = i; }
			indices.graphicsFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}
	return indices;
}

helloTriangle::mSwapChainSupportDetails helloTriangle::querySwapChainSupport(VkPhysicalDevice device) {
	helloTriangle::mSwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.capabilities);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, details.formats.data());
	}

	uint32_t presentModesCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModesCount, nullptr);
	if (presentModesCount != 0) {
		details.presentModes.resize(presentModesCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModesCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR helloTriangle::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availibleFormats) {
	for (const auto& availibleFormat : availibleFormats) {
		if (availibleFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availibleFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availibleFormat;
		}
	}
	return availibleFormats[0];
}

VkPresentModeKHR helloTriangle::choseSwapPresentMode(const std::vector<VkPresentModeKHR>& availiblePresetModes) {
	for (const auto& availiblePresetMode : availiblePresetModes) {
		if (availiblePresetMode== VK_PRESENT_MODE_MAILBOX_KHR) {
			return availiblePresetMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D helloTriangle::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(mWindow, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width  = std::clamp(actualExtent.width,  capabilities.minImageExtent.width,  capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

VkShaderModule helloTriangle::createShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void helloTriangle::createLogicalDevice() {
	helloTriangle::mQueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);


	}
	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(RequiredDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = RequiredDeviceExtensions.data();


	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);
	vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
}

uint32_t helloTriangle::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		// Check both the filter (requirements) and the desired properties
		if ((typeFilter & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error("Failed to find suitable memory type!");
}

