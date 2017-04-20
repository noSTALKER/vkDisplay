#include "Application.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

namespace vkDisplay
{
VKAPI_ATTR VkBool32 VKAPI_CALL callbackFunction(
	VkDebugReportFlagsEXT                       flags,
	VkDebugReportObjectTypeEXT                  objectType,
	uint64_t                                    object,
	size_t                                      location,
	int32_t                                     messageCode,
	const char*                                 pLayerPrefix,
	const char*                                 pMessage,
	void*                                       pUserData)
{
	std::cout << pMessage << std::endl;
	return VK_TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	switch (uMsg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		return 0;
	default:
		break;
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

Application::Application()
{
}

Application::~Application()
{
}

vk::Result
Application::createInstance(const std::string& applicationName, uint32_t version)
{
	vk::Result result;
	vk::ApplicationInfo applicationInfo(applicationName.c_str(), version, applicationName.c_str(), version);

	const uint32_t layerCount = 1;
	const char* layerNames[] = { "VK_LAYER_LUNARG_standard_validation" };

	const uint32_t extensionCount = 3;
	const char* extensionNames[] = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_report" };

	//vk::InstanceCreateInfo instanceCreateInfo{ {}, &applicationInfo, layerCount, layerNames, extensionCount, extensionNames};
	vk::InstanceCreateInfo instanceCreateInfo{ {}, &applicationInfo, 0, nullptr, 2, extensionNames };
	std::tie(result, mInstance) = vk::createInstance(instanceCreateInfo);

	auto vkCreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(mInstance, "vkCreateDebugReportCallbackEXT");
	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	createInfo.pfnCallback = callbackFunction;

	VkDebugReportCallbackEXT callback;
	//vkCreateDebugReportCallback(mInstance, &createInfo, nullptr, &callback);

	return result;
}

vk::Result
Application::createDevice()
{
	vk::Result result;

	//get all physical devices
	std::vector<vk::PhysicalDevice> physicalDevices;
	std::tie(result, physicalDevices) = mInstance.enumeratePhysicalDevices();

	if (result != vk::Result::eSuccess) {
		return result;
	}

	// select the first device for now, later we can check the device capabilities and pick the best out of them according to the
	// reqirement
	mPhysicalDevice = physicalDevices[0];
	mMemoryProperties = mPhysicalDevice.getMemoryProperties();

	//get different queue properties
	auto queueProperties = mPhysicalDevice.getQueueFamilyProperties();

	//pick a queue family which supports graphics and transfer capabilities
	mQueueFamily = 0;
	for (uint32_t i = 0; mQueueFamily < queueProperties.size(); ++i) {
		if (queueProperties[i].queueFlags & vk::QueueFlagBits::eGraphics &&
			queueProperties[i].queueFlags & vk::QueueFlagBits::eTransfer) {
			mQueueFamily = i;
			break;
		}
	}

	//create a queue info for creating 1 queue with 1 priority from the desired queue family
	float defaultPriority[] = { 1.0f };
	vk::DeviceQueueCreateInfo defaultQueueCreateInfo({}, mQueueFamily, 1, defaultPriority);

	// get the essential device exntensions
	const char* deviceExtensionNames[] = { "VK_KHR_swapchain" };

	vk::PhysicalDeviceFeatures deviceFeatures;
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.fullDrawIndexUint32 = VK_TRUE;
	deviceFeatures.independentBlend = VK_TRUE;
	deviceFeatures.shaderResourceMinLod = VK_TRUE;

	//create a logical vulkan device with given queues, layers, extensions and features
	vk::DeviceCreateInfo deviceCreateInfo({}, 1, &defaultQueueCreateInfo, 0, nullptr, 1, deviceExtensionNames, &deviceFeatures);
	std::tie(result, mDevice) = mPhysicalDevice.createDevice(deviceCreateInfo);

	if (result != vk::Result::eSuccess) {
		return result;
	}

	//get the default queue that we created
	mQueue = mDevice.getQueue(mQueueFamily, 0);

	//create a command pool for creating commands to submit to the queues
	vk::CommandPoolCreateInfo commandPoolCreateInfo({}, mQueueFamily);
	std::tie(result, mCommandPool) = mDevice.createCommandPool(commandPoolCreateInfo);
	return result;
}

bool
Application::createWindow(const std::string& windowName, int width, int height)
{
	//create a window
	WNDCLASSEX winClass;
	mHinstance = GetModuleHandle(nullptr);

	mWidth = width;
	mHeight = height;

	// Initialize the window class structure
	winClass.cbSize = sizeof(WNDCLASSEX);
	winClass.style = CS_HREDRAW | CS_VREDRAW;
	winClass.lpfnWndProc = WndProc;
	winClass.cbClsExtra = 0;
	winClass.cbWndExtra = 0;
	winClass.hInstance = mHinstance;
	winClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	winClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winClass.lpszMenuName = nullptr;
	winClass.lpszClassName = windowName.c_str();
	winClass.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

	// Register window class
	if (!RegisterClassEx(&winClass)) {
		// It didn't work, so try to give a useful error:
		return false;
	}

	// Create window with the registered class:
	RECT wr = { 0, 0, mWidth, mHeight };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	mHandle = CreateWindowEx(0,
		windowName.c_str(),             // class name
		windowName.c_str(),             // app name
		WS_OVERLAPPEDWINDOW |         // window style
		WS_VISIBLE | WS_SYSMENU,
		100, 100,                     // x/y coords
		wr.right - wr.left,           // width
		wr.bottom - wr.top,           // height
		nullptr,                      // handle to parent
		nullptr,                      // handle to menu
		mHinstance,                    // hInstance
		nullptr);                     // no extra parameters

	if (!mHandle) {
		// It didn't work, so try to give a useful error:
		return false;
	}

	return true;
}

vk::Result Application::createSwapchain()
{
	vk::Result result;

	//create a win32 surface
	vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo({}, mHinstance, mHandle);
	std::tie(result, mSurface) = mInstance.createWin32SurfaceKHR(surfaceCreateInfo);

	if (result != vk::Result::eSuccess) {
		return result;
	}

	//chech if the queue family supports presetation
	if (!mPhysicalDevice.getWin32PresentationSupportKHR(mQueueFamily)) {
		return vk::Result::eSuboptimalKHR;
	}

	//get the surface formats
	std::vector<vk::SurfaceFormatKHR> surfaceFormats;
	std::tie(result, surfaceFormats) = mPhysicalDevice.getSurfaceFormatsKHR(mSurface);

	//get the desired format for surface
	vk::SurfaceFormatKHR surfaceFormat;

	//in case surface formats is undefined, stick to R8B8G8A8UNORM format and SRGB color space
	if (surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined) {
		surfaceFormat.format = vk::Format::eB8G8R8A8Unorm;
		surfaceFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
	}
	else {
		surfaceFormat = surfaceFormats[0];
	}

	mSwapchainFormat = surfaceFormat.format;

	vk::Bool32 supported;
	std::tie(result, supported) = mPhysicalDevice.getSurfaceSupportKHR(mQueueFamily, mSurface);

	//get the surface presentation modes
	std::vector<vk::PresentModeKHR> presentModes;
	std::tie(result, presentModes) = mPhysicalDevice.getSurfacePresentModesKHR(mSurface);

	//set the default desired presentation mode to FIFO but set to Mailbox mode if that is supported
	vk::PresentModeKHR desiredPresentMode = vk::PresentModeKHR::eFifo;
	for (const auto& presentMode : presentModes) {
		if (presentMode == vk::PresentModeKHR::eMailbox) {
			desiredPresentMode = vk::PresentModeKHR::eMailbox;
			break;
		}
	}

	//get the surface capabilities
	vk::SurfaceCapabilitiesKHR surfaceCapabilities;
	std::tie(result, surfaceCapabilities) = mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface);

	//if current swapchain extent is undefined, set it to default window height and width
	if (surfaceCapabilities.currentExtent.width == 0xffffffff) {
		mSwapchainExtent.width = mWidth;
		mSwapchainExtent.height = mHeight;
	}
	else {
		mSwapchainExtent = surfaceCapabilities.currentExtent;
	}

	//Surface transforms and set it to identity if that is supported
	vk::SurfaceTransformFlagBitsKHR surfaceTransform;
	if (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
		surfaceTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	}
	else {
		surfaceTransform = surfaceCapabilities.currentTransform;
	}

	uint32_t swapchainImagesCount = surfaceCapabilities.minImageCount + 1;

	vk::SwapchainCreateInfoKHR swapchainCreateInfo({},
		mSurface,
		swapchainImagesCount,
		surfaceFormat.format,
		surfaceFormat.colorSpace,
		mSwapchainExtent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		0,
		nullptr,
		surfaceTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		desiredPresentMode);

	std::tie(result, mSwapchain) = mDevice.createSwapchainKHR(swapchainCreateInfo);

	//get the swapchain images
	std::tie(result, mSwapchainImages) = mDevice.getSwapchainImagesKHR(mSwapchain);

	//create image view for each image in swapbuffer
	mSwapchainImageViews.reserve(mSwapchainImages.size());

	for (auto& image : mSwapchainImages) {
		vk::ComponentMapping componentMapping(vk::ComponentSwizzle::eR,
			vk::ComponentSwizzle::eG,
			vk::ComponentSwizzle::eB,
			vk::ComponentSwizzle::eA);

		vk::ImageSubresourceRange imageSubresourceRange(vk::ImageAspectFlagBits::eColor,	//Image type
			0,									//Base Mipmap level
			1,									//Mipmap level counts
			0,									//Base Array layer
			1);									//Array layer count

		vk::ImageViewCreateInfo imageViewCreateInfo({}, image, vk::ImageViewType::e2D, surfaceFormat.format, componentMapping, imageSubresourceRange);

		vk::ImageView imageView;
		std::tie(result, imageView) = mDevice.createImageView(imageViewCreateInfo);
		mSwapchainImageViews.push_back(imageView);
	}

	return result;
}

vk::Result Application::createDepthStencilBuffer(vk::Format format)
{
	vk::Result result;

	//create a image for depth-stencil buffer
	vk::Extent3D depthImageExtent(mSwapchainExtent.width, mSwapchainExtent.height, 1);
	vk::ImageCreateInfo depthImageCreateInfo({},
		vk::ImageType::e2D,
		format,
		depthImageExtent,
		1,
		1,
		vk::SampleCountFlagBits::e1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::SharingMode::eExclusive,
		0,
		nullptr);
	std::tie(result, mDepthStencilImage) = mDevice.createImage(depthImageCreateInfo);

	//get the memory requirement for the depth-stencil image
	auto depthMemoryRequirement = mDevice.getImageMemoryRequirements(mDepthStencilImage);
	//allocate momory for depth-stencil image
	vk::DeviceMemory depthMemory = allocateMemory(depthMemoryRequirement, vk::MemoryPropertyFlagBits::eDeviceLocal);
	mDevice.bindImageMemory(mDepthStencilImage, depthMemory, 0);

	//create a view for depth-stencil image
	vk::ComponentMapping depthComponentMapping(vk::ComponentSwizzle::eR,
		vk::ComponentSwizzle::eG,
		vk::ComponentSwizzle::eB,
		vk::ComponentSwizzle::eA);

	vk::ImageSubresourceRange depthSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,	//Image type
		0,									//Base Mipmap level
		1,									//Mipmap level counts
		0,									//Base Array layer
		1);									//Array layer count

	vk::ImageViewCreateInfo depthViewCreateInfo({}, mDepthStencilImage, vk::ImageViewType::e2D, vk::Format::eD24UnormS8Uint, depthComponentMapping, depthSubresourceRange);

	std::tie(result, mDepthStencilView) = mDevice.createImageView(depthViewCreateInfo);
	return result;
}

vk::Buffer Application::createBuffer(void* data, uint64_t dataSize, vk::BufferUsageFlags bufferFlags)
{
	vk::Result result;
	//create a stage buffer to tranfer data from host to device
	vk::BufferCreateInfo stageBufferInfo({},
		dataSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::SharingMode::eExclusive,
		0,
		nullptr);

	vk::Buffer stageBuffer;
	std::tie(result, stageBuffer) = mDevice.createBuffer(stageBufferInfo);

	//get the memory requirement for the stage buffer
	auto stageBufferMemoryRequirement = mDevice.getBufferMemoryRequirements(stageBuffer);

	//Allocate memory for quad buffer in the desired memory type
	vk::DeviceMemory stageBufferMemory = allocateMemory(stageBufferMemoryRequirement, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	//fill the stage buffer memory with vertex data
	void* bufferData;
	std::tie(result, bufferData) = mDevice.mapMemory(stageBufferMemory, 0, dataSize);
	std::memcpy(bufferData, data, dataSize);
	mDevice.unmapMemory(stageBufferMemory);

	//bind the allocated momory to the stage buffer
	mDevice.bindBufferMemory(stageBuffer, stageBufferMemory, 0);

	//create a common buffer for quad vertices, colors and indices
	vk::BufferCreateInfo quadBufferInfo({},
		dataSize,
		bufferFlags,
		vk::SharingMode::eExclusive,
		0,
		nullptr);
	vk::Buffer buffer;
	std::tie(result, buffer) = mDevice.createBuffer(quadBufferInfo);

	//get the memory requirement for the depth-stencil image
	auto bufferMemoryRequirement = mDevice.getBufferMemoryRequirements(buffer);

	//find the memory and allocate it on Device local memory
	vk::DeviceMemory quadBufferMemory = allocateMemory(bufferMemoryRequirement, vk::MemoryPropertyFlagBits::eDeviceLocal);

	//bind the quad buffer to the allocated memory
	mDevice.bindBufferMemory(buffer, quadBufferMemory, 0);

	//create a command buffer to transfer data from stage buffer to device local buffer
	std::vector<vk::CommandBuffer> transferCommandBuffer;
	vk::CommandBufferAllocateInfo transferCommandBufferCreateInfo(mCommandPool, vk::CommandBufferLevel::ePrimary, 1);
	std::tie(result, transferCommandBuffer) = mDevice.allocateCommandBuffers(transferCommandBufferCreateInfo);

	//start the command buffer recording
	vk::CommandBufferBeginInfo transferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	transferCommandBuffer[0].begin(transferBeginInfo);
	vk::BufferCopy bufferCopy(0, 0, dataSize);
	transferCommandBuffer[0].copyBuffer(stageBuffer, buffer, { bufferCopy });
	transferCommandBuffer[0].end();

	//submit the command buffer to the default and wait for it to complete
	vk::SubmitInfo transferSubmitInfo(0, nullptr, nullptr, 1, transferCommandBuffer.data());
	mQueue.submit(1, &transferSubmitInfo, vk::Fence());
	mQueue.waitIdle();

	return buffer;
}

Image 
Application::createImage(const std::string& filename)
{
	vk::Result result;
	int width, height, channels;
	stbi_uc* pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	uint32_t* coverted = reinterpret_cast<uint32_t*>(pixels);
	vk::DeviceSize imageSize = width * height * 4;

	vk::BufferCreateInfo bufferCreateInfo({}, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, 0, nullptr);

	vk::Buffer hostImage;
	std::tie(result, hostImage) = mDevice.createBuffer(bufferCreateInfo);
	auto hostMemoryRequirement = mDevice.getBufferMemoryRequirements(hostImage);
	vk::DeviceMemory hostMemory = allocateMemory(hostMemoryRequirement, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* hostData;
	std::tie(result, hostData) = mDevice.mapMemory(hostMemory, 0, hostMemoryRequirement.size);
	memcpy(hostData, pixels, width * height * 4);
	mDevice.unmapMemory(hostMemory);
	mDevice.bindBufferMemory(hostImage, hostMemory, 0);
	stbi_image_free(pixels);

	//create a image on device memory which is optimized for sampling
	vk::ImageCreateInfo deviceImageInfo({},
		vk::ImageType::e2D,
		vk::Format::eR8G8B8A8Unorm,
		{ (uint32_t)width, (uint32_t)height, 1 },
		1,
		1,
		vk::SampleCountFlagBits::e1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
		vk::SharingMode::eExclusive,
		0,
		nullptr,
		vk::ImageLayout::eUndefined);

	vk::Image deviceImage;
	std::tie(result, deviceImage) = mDevice.createImage(deviceImageInfo);

	vk::MemoryRequirements deviceImageRequirements = mDevice.getImageMemoryRequirements(deviceImage);
	vk::DeviceMemory deviceImageMemory = allocateMemory(deviceImageRequirements, vk::MemoryPropertyFlagBits::eDeviceLocal);
	mDevice.bindImageMemory(deviceImage, deviceImageMemory, 0);

	//
	vk::CommandBuffer commandBuffer = beginSingleTimeCommandBuffer();

	// convert the layout of device image to dest tranfer optimal
	vk::ImageSubresourceRange deviceImageRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
	// the source access flag is host write as host is writing the image data and destination access flag is tranfer read as it is acting as source for transfer image
	vk::ImageMemoryBarrier deviceImageBarrier{ {}, vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 0, 0, deviceImage, deviceImageRange };
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, { deviceImageBarrier });

	//copy the image data
	vk::ImageSubresourceLayers destImageLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1);
	vk::BufferImageCopy bufferImageCopy(0, (uint32_t)width, (uint32_t)height, destImageLayers, { 0, 0, 0 }, {(uint32_t)width, (uint32_t)height, 1});
	commandBuffer.copyBufferToImage(hostImage, deviceImage, vk::ImageLayout::eTransferDstOptimal, { bufferImageCopy });

	//convert the device image to sample optimal
	vk::ImageMemoryBarrier deviceImageSampleBarrier{ vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 0, 0, deviceImage, deviceImageRange };
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, { deviceImageSampleBarrier });

	endSingleTimeCommandBuffer(commandBuffer);

	Image image;
	image.image = deviceImage;
	image.width = width;
	image.height = height;
	image.depth = 1;
	image.format = vk::Format::eR8G8B8A8Unorm;
	return image;
}

vk::DeviceMemory 
Application::allocateMemory(const vk::MemoryRequirements& requirements, vk::MemoryPropertyFlags flags)
{
	vk::Result result;
	//find the memory and allocate it on Device local memory
	uint32_t bufferMemoryType = 0;
	uint32_t bufferMemoryTypeBits = requirements.memoryTypeBits;
	for (uint32_t i = 0; i < mMemoryProperties.memoryTypeCount; ++i) {
		if ((bufferMemoryTypeBits & 1) == 1 && (mMemoryProperties.memoryTypes[i].propertyFlags & flags) == flags) {
			break;
		}

		++bufferMemoryType;
		bufferMemoryTypeBits >>= 1;
	}

	//Allocate memory for quad buffer in the desired memory type and bind it to buffer
	vk::MemoryAllocateInfo allocateInfo(requirements.size, bufferMemoryType);
	vk::DeviceMemory deviceMemory;
	std::tie(result, deviceMemory) = mDevice.allocateMemory(allocateInfo);
	return deviceMemory;
}

vk::CommandBuffer Application::beginSingleTimeCommandBuffer()
{
	vk::CommandBuffer commandBuffer;
	vk::CommandBufferAllocateInfo allocateInfo(mCommandPool, vk::CommandBufferLevel::ePrimary, 1);
	mDevice.allocateCommandBuffers(&allocateInfo, &commandBuffer);

	vk::CommandBufferBeginInfo commandBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(commandBeginInfo);
	return commandBuffer;
}

void Application::endSingleTimeCommandBuffer(vk::CommandBuffer buffer)
{
	buffer.end();
	vk::SubmitInfo submitInfo({}, nullptr, nullptr, 1, &buffer, 0, nullptr);
	mQueue.submit({ submitInfo }, vk::Fence());
	mQueue.waitIdle();
}

vk::Result
Application::createFramebuffers()
{
	vk::Result result;

	//create a renderbuffer for each swapchain image views
	vk::ImageView framebufferImageViews[2];
	framebufferImageViews[1] = mDepthStencilView;

	//create framebuffer coresponding to each swapchain image view
	vk::FramebufferCreateInfo framebufferCreateInfo({}, mRenderpass, 2, framebufferImageViews, mSwapchainExtent.width, mSwapchainExtent.height, 1);
	mFramebuffers.clear();
	mFramebuffers.reserve(mSwapchainImageViews.size());

	for (std::size_t i = 0; i < mSwapchainImageViews.size(); ++i) {
		framebufferImageViews[0] = mSwapchainImageViews[i];
		vk::Framebuffer framebuffer;
		std::tie(result, framebuffer) = mDevice.createFramebuffer(framebufferCreateInfo);
		mFramebuffers.push_back(framebuffer);
	}

	return result;
}

vk::Result 
Application::createRenderpass()
{
	vk::Result result;

	//Main Renderpass attachment info, one for color attachment which will be the one of the swapchain image view 
	// and other will be depth-stencil image view
	vk::AttachmentDescription renderpassAttachments[2];
	vk::AttachmentDescription& colorAttachment = renderpassAttachments[0];
	colorAttachment.format = mSwapchainFormat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

	vk::AttachmentDescription& depthStencilAttachment = renderpassAttachments[1];
	depthStencilAttachment.format = vk::Format::eD24UnormS8Uint;
	depthStencilAttachment.samples = vk::SampleCountFlagBits::e1;
	depthStencilAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthStencilAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	depthStencilAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthStencilAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	depthStencilAttachment.stencilLoadOp = vk::AttachmentLoadOp::eClear;
	depthStencilAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

	//color attachment is at position 0 and depth stencil attachment is at position 1
	vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentReference depthStencilAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	//our renderpass only has a single subpass. Also this renderpass doesn't has a input 
	vk::SubpassDescription subpassDescription({}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &colorAttachmentRef, nullptr, &depthStencilAttachmentRef, 0, nullptr);

	//create a renderpass now with a single subpass with attachments descriptions
	vk::RenderPassCreateInfo renderpassCreateInfo({}, 2, renderpassAttachments, 1, &subpassDescription, 0, nullptr);
	std::tie(result, mRenderpass) = mDevice.createRenderPass(renderpassCreateInfo);

	return result;
}

void 
Application::renderLoop()
{
	while (true) {
		render();
	}
}

}