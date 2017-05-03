#ifndef VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_NO_EXCEPTIONS
#endif

#pragma comment(linker, "/subsystem:console")
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.hpp>
#include <iostream>
#include <windows.h>
#include <fstream>

#include "Util/Application.h"

class MultipleSubpassApplication : public vkDisplay::Application
{
public:
	MultipleSubpassApplication() {}
	vk::Result createRenderpass() override;
	vk::Result createResources() override;
	vk::Result createFramebuffers() override;
	vk::Result createPipeline() override;
	vk::Result createCommandBuffers() override;
	void render(double frameTime, double totalTime) override;
private:
	std::vector<vk::CommandBuffer> mCommandBuffers;
	vkDisplay::Buffer mTextureBuffer;
	vk::Pipeline mPipeline;
	vk::PipelineLayout mPipelineLayout;
	vk::Image mImage;
	vk::ImageView mImageView;
	vk::Sampler mSampler;
	vk::DescriptorSetLayout mDescriptorSetLayout;
	vk::Semaphore mRenderFinishSemaphore;
	vk::Semaphore mImageAquireSemaphore;

	//second pass resources
	vk::Image mRenderImage;
	vk::ImageView mRenderImageView;
	vk::Sampler mFullscreenSampler;
	vk::Pipeline mFullscreenPipeline;
	vk::PipelineLayout mFullscreenPipelineLayout;
};

vk::Result
MultipleSubpassApplication::createRenderpass()
{
	vk::Result result;

	//Main Renderpass attachment info, one for color attachment which will be the one of the swapchain image view 
	// and other will be depth-stencil image view
	vk::AttachmentDescription renderpassAttachments[3];
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

	vk::AttachmentDescription& textureAttachment = renderpassAttachments[2];
	textureAttachment.format = vk::Format::eR8G8B8A8Unorm;
	textureAttachment.samples = vk::SampleCountFlagBits::e1;
	textureAttachment.initialLayout = vk::ImageLayout::eUndefined;
	textureAttachment.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	textureAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	textureAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	textureAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	textureAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

	//color attachment is at position 0 and depth stencil attachment is at position 1
	vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentReference depthStencilAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	vk::AttachmentReference textureAttachmentRef(2, vk::ImageLayout::eColorAttachmentOptimal);

	//our renderpass has two subpasses
	//first subpass is the main subpass which has a texture attachment and depoth attachment
	vk::SubpassDescription subpasses[2];
	vk::SubpassDescription& mainPass = subpasses[0];
	mainPass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	mainPass.colorAttachmentCount = 1;
	mainPass.pColorAttachments = &textureAttachmentRef;
	mainPass.pDepthStencilAttachment = &depthStencilAttachmentRef;

	//second subpass is only a fullscreen subpass which reads the texture output from the first subpass
	vk::SubpassDescription& fullscreenPass = subpasses[1];
	fullscreenPass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	fullscreenPass.colorAttachmentCount = 1;
	fullscreenPass.pColorAttachments = &colorAttachmentRef;

	//specify a subpass dependency
	vk::SubpassDependency subpassDependency(0,
		1,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eFragmentShader,
		vk::AccessFlagBits::eColorAttachmentWrite,
		vk::AccessFlagBits::eShaderRead);


	//create a renderpass now with a two subpass with attachments descriptions
	vk::RenderPassCreateInfo renderpassCreateInfo({}, 3, renderpassAttachments, 2, subpasses, 1, &subpassDependency);
	std::tie(result, mRenderpass) = mDevice.createRenderPass(renderpassCreateInfo);

	return result;
}

vk::Result
MultipleSubpassApplication::createResources()
{
	vk::Result result;
	//buffer data
	struct BufferData {
		float vertexData[16] = { -1, 1, 0.0, 1.0,
			-1, -1, 0.0, 0.0,
			1, -1, 1.0, 0.0,
			1, 1, 1.0, 1.0 };
		uint32_t indexData[6] = { 0, 2, 1, 0, 3, 2 };
	} bufferData;

	mTextureBuffer = createDeviceBuffer(&bufferData,
		sizeof(bufferData),
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);

	//create a image by reading the external image
	vkDisplay::Image image = createImage("../images/sample.jpg");
	mImage = image.image;

	//create a image view
	vk::ImageSubresourceRange imageViewRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
	vk::ImageViewCreateInfo imageViewCreateInfo({}, mImage, vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Unorm, vk::ComponentMapping(), imageViewRange);
	std::tie(result, mImageView) = mDevice.createImageView(imageViewCreateInfo);

	vk::SamplerCreateInfo samplerCreateInfo({},
		vk::Filter::eLinear,
		vk::Filter::eLinear,
		vk::SamplerMipmapMode::eLinear,
		vk::SamplerAddressMode::eRepeat,
		vk::SamplerAddressMode::eRepeat,
		vk::SamplerAddressMode::eRepeat,
		0,
		VK_TRUE,
		1,
		VK_FALSE,
		vk::CompareOp::eNever,
		0,
		0,
		vk::BorderColor::eFloatOpaqueBlack,
		VK_FALSE);
	std::tie(result, mSampler) = mDevice.createSampler(samplerCreateInfo);

	//create a image so that it can be used as a color attachment in Renderpass
	vk::ImageCreateInfo renderImageInfo({},
		vk::ImageType::e2D,
		vk::Format::eR8G8B8A8Unorm,
		{ mSwapchainExtent.width, mSwapchainExtent.height, 1 },
		1,
		1,
		vk::SampleCountFlagBits::e1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		0,
		nullptr,
		vk::ImageLayout::eUndefined);

	std::tie(result, mRenderImage) = mDevice.createImage(renderImageInfo);

	vk::MemoryRequirements deviceImageRequirements = mDevice.getImageMemoryRequirements(mRenderImage);
	vk::DeviceMemory renderImageMemory = allocateMemory(deviceImageRequirements, vk::MemoryPropertyFlagBits::eDeviceLocal);
	mDevice.bindImageMemory(mRenderImage, renderImageMemory, 0);

	//create a image view
	vk::ImageViewCreateInfo renderImageViewCreateInfo({}, mRenderImage, vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Unorm, vk::ComponentMapping(), imageViewRange);
	std::tie(result, mRenderImageView) = mDevice.createImageView(renderImageViewCreateInfo);

	vk::SamplerCreateInfo fullscreenSamplerCreateInfo({},
		vk::Filter::eLinear,
		vk::Filter::eLinear,
		vk::SamplerMipmapMode::eLinear,
		vk::SamplerAddressMode::eRepeat,
		vk::SamplerAddressMode::eRepeat,
		vk::SamplerAddressMode::eRepeat,
		0,
		VK_TRUE,
		1,
		VK_FALSE,
		vk::CompareOp::eNever,
		0,
		0,
		vk::BorderColor::eFloatOpaqueBlack,
		VK_FALSE);
	std::tie(result, mFullscreenSampler) = mDevice.createSampler(fullscreenSamplerCreateInfo);

	return vk::Result::eSuccess;
}

vk::Result
MultipleSubpassApplication::createFramebuffers()
{
	vk::Result result;

	//create a renderbuffer for each swapchain image views
	vk::ImageView framebufferImageViews[3];
	framebufferImageViews[1] = mDepthStencilView;
	framebufferImageViews[2] = mRenderImageView;

	//create framebuffer coresponding to each swapchain image view
	vk::FramebufferCreateInfo framebufferCreateInfo({}, mRenderpass, 3, framebufferImageViews, mSwapchainExtent.width, mSwapchainExtent.height, 1);
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
MultipleSubpassApplication::createPipeline()
{
	vk::Result result;
	vk::VertexInputBindingDescription vertexBinding(0, sizeof(float) * 4, vk::VertexInputRate::eVertex);

	//two vertex input attachments, position at location 0 and tex coord at location 1 
	vk::VertexInputAttributeDescription vertexAttributes[2];
	vk::VertexInputAttributeDescription& positionAttribute = vertexAttributes[0];
	positionAttribute.format = vk::Format::eR32G32Sfloat;
	positionAttribute.location = 0;
	positionAttribute.offset = 0;
	positionAttribute.binding = vertexBinding.binding;

	vk::VertexInputAttributeDescription& texCoordAttribute = vertexAttributes[1];
	texCoordAttribute.format = vk::Format::eR32G32Sfloat;
	texCoordAttribute.location = 1;
	texCoordAttribute.offset = 2 * sizeof(float);
	texCoordAttribute.binding = vertexBinding.binding;

	//
	vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding{ 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr };
	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo({}, 1, &descriptorSetLayoutBinding);
	std::tie(result, mDescriptorSetLayout) = mDevice.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

	std::fstream vertexFile("../shaders/MultipleSubpass/basic.vert.spv", std::ios::binary | std::ios::in);
	vertexFile.seekg(0, std::ios::end);
	std::size_t vertexSize = vertexFile.tellg();
	vertexFile.seekg(0, std::ios::beg);

	uint32_t* vertexSPV = new uint32_t[vertexSize / sizeof(uint32_t)];
	vertexFile.read(reinterpret_cast<char*>(vertexSPV), vertexSize);

	vertexFile.close();

	std::fstream fragmentFile("../shaders/MultipleSubpass/basic.frag.spv", std::ios::binary | std::ios::in);
	fragmentFile.seekg(0, std::ios::end);
	std::size_t fragmentSize = fragmentFile.tellg();
	fragmentFile.seekg(0, std::ios::beg);

	uint32_t* fragmentSPV = new uint32_t[fragmentSize / sizeof(uint32_t)];
	fragmentFile.read(reinterpret_cast<char*>(fragmentSPV), fragmentSize);

	fragmentFile.close();

	std::fstream fullscreenFile("../shaders/MultipleSubpass/fullscreen.frag.spv", std::ios::binary | std::ios::in);
	fullscreenFile.seekg(0, std::ios::end);
	std::size_t fullscreenSize = fullscreenFile.tellg();
	fullscreenFile.seekg(0, std::ios::beg);

	uint32_t* fullscreenSPV = new uint32_t[fullscreenSize / sizeof(uint32_t)];
	fullscreenFile.read(reinterpret_cast<char*>(fullscreenSPV), fullscreenSize);

	fullscreenFile.close();

	//create shader modules for vertex and fragment shader
	vk::ShaderModuleCreateInfo vertexShaderModuleCreateInfo({}, vertexSize, vertexSPV);
	vk::ShaderModule vertexShaderModule;
	std::tie(result, vertexShaderModule) = mDevice.createShaderModule(vertexShaderModuleCreateInfo);

	vk::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo({}, fragmentSize, fragmentSPV);
	vk::ShaderModule fragmentShaderModule;
	std::tie(result, fragmentShaderModule) = mDevice.createShaderModule(fragmentShaderModuleCreateInfo);

	vk::ShaderModuleCreateInfo fullscreenShaderModuleCreateInfo({}, fullscreenSize, fullscreenSPV);
	vk::ShaderModule fullscreenShaderModule;
	std::tie(result, fullscreenShaderModule) = mDevice.createShaderModule(fullscreenShaderModuleCreateInfo);

	//create pipeline shader stage info
	vk::PipelineShaderStageCreateInfo shaderStageInfos[2];
	vk::PipelineShaderStageCreateInfo& vertexShaderInfo = shaderStageInfos[0];
	vertexShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
	vertexShaderInfo.pName = "main";
	vertexShaderInfo.module = vertexShaderModule;

	vk::PipelineShaderStageCreateInfo& fragmentShaderInfo = shaderStageInfos[1];
	fragmentShaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
	fragmentShaderInfo.pName = "main";
	fragmentShaderInfo.module = fragmentShaderModule;

	//vertex attribute input info
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, 1, &vertexBinding, 2, vertexAttributes);

	//input assembly info
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

	//viewport info
	vk::Viewport viewport(0, 0, mSwapchainExtent.width, mSwapchainExtent.height, 0.0f, 1.0f);
	vk::Rect2D scissor({ 0, 0 }, mSwapchainExtent);
	vk::PipelineViewportStateCreateInfo viewportInfo({}, 1, &viewport, 1, &scissor);

	//rasterization info
	vk::PipelineRasterizationStateCreateInfo rasterizationInfo({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);

	//multisampling info
	vk::PipelineMultisampleStateCreateInfo multisampleInfo({}, vk::SampleCountFlagBits::e1, VK_FALSE, 0.0f, nullptr, VK_FALSE, VK_FALSE);

	//depth-stencil test info
	vk::PipelineDepthStencilStateCreateInfo depthStencilInfo({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE, VK_FALSE, vk::StencilOpState(), vk::StencilOpState(), 0.0f, 1.0f);

	//color blend info
	vk::PipelineColorBlendAttachmentState colorBlendAttachment(VK_FALSE);
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;
	vk::PipelineColorBlendStateCreateInfo colorBlendInfo({}, VK_FALSE, vk::LogicOp::eNoOp, 1, &colorBlendAttachment);

	//pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo({}, 1, &mDescriptorSetLayout, 0, nullptr);
	std::tie(result, mPipelineLayout) = mDevice.createPipelineLayout(pipelineLayoutCreateInfo);

	vk::GraphicsPipelineCreateInfo pipelineCreateInfo({}, 2, shaderStageInfos, &vertexInputInfo, &inputAssemblyInfo, nullptr, &viewportInfo, &rasterizationInfo, &multisampleInfo, &depthStencilInfo, &colorBlendInfo, nullptr, mPipelineLayout, mRenderpass, 0);

	std::tie(result, mPipeline) = mDevice.createGraphicsPipeline(vk::PipelineCache(), pipelineCreateInfo);

	//now create second pipeline for fullscreen pass
	//create pipeline shader stage info
	vk::PipelineShaderStageCreateInfo fullscreenShaderStageInfos[2];
	vk::PipelineShaderStageCreateInfo& fullscreenVertexShaderInfo = fullscreenShaderStageInfos[0];
	fullscreenVertexShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
	fullscreenVertexShaderInfo.pName = "main";
	fullscreenVertexShaderInfo.module = vertexShaderModule;

	vk::PipelineShaderStageCreateInfo& fullscreenFragmentShaderInfo = fullscreenShaderStageInfos[1];
	fullscreenFragmentShaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
	fullscreenFragmentShaderInfo.pName = "main";
	fullscreenFragmentShaderInfo.module = fullscreenShaderModule;

	//depth-stencil test info, since we dont need a depth pass for a fullscreen effect
	vk::PipelineDepthStencilStateCreateInfo fullscreenDepthStencilInfo({}, VK_FALSE, VK_FALSE, vk::CompareOp::eAlways, VK_FALSE, VK_FALSE, vk::StencilOpState(), vk::StencilOpState(), 0.0f, 1.0f);

	//pipeline layout
	vk::PipelineLayoutCreateInfo fullscreenPipelineLayoutCreateInfo({}, 1, &mDescriptorSetLayout, 0, nullptr);
	std::tie(result, mFullscreenPipelineLayout) = mDevice.createPipelineLayout(fullscreenPipelineLayoutCreateInfo);

	//create the fullscreen pipeline
	vk::GraphicsPipelineCreateInfo fullscreenPipelineCreateInfo({}, 2, fullscreenShaderStageInfos, &vertexInputInfo, &inputAssemblyInfo, nullptr, &viewportInfo, &rasterizationInfo, &multisampleInfo, &fullscreenDepthStencilInfo, &colorBlendInfo, nullptr, mFullscreenPipelineLayout, mRenderpass, 1);
	std::tie(result, mFullscreenPipeline) = mDevice.createGraphicsPipeline(vk::PipelineCache(), fullscreenPipelineCreateInfo);

	return result;
}

vk::Result
MultipleSubpassApplication::createCommandBuffers()
{
	vk::Result result;
	//create command buffer equal to the swapchain images
	vk::CommandBufferAllocateInfo renderCommandBuffersCreateInfo(mCommandPool, vk::CommandBufferLevel::ePrimary, mSwapchainImageViews.size());
	std::tie(result, mCommandBuffers) = mDevice.allocateCommandBuffers(renderCommandBuffersCreateInfo);

	vk::DescriptorPoolSize poolSize(vk::DescriptorType::eCombinedImageSampler, 2);
	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo({}, 2, 1, &poolSize);

	vk::DescriptorPool descriptorPool;
	std::tie(result, descriptorPool) = mDevice.createDescriptorPool(descriptorPoolCreateInfo);
	vk::DescriptorSetAllocateInfo allocateInfo(descriptorPool, 1, &mDescriptorSetLayout);
	std::vector<vk::DescriptorSet> sets;
	std::tie(result, sets) = mDevice.allocateDescriptorSets(allocateInfo);

	vk::DescriptorImageInfo imageInfo(mSampler, mImageView, vk::ImageLayout::eShaderReadOnlyOptimal);
	vk::WriteDescriptorSet writeDescriptorSet(sets[0], 0, 0, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo);
	mDevice.updateDescriptorSets({ writeDescriptorSet }, {});

	vk::DescriptorSetAllocateInfo fullscreenAllocateInfo(descriptorPool, 1, &mDescriptorSetLayout);
	std::vector<vk::DescriptorSet> fullscreenSets;
	std::tie(result, fullscreenSets) = mDevice.allocateDescriptorSets(fullscreenAllocateInfo);

	vk::DescriptorImageInfo fullscreenImageInfo(mFullscreenSampler, mRenderImageView, vk::ImageLayout::eShaderReadOnlyOptimal);
	vk::WriteDescriptorSet fullscreenWriteDescriptorSet(fullscreenSets[0], 0, 0, 1, vk::DescriptorType::eCombinedImageSampler, &fullscreenImageInfo);
	mDevice.updateDescriptorSets({ fullscreenWriteDescriptorSet }, {});

	//
	vk::Rect2D renderArea({ 0, 0 }, mSwapchainExtent);
	vk::ClearValue clearValues[3];

	vk::ClearValue& colorClear = clearValues[0];
	colorClear.color.float32[0] = 0.0f;
	colorClear.color.float32[1] = 0.0f;
	colorClear.color.float32[2] = 0.0f;
	colorClear.color.float32[3] = 0.0f;

	vk::ClearValue& depthStencilClear = clearValues[1];
	depthStencilClear.depthStencil.depth = 1.0f;
	depthStencilClear.depthStencil.stencil = 0;

	vk::ClearValue& textureClear = clearValues[0];
	textureClear.color.float32[0] = 0.0f;
	textureClear.color.float32[1] = 0.0f;
	textureClear.color.float32[2] = 0.0f;
	textureClear.color.float32[3] = 0.0f;

	for (std::size_t i = 0; i < mSwapchainImageViews.size(); ++i) {
		vk::CommandBuffer& commandBuffer = mCommandBuffers[i];
		vk::CommandBufferBeginInfo commandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse, nullptr);
		commandBuffer.begin(commandBufferBeginInfo);

		vk::RenderPassBeginInfo renderpassBeginInfo(mRenderpass, mFramebuffers[i], renderArea, 3, clearValues);
		commandBuffer.beginRenderPass(renderpassBeginInfo, vk::SubpassContents::eInline);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline);
		commandBuffer.bindVertexBuffers(0, { mTextureBuffer.buffer }, { 0 });
		commandBuffer.bindIndexBuffer(mTextureBuffer.buffer, sizeof(float) * 4 * 4, vk::IndexType::eUint32);
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, sets, {});
		commandBuffer.drawIndexed(6, 1, 0, 0, 0);
		commandBuffer.nextSubpass(vk::SubpassContents::eInline);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mFullscreenPipeline);
		//No need to bind the vertex and index buffer as we are using the same data in both subpasses
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mFullscreenPipelineLayout, 0, fullscreenSets, {});
		commandBuffer.drawIndexed(6, 1, 0, 0, 0);
		commandBuffer.endRenderPass();
		commandBuffer.end();
	}

	//create a semaphore to aquire next image from the swapchain images
	std::tie(result, mImageAquireSemaphore) = mDevice.createSemaphore(vk::SemaphoreCreateInfo());
	std::tie(result, mRenderFinishSemaphore) = mDevice.createSemaphore(vk::SemaphoreCreateInfo());

	return result;
}

void
MultipleSubpassApplication::render(double frameTime, double totalTime)
{
	vk::Result result;
	uint32_t currentSwapchainImage;
	std::tie(result, currentSwapchainImage) = mDevice.acquireNextImageKHR(mSwapchain, 10000000, mImageAquireSemaphore, vk::Fence());

	vk::Semaphore waitSemaphores[] = { mImageAquireSemaphore };
	vk::Semaphore finishSemaphores[] = { mRenderFinishSemaphore };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eTopOfPipe };
	vk::SubmitInfo submitInfo(1, waitSemaphores, waitStages, 1, &mCommandBuffers[currentSwapchainImage], 1, finishSemaphores);
	mQueue.submit({ submitInfo }, vk::Fence());

	vk::PresentInfoKHR presentInfo(1, finishSemaphores, 1, &mSwapchain, &currentSwapchainImage);
	mQueue.presentKHR(presentInfo);
}

int main()
{
	int width = 800, height = 600;
	vk::Result result;

	MultipleSubpassApplication application;
	result = application.createInstance("Multiple Subpass", VK_MAKE_VERSION(1, 0, 0));
	result = application.createDevice();
	application.createWindow("Multiple Subpass", 800, 600);
	result = application.createSwapchain();
	result = application.createDepthStencilBuffer(vk::Format::eD24UnormS8Uint);
	result = application.createResources();
	result = application.createRenderpass();
	result = application.createFramebuffers();
	result = application.createPipeline();
	result = application.createCommandBuffers();

	application.renderLoop();

	return 0;
}