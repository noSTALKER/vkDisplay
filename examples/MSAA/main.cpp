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

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class MSAAApplication : public vkDisplay::Application
{
public:
	MSAAApplication() {}
	vk::Result createResources() override;
	vk::Result createPipeline() override;
	vk::Result createFramebuffers() override;
	vk::Result createRenderpass() override;
	vk::Result createCommandBuffers() override;
	void render(double frameTime, double totalTime) override;
private:
	std::vector<vk::CommandBuffer> mCommandBuffers;
	vkDisplay::Buffer mCubeBuffer;
	vkDisplay::Buffer mUniformBuffer;
	vk::Pipeline mPipeline;
	vk::PipelineLayout mPipelineLayout;
	vk::Image mImage;
	vk::ImageView mImageView;
	vk::Sampler mSampler;
	vk::DescriptorSetLayout mDescriptorSetLayout;
	vk::Semaphore mRenderFinishSemaphore;
	vk::Semaphore mImageAquireSemaphore;
	glm::mat4x4 mMVP;
	vk::Image mSampledImage;
	vk::ImageView mSampledImageView;
	vk::Image mSampledDepth;
	vk::ImageView mSampledDepthView;
};

vk::Result
MSAAApplication::createResources()
{
	vk::Result result;

	mUniformBuffer = createCoherantBuffer(&mMVP, sizeof(mMVP), vk::BufferUsageFlagBits::eUniformBuffer);

	//buffer data
	struct BufferData {
		float vertexData[180] = {
		 // left face
		 -1, 1, 1, 0.f, 0.f,    // lft-btm-back
		 -1, -1, -1, 1.f, 1.f,  // lft-top-front
		 -1, -1, 1, 0.f, 1.f,   // lft-top-back
		 -1, -1, -1, 1.f, 1.f,  // lft-top-front
		 -1, 1, 1, 0.f, 0.f,    // lft-btm-back
		 -1, 1, -1, 1.f, 0.f,   // lft-btm-front
		// front face
		 1, -1, -1, 1.f, 1.f,   // rgt-top-front
		 -1, -1, -1, 0.f, 1.f,  // lft-top-front
		 1, 1, -1, 1.f, 0.f,    // rgt-btm-front
		 1, 1, -1, 1.f, 0.f,    // rgt-btm-front
		 -1, -1, -1, 0.f, 1.f,  // lft-top-front
		 -1, 1, -1, 0.f, 0.f,   // lft-btm-front
	     // top face
		 1, -1, 1, 1.f, 1.f,    // rgt-top-back
		 -1, -1, -1, 0.f, 0.f,  // lft-top-front
		 1, -1, -1, 1.f, 0.f,   // rgt-top-front
		 -1, -1, 1, 0.f, 1.f,   // lft-top-back
		 -1, -1, -1, 0.f, 0.f,  // lft-top-front
		 1, -1, 1, 1.f, 1.f,    // rgt-top-back
		 // bottom face
		 1, 1, 1, 1.f, 0.f,    // rgt-btm-back
		 -1, 1, -1, 0.f, 1.f,  // lft-btm-front
		 -1, 1, 1, 0.f, 0.f,   // lft-btm-back
		 1, 1, -1, 1.f, 1.f,   // rgt-btm-front
		 -1, 1, -1, 0.f, 1.f,  // lft-btm-front
		 1, 1, 1, 1.f, 0.f,    // rgt-btm-back
		// right face
		 1, -1, 1, 1.f, 1.f,   // rgt-top-back
		 1, 1, -1, 0.f, 0.f,   // rgt-btm-front
		 1, 1, 1, 1.f, 0.f,    // rgt-btm-back
		 1, 1, -1, 0.f, 0.f,   // rgt-btm-front
		 1, -1, 1, 1.f, 1.f,   // rgt-top-back
		 1, -1, -1, 0.f, 1.f,  // rgt-top-front
		// back face
		 1, 1, 1, 0.f, 0.f,    // rgt-btm-back
		 -1, 1, 1, 1.f, 0.f,   // lft-btm-back
		 -1, -1, 1, 1.f, 1.f,  // lft-top-back
		 1, 1, 1, 0.f, 0.f,    // rgt-btm-back
		 -1, -1, 1, 1.f, 1.f,  // lft-top-back
		 1, -1, 1, 0.f, 1.f };   // rgt-top-back
	} bufferData;

	mCubeBuffer = createDeviceBuffer(&bufferData,
		sizeof(bufferData),
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);

	vkDisplay::Image image = createImage("../images/sample.jpg");
	mImage = image.image;

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
		4,
		VK_FALSE,
		vk::CompareOp::eNever,
		0,
		0,
		vk::BorderColor::eFloatOpaqueBlack,
		VK_FALSE);
	std::tie(result, mSampler) = mDevice.createSampler(samplerCreateInfo);

	//create a transient image for multisample color attachment
	vk::Extent3D extent(mSwapchainExtent.width, mSwapchainExtent.height, 1);
	vk::ImageCreateInfo sampledImageCreateInfo({}, vk::ImageType::e2D, mSwapchainFormat, extent, 1, 1, vk::SampleCountFlagBits::e4, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment);
	std::tie(result, mSampledImage) = mDevice.createImage(sampledImageCreateInfo);

	vk::MemoryRequirements sampledImageRequirements = mDevice.getImageMemoryRequirements(mSampledImage);
	vk::DeviceMemory sampledImageMemory = allocateMemory(sampledImageRequirements, vk::MemoryPropertyFlagBits::eDeviceLocal);
	mDevice.bindImageMemory(mSampledImage, sampledImageMemory, 0);

	vk::ImageViewCreateInfo sampledImageViewCreateInfo({}, mSampledImage, vk::ImageViewType::e2D, mSwapchainFormat, vk::ComponentMapping(), imageViewRange);
	std::tie(result, mSampledImageView) = mDevice.createImageView(sampledImageViewCreateInfo);

	//create a transient image for multisample depth attachment
	vk::ImageCreateInfo sampledDepthCreateInfo({}, vk::ImageType::e2D, mDepthStencilFormat, extent, 1, 1, vk::SampleCountFlagBits::e4, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eDepthStencilAttachment);
	std::tie(result, mSampledDepth) = mDevice.createImage(sampledDepthCreateInfo);

	vk::MemoryRequirements sampledDepthRequirements = mDevice.getImageMemoryRequirements(mSampledDepth);
	vk::DeviceMemory sampledDepthMemory = allocateMemory(sampledDepthRequirements, vk::MemoryPropertyFlagBits::eDeviceLocal);
	mDevice.bindImageMemory(mSampledDepth, sampledDepthMemory, 0);


	vk::ImageSubresourceRange depthSubresourceRange(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,	//Image type
		0,									//Base Mipmap level
		1,									//Mipmap level counts
		0,									//Base Array layer
		1);									//Array layer count
	vk::ImageViewCreateInfo sampledDepthViewCreateInfo({}, mSampledDepth, vk::ImageViewType::e2D, mDepthStencilFormat, vk::ComponentMapping(), depthSubresourceRange);
	std::tie(result, mSampledDepthView) = mDevice.createImageView(sampledDepthViewCreateInfo);

	return vk::Result::eSuccess;
}

vk::Result
MSAAApplication::createPipeline()
{
	vk::Result result;
	vk::VertexInputBindingDescription vertexBinding(0, sizeof(float) * 5, vk::VertexInputRate::eVertex);

	//two vertex input attachments, position at location 0 and tex coord at location 1 
	vk::VertexInputAttributeDescription vertexAttributes[2];
	vk::VertexInputAttributeDescription& positionAttribute = vertexAttributes[0];
	positionAttribute.format = vk::Format::eR32G32B32Sfloat;
	positionAttribute.location = 0;
	positionAttribute.offset = 0;
	positionAttribute.binding = vertexBinding.binding;

	vk::VertexInputAttributeDescription& texCoordAttribute = vertexAttributes[1];
	texCoordAttribute.format = vk::Format::eR32G32Sfloat;
	texCoordAttribute.location = 1;
	texCoordAttribute.offset = 3 * sizeof(float);
	texCoordAttribute.binding = vertexBinding.binding;

	//
	vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding[2];
	vk::DescriptorSetLayoutBinding& uniformLayoutBinding = descriptorSetLayoutBinding[0];
	uniformLayoutBinding.binding = 0;
	uniformLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	uniformLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
	uniformLayoutBinding.descriptorCount = 1;

	vk::DescriptorSetLayoutBinding& samplerLayoutBinding = descriptorSetLayoutBinding[1];
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;
	samplerLayoutBinding.descriptorCount = 1;
	
	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo({}, 2, descriptorSetLayoutBinding);
	std::tie(result, mDescriptorSetLayout) = mDevice.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

	std::fstream vertexFile("../shaders/UniformBuffer/basic.vert.spv", std::ios::binary | std::ios::in);
	vertexFile.seekg(0, std::ios::end);
	std::size_t vertexSize = vertexFile.tellg();
	vertexFile.seekg(0, std::ios::beg);

	uint32_t* vertexSPV = new uint32_t[vertexSize / sizeof(uint32_t)];
	vertexFile.read(reinterpret_cast<char*>(vertexSPV), vertexSize);

	vertexFile.close();

	std::fstream fragmentFile("../shaders/UniformBuffer/basic.frag.spv", std::ios::binary | std::ios::in);
	fragmentFile.seekg(0, std::ios::end);
	std::size_t fragmentSize = fragmentFile.tellg();
	fragmentFile.seekg(0, std::ios::beg);

	uint32_t* fragmentSPV = new uint32_t[fragmentSize / sizeof(uint32_t)];
	fragmentFile.read(reinterpret_cast<char*>(fragmentSPV), fragmentSize);

	fragmentFile.close();

	//create shader modules for vertex and fragment shader
	vk::ShaderModuleCreateInfo vertexShaderModuleCreateInfo({}, vertexSize, vertexSPV);
	vk::ShaderModule vertexShaderModule;
	std::tie(result, vertexShaderModule) = mDevice.createShaderModule(vertexShaderModuleCreateInfo);

	vk::ShaderModuleCreateInfo fragmentShaderModuleCreateInfo({}, fragmentSize, fragmentSPV);
	vk::ShaderModule fragmentShaderModule;
	std::tie(result, fragmentShaderModule) = mDevice.createShaderModule(fragmentShaderModuleCreateInfo);

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
	vk::PipelineMultisampleStateCreateInfo multisampleInfo({}, vk::SampleCountFlagBits::e4, VK_FALSE, 0.0f, nullptr, VK_FALSE, VK_FALSE);

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

	return result;
}

vk::Result MSAAApplication::createFramebuffers()
{
	vk::Result result;

	//create a renderbuffer for each swapchain image views
	vk::ImageView framebufferImageViews[4];
	framebufferImageViews[0] = mSampledImageView;
	framebufferImageViews[2] = mSampledDepthView;
	framebufferImageViews[3] = mDepthStencilView;

	//create framebuffer coresponding to each swapchain image view
	vk::FramebufferCreateInfo framebufferCreateInfo({}, mRenderpass, 4, framebufferImageViews, mSwapchainExtent.width, mSwapchainExtent.height, 1);
	mFramebuffers.clear();
	mFramebuffers.reserve(mSwapchainImageViews.size());

	for (std::size_t i = 0; i < mSwapchainImageViews.size(); ++i) {
		framebufferImageViews[1] = mSwapchainImageViews[i];
		vk::Framebuffer framebuffer;
		std::tie(result, framebuffer) = mDevice.createFramebuffer(framebufferCreateInfo);
		mFramebuffers.push_back(framebuffer);
	}

	return result;
}

vk::Result MSAAApplication::createRenderpass()
{
	vk::Result result;

	//Main Renderpass attachment info, it has 4 attachments
	// 1. Sampled Color Attachment
	// 2. Color Attachment which will contain the resolved result
	// 3. Sampled Depth Stencil Attachment
	// 4. Depth Stencil Attachment which will contain the resolved result
	vk::AttachmentDescription renderpassAttachments[4];
	vk::AttachmentDescription& sampledColorAttachment = renderpassAttachments[0];
	sampledColorAttachment.format = mSwapchainFormat;
	sampledColorAttachment.samples = vk::SampleCountFlagBits::e4;
	sampledColorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	sampledColorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
	sampledColorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	sampledColorAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	sampledColorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	sampledColorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

	vk::AttachmentDescription& colorAttachment = renderpassAttachments[1];
	colorAttachment.format = mSwapchainFormat;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

	vk::AttachmentDescription& sampledDepthAttachment = renderpassAttachments[2];
	sampledDepthAttachment.format = mDepthStencilFormat;
	sampledDepthAttachment.samples = vk::SampleCountFlagBits::e4;
	sampledDepthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	sampledDepthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	sampledDepthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	sampledDepthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	sampledDepthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eClear;
	sampledDepthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

	vk::AttachmentDescription& depthStencilAttachment = renderpassAttachments[3];
	depthStencilAttachment.format = mDepthStencilFormat;
	depthStencilAttachment.samples = vk::SampleCountFlagBits::e1;
	depthStencilAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthStencilAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	depthStencilAttachment.loadOp = vk::AttachmentLoadOp::eDontCare;
	depthStencilAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	depthStencilAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	depthStencilAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

	//color attachment is at position 0 and depth stencil attachment is at position 1
	vk::AttachmentReference sampledColorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentReference colorAttachmentRef(1, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentReference depthStencilAttachmentRef(2, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	//our renderpass only has a single subpass. Also this renderpass doesn't has a input 
	vk::SubpassDescription subpassDescription({}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &sampledColorAttachmentRef, &colorAttachmentRef, &depthStencilAttachmentRef, 0, nullptr);

	//create a renderpass now with a single subpass with attachments descriptions
	vk::RenderPassCreateInfo renderpassCreateInfo({}, 4, renderpassAttachments, 1, &subpassDescription, 0, nullptr);
	std::tie(result, mRenderpass) = mDevice.createRenderPass(renderpassCreateInfo);

	return result;
}

vk::Result
MSAAApplication::createCommandBuffers()
{
	vk::Result result;
	//create command buffer equal to the swapchain images
	vk::CommandBufferAllocateInfo renderCommandBuffersCreateInfo(mCommandPool, vk::CommandBufferLevel::ePrimary, mSwapchainImageViews.size());
	std::tie(result, mCommandBuffers) = mDevice.allocateCommandBuffers(renderCommandBuffersCreateInfo);

	vk::DescriptorPoolSize poolSize[2];
	vk::DescriptorPoolSize& uniformPoolSize = poolSize[0];
	uniformPoolSize.descriptorCount = 1;
	uniformPoolSize.type = vk::DescriptorType::eUniformBuffer;

	vk::DescriptorPoolSize& samplerPoolSize = poolSize[1];
	samplerPoolSize.descriptorCount = 1;
	samplerPoolSize.type = vk::DescriptorType::eCombinedImageSampler;

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo({}, 1, 2, poolSize);

	vk::DescriptorPool descriptorPool;
	std::tie(result, descriptorPool) = mDevice.createDescriptorPool(descriptorPoolCreateInfo);
	vk::DescriptorSetAllocateInfo allocateInfo(descriptorPool, 1, &mDescriptorSetLayout);
	std::vector<vk::DescriptorSet> sets;
	std::tie(result, sets) = mDevice.allocateDescriptorSets(allocateInfo);

	vk::DescriptorImageInfo imageInfo(mSampler, mImageView, vk::ImageLayout::eShaderReadOnlyOptimal);
	vk::DescriptorBufferInfo bufferInfo(mUniformBuffer.buffer, 0, mUniformBuffer.size);
	vk::WriteDescriptorSet writeDescriptorSet[2];
	vk::WriteDescriptorSet& bufferWrite = writeDescriptorSet[0];
	bufferWrite.dstSet = sets[0];
	bufferWrite.descriptorCount = 1;
	bufferWrite.dstArrayElement = 0;
	bufferWrite.dstBinding = 0;
	bufferWrite.descriptorType = vk::DescriptorType::eUniformBuffer; 
	bufferWrite.pBufferInfo = &bufferInfo;

	vk::WriteDescriptorSet& samplerWrite = writeDescriptorSet[1];
	samplerWrite.dstSet = sets[0];
	samplerWrite.descriptorCount = 1;
	samplerWrite.dstArrayElement = 0;
	samplerWrite.dstBinding = 1;
	samplerWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	samplerWrite.pImageInfo = &imageInfo;

	mDevice.updateDescriptorSets(2, writeDescriptorSet, 0, nullptr);

	//
	vk::Rect2D renderArea({ 0, 0 }, mSwapchainExtent);
	vk::ClearValue clearValues[3];

	vk::ClearValue& colorClear = clearValues[0];
	colorClear.color.float32[0] = 0.0f;
	colorClear.color.float32[1] = 0.0f;
	colorClear.color.float32[2] = 0.0f;
	colorClear.color.float32[3] = 0.0f;

	vk::ClearValue& resolveClear = clearValues[1];
	resolveClear.color.float32[0] = 0.0f;
	resolveClear.color.float32[1] = 0.0f;
	resolveClear.color.float32[2] = 0.0f;
	resolveClear.color.float32[3] = 0.0f;


	vk::ClearValue& depthStencilClear = clearValues[2];
	depthStencilClear.depthStencil.depth = 1.0f;
	depthStencilClear.depthStencil.stencil = 0;

	for (std::size_t i = 0; i < mSwapchainImageViews.size(); ++i) {
		vk::CommandBuffer& commandBuffer = mCommandBuffers[i];
		vk::CommandBufferBeginInfo commandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse, nullptr);
		commandBuffer.begin(commandBufferBeginInfo);

		vk::RenderPassBeginInfo renderpassBeginInfo(mRenderpass, mFramebuffers[i], renderArea, 3, clearValues);
		commandBuffer.beginRenderPass(renderpassBeginInfo, vk::SubpassContents::eInline);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline);
		commandBuffer.bindVertexBuffers(0, { mCubeBuffer.buffer }, { 0 });
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, sets, {});
		commandBuffer.draw(36, 1, 0, 0);
		commandBuffer.endRenderPass();
		commandBuffer.end();
	}

	//create a semaphore to aquire next image from the swapchain images
	std::tie(result, mImageAquireSemaphore) = mDevice.createSemaphore(vk::SemaphoreCreateInfo());
	std::tie(result, mRenderFinishSemaphore) = mDevice.createSemaphore(vk::SemaphoreCreateInfo());

	return result;
}

void
MSAAApplication::render(double frameTime, double totalTime)
{
	glm::vec3 eye(0, 2, -5);
	glm::vec3 up(0, -1, 0);
	glm::vec3 center(0, 0, 0);

	float angle = glm::quarter_pi<float>() * totalTime;
	glm::mat4x4 modelMatrix = glm::rotate(glm::mat4x4(), angle, glm::vec3(0, 1, 0));
	glm::mat4x4 lookMatrix = glm::lookAt(eye, center, up);
	glm::mat4x4 projectionMatrix = glm::perspective(glm::radians(60.0f), (float)mSwapchainExtent.width / (float)mSwapchainExtent.height, 1.0f, 100.0f);
	mMVP = projectionMatrix * lookMatrix * modelMatrix;

	void* data;
	vk::Result result;
	std::tie(result, data) = mDevice.mapMemory(mUniformBuffer.memory, mUniformBuffer.offset, mUniformBuffer.size);
	memcpy(data, &mMVP, sizeof(mMVP));
	mDevice.unmapMemory(mUniformBuffer.memory);

	uint32_t currentSwapchainImage;
	std::tie(result, currentSwapchainImage) = mDevice.acquireNextImageKHR(mSwapchain, 10000000, mImageAquireSemaphore, vk::Fence());

	vk::Semaphore waitSemaphores[] = { mImageAquireSemaphore };
	vk::Semaphore finishSemaphores[] = { mRenderFinishSemaphore };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	vk::SubmitInfo submitInfo(1, waitSemaphores, waitStages, 1, &mCommandBuffers[currentSwapchainImage], 1, finishSemaphores);
	mQueue.submit({ submitInfo }, vk::Fence());

	vk::PresentInfoKHR presentInfo(1, finishSemaphores, 1, &mSwapchain, &currentSwapchainImage);
	mQueue.presentKHR(presentInfo);
}

int main()
{
	int width = 800, height = 600;
	vk::Result result;

	MSAAApplication application;
	result = application.createInstance("MSAA", VK_MAKE_VERSION(1, 0, 0));
	result = application.createDevice();
	application.createWindow("MSAA", 800, 600);
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