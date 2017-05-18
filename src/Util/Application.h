#pragma once
#ifndef VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NO_EXCEPTIONS
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.hpp>
#include <windows.h>
#include <functional>

#include "Export.h"

namespace vkDisplay
{

struct UTIL_API Image
{
	vk::Image image;
	vk::Format format;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
};

struct UTIL_API Buffer
{
	vk::Buffer buffer;
	vk::DeviceMemory memory;
	uint64_t offset;
	uint64_t size;
};

struct UTIL_API Model
{
	std::vector<float> data;
	std::vector<uint32_t> indices;
	uint64_t vertexCount;
	bool hasNormal;
	bool hasTexCoord;
	bool hasIndices;
};

class UTIL_API Application
{
public:
	Application();
	~Application();
	vk::Result createInstance(const std::string& applicationName, uint32_t version);
	vk::Result createDevice();
	bool createWindow(const std::string& windowName, int width, int height);
	vk::Result createSwapchain();
	vk::Result createDepthStencilBuffer(vk::Format format, vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1);
	Buffer createCoherantBuffer(void * data, uint64_t dataSize, vk::BufferUsageFlags bufferFlags);
	Buffer createDeviceBuffer(void* data, uint64_t dataSize, vk::BufferUsageFlags bufferFlags);
	Image createImage(const std::string& filename);
	Model createModel(const std::string& filename);
	vk::DeviceMemory allocateMemory(const vk::MemoryRequirements& requirements, vk::MemoryPropertyFlags flags);
	vk::CommandBuffer beginSingleTimeCommandBuffer();
	void endSingleTimeCommandBuffer(vk::CommandBuffer buffer);
	virtual vk::Result createFramebuffers();
	virtual vk::Result createRenderpass();
	virtual vk::Result createResources() { return vk::Result::eSuccess; }
	virtual vk::Result createPipeline() { return vk::Result::eSuccess; }
	virtual vk::Result createCommandBuffers() { return vk::Result::eSuccess; }
	virtual void render(double frameTime, double totalTime) {}
	void renderLoop();

	vk::Instance getInstance() const
	{
		return mInstance;
	}

	vk::Device getDevice() const
	{
		return mDevice;
	}

	vk::PhysicalDevice getPhysicalDevice() const
	{
		return mPhysicalDevice;
	}

	vk::SwapchainKHR getSwapchain() const
	{
		return mSwapchain;
	}

	vk::Queue getQueue() const
	{
		return mQueue;
	}

	uint32_t getQueueFamily() const
	{
		return mQueueFamily;
	}

	vk::CommandPool getCommandPool() const
	{
		return mCommandPool;
	}

	vk::Extent2D getSwapchainExtent() const
	{
		return mSwapchainExtent;
	}

	vk::Format getSwapchainFormat() const
	{
		return mSwapchainFormat;
	}

	std::vector<vk::ImageView> getSwapchainImageViews() const
	{
		return mSwapchainImageViews;
	}

	vk::ImageView getDepthStencilView() const
	{
		return mDepthStencilView;
	}

	std::vector<vk::Framebuffer> getFramebuffers() const
	{
		return mFramebuffers;
	}

	vk::RenderPass getRenderpass() const
	{
		return mRenderpass;
	}

	// Disable copying of application instance
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

protected:
	vk::Instance mInstance;
	vk::Device mDevice;
	vk::PhysicalDevice mPhysicalDevice;
	vk::Queue mQueue;
	vk::CommandPool mCommandPool;
	uint32_t mQueueFamily;

	//surface info
	int mWidth;
	int mHeight;
	vk::SurfaceKHR mSurface;

	//swapchain info
	vk::SwapchainKHR mSwapchain;
	std::vector<vk::Image> mSwapchainImages;
	std::vector<vk::ImageView> mSwapchainImageViews;
	vk::Extent2D mSwapchainExtent;
	vk::Format mSwapchainFormat;

	//Depth-Stencil info
	vk::Image mDepthStencilImage;
	vk::ImageView mDepthStencilView;
	vk::Format mDepthStencilFormat;

	//memory info
	vk::PhysicalDeviceMemoryProperties mMemoryProperties;

	// renderpass info
	std::vector<vk::Framebuffer> mFramebuffers;
	vk::RenderPass mRenderpass;

	HINSTANCE mHinstance;
	HWND mHandle;

};

}