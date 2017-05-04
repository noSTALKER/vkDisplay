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
#include "Util/Matrix4x4.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class InstancingApplication : public vkDisplay::Application
{
public:
	InstancingApplication() {}
	vk::Result createResources() override;
	vk::Result createPipeline() override;
	vk::Result createCommandBuffers() override;
	void render(double frameTime, double totalTime) override;
private:
	std::vector<vk::CommandBuffer> mCommandBuffers;
	vkDisplay::Buffer mCubeBuffer;
	vk::Pipeline mPipeline;
	vk::PipelineLayout mPipelineLayout;
	vk::Image mImage;
	vk::ImageView mImageView;
	vk::Sampler mSampler;
	vk::DescriptorSetLayout mDescriptorSetLayout;
	vk::Semaphore mRenderFinishSemaphore;
	vk::Semaphore mImageAquireSemaphore;
};

vk::Result
InstancingApplication::createResources()
{
	vk::Result result;

	//buffer data
	struct BufferData {
		float vertexData[180] = {
		 // left face
		 -1, 1, 1, 0.f, 1.f,    // lft-btm-back
		 -1, -1, -1, 1.f, 0.f,  // lft-top-front
		 -1, -1, 1, 0.f, 0.f,   // lft-top-back
		 -1, -1, -1,1.f, 0.f,  // lft-top-front
		 -1, 1, 1, 0.f, 1.f,    // lft-btm-back
		 -1, 1, -1, 1.f, 1.f,   // lft-btm-front
		// front face
		 1, -1, -1, 1.f, 0.f,   // rgt-top-front
		 -1, -1, -1, 0.f, 0.f,  // lft-top-front
		 1, 1, -1, 1.f, 1.f,    // rgt-btm-front
		 1, 1, -1, 1.f, 1.f,    // rgt-btm-front
		 -1, -1, -1, 0.f, 0.f,  // lft-top-front
		 -1, 1, -1, 0.f, 1.f,   // lft-btm-front
	     // top face
		 1, -1, 1, 1.f, 0.f,    // rgt-top-back
		 -1, -1, -1, 0.f, 1.f,  // lft-top-front
		 1, -1, -1, 1.f, 1.f,   // rgt-top-front
		 -1, -1, 1, 0.f, 0.f,   // lft-top-back
		 -1, -1, -1, 0.f, 1.f,  // lft-top-front
		 1, -1, 1, 1.f, 0.f,    // rgt-top-back
		 // bottom face
		 1, 1, 1, 1.f, 1.f,    // rgt-btm-back
		 -1, 1, -1, 0.f, 0.f,  // lft-btm-front
		 -1, 1, 1, 0.f, 1.f,   // lft-btm-back
		 1, 1, -1, 1.f, 0.f,   // rgt-btm-front
		 -1, 1, -1, 0.f, 0.f,  // lft-btm-front
		 1, 1, 1, 1.f, 1.f,    // rgt-btm-back
		// right face
		 1, -1, 1, 1.f, 0.f,   // rgt-top-back
		 1, 1, -1, 0.f, 1.f,   // rgt-btm-front
		 1, 1, 1, 1.f, 1.f,    // rgt-btm-back
		 1, 1, -1, 0.f, 1.f,   // rgt-btm-front
		 1, -1, 1, 1.f, 0.f,   // rgt-top-back
		 1, -1, -1, 0.f, 0.f,  // rgt-top-front
		// back face
		 1, 1, 1, 0.f, 1.f,    // rgt-btm-back
		 -1, 1, 1, 1.f, 1.f,   // lft-btm-back
		 -1, -1, 1, 1.f, 0.f,  // lft-top-back
		 1, 1, 1, 0.f, 1.f,    // rgt-btm-back
		 -1, -1, 1, 1.f, 0.f,  // lft-top-back
		 1, -1, 1, 0.f, 0.f };   // rgt-top-back
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

	return vk::Result::eSuccess;
}

vk::Result
InstancingApplication::createPipeline()
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

	vk::DescriptorSetLayoutBinding samplerLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo({}, 1, &samplerLayoutBinding);
	std::tie(result, mDescriptorSetLayout) = mDevice.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

	std::fstream vertexFile("../shaders/Instancing/basic.vert.spv", std::ios::binary | std::ios::in);
	vertexFile.seekg(0, std::ios::end);
	std::size_t vertexSize = vertexFile.tellg();
	vertexFile.seekg(0, std::ios::beg);

	uint32_t* vertexSPV = new uint32_t[vertexSize / sizeof(uint32_t)];
	vertexFile.read(reinterpret_cast<char*>(vertexSPV), vertexSize);

	vertexFile.close();

	std::fstream fragmentFile("../shaders/Instancing/basic.frag.spv", std::ios::binary | std::ios::in);
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

	//Push Constant Info
	vk::PushConstantRange pushConstantRange{ vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4x4) * 3};

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
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo({}, 1, &mDescriptorSetLayout, 1, &pushConstantRange);
	std::tie(result, mPipelineLayout) = mDevice.createPipelineLayout(pipelineLayoutCreateInfo);

	vk::GraphicsPipelineCreateInfo pipelineCreateInfo({}, 2, shaderStageInfos, &vertexInputInfo, &inputAssemblyInfo, nullptr, &viewportInfo, &rasterizationInfo, &multisampleInfo, &depthStencilInfo, &colorBlendInfo, nullptr, mPipelineLayout, mRenderpass, 0);

	std::tie(result, mPipeline) = mDevice.createGraphicsPipeline(vk::PipelineCache(), pipelineCreateInfo);

	return result;
}

vk::Result
InstancingApplication::createCommandBuffers()
{
	vk::Result result;

	const glm::vec3 eye(0.f, 0.f, -10.f);
	const glm::vec3 up(0.f, 1.f, 0.f);
	const glm::vec3 center(0.f, 0.f, 0.f);

	glm::mat4x4 lookMatrix = glm::lookAt(eye, center, up);
	glm::mat4x4 projectionMatrix = glm::perspective(glm::radians(60.0f), (float)mSwapchainExtent.width / (float)mSwapchainExtent.height, 1.0f, 100.0f);
	glm::mat4x4 VP = projectionMatrix * lookMatrix;

	constexpr float angle = glm::radians(45.0f);
	glm::mat4x4 modelMatrix1 = glm::rotate(glm::mat4x4(), angle, glm::vec3(0.f, 1.f, 0.f));

	glm::mat4x4 modelMatrix2 = glm::translate(glm::mat4x4(), glm::vec3(-4.f, 0.f, 0.f));
	modelMatrix2 = glm::rotate(modelMatrix2, angle, glm::vec3(1.f, 0.f, 0.f));
	

	glm::mat4x4 modelMatrix3 = glm::translate(glm::mat4x4(), glm::vec3(4.f, 0.f, 0.f));
	modelMatrix3 = glm::rotate(modelMatrix3, angle, glm::vec3(0.f, 0.f, 1.f));
	

	glm::mat4x4 MVPs[3];
	MVPs[0] = VP * modelMatrix1;
	MVPs[1] = VP * modelMatrix2;
	MVPs[2] = VP * modelMatrix3;


	//create command buffer equal to the swapchain images
	vk::CommandBufferAllocateInfo renderCommandBuffersCreateInfo(mCommandPool, vk::CommandBufferLevel::ePrimary, mSwapchainImageViews.size());
	std::tie(result, mCommandBuffers) = mDevice.allocateCommandBuffers(renderCommandBuffersCreateInfo);

	vk::DescriptorPoolSize samplerPoolSize;
	samplerPoolSize.descriptorCount = 1;
	samplerPoolSize.type = vk::DescriptorType::eCombinedImageSampler;

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo({}, 1, 1, &samplerPoolSize);

	vk::DescriptorPool descriptorPool;
	std::tie(result, descriptorPool) = mDevice.createDescriptorPool(descriptorPoolCreateInfo);
	vk::DescriptorSetAllocateInfo allocateInfo(descriptorPool, 1, &mDescriptorSetLayout);
	std::vector<vk::DescriptorSet> sets;
	std::tie(result, sets) = mDevice.allocateDescriptorSets(allocateInfo);

	vk::DescriptorImageInfo imageInfo(mSampler, mImageView, vk::ImageLayout::eShaderReadOnlyOptimal);
	vk::WriteDescriptorSet samplerWrite(sets[0], 0, 0, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo);
	mDevice.updateDescriptorSets(1, &samplerWrite, 0, nullptr);

	//
	vk::Rect2D renderArea({ 0, 0 }, mSwapchainExtent);
	vk::ClearValue clearValues[2];

	vk::ClearValue& colorClear = clearValues[0];
	colorClear.color.float32[0] = 0.0f;
	colorClear.color.float32[1] = 0.0f;
	colorClear.color.float32[2] = 0.0f;
	colorClear.color.float32[3] = 0.0f;

	vk::ClearValue& depthStencilClear = clearValues[1];
	depthStencilClear.depthStencil.depth = 1.0f;
	depthStencilClear.depthStencil.stencil = 0;

	for (std::size_t i = 0; i < mSwapchainImageViews.size(); ++i) {
		vk::CommandBuffer& commandBuffer = mCommandBuffers[i];
		vk::CommandBufferBeginInfo commandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse, nullptr);
		commandBuffer.begin(commandBufferBeginInfo);

		//record the push constants
		commandBuffer.pushConstants(mPipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4x4) * 3, MVPs);

		vk::RenderPassBeginInfo renderpassBeginInfo(mRenderpass, mFramebuffers[i], renderArea, 2, clearValues);
		commandBuffer.beginRenderPass(renderpassBeginInfo, vk::SubpassContents::eInline);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline);
		commandBuffer.bindVertexBuffers(0, { mCubeBuffer.buffer }, { 0 });
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, sets, {});
		commandBuffer.draw(36, 3, 0, 0);
		commandBuffer.endRenderPass();
		commandBuffer.end();
	}

	//create a semaphore to aquire next image from the swapchain images
	std::tie(result, mImageAquireSemaphore) = mDevice.createSemaphore(vk::SemaphoreCreateInfo());
	std::tie(result, mRenderFinishSemaphore) = mDevice.createSemaphore(vk::SemaphoreCreateInfo());

	return result;
}

void
InstancingApplication::render(double frameTime, double totalTime)
{
	vk::Result result;

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

	InstancingApplication application;
	result = application.createInstance("Push Constant", VK_MAKE_VERSION(1, 0, 0));
	result = application.createDevice();
	application.createWindow("Push Constant", 800, 600);
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