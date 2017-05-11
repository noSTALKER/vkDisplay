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

class PhongShadingApplication : public vkDisplay::Application
{
public:
	PhongShadingApplication() {}
	vk::Result createResources() override;
	vk::Result createPipeline() override;
	vk::Result createCommandBuffers() override;
	void render(double frameTime, double totalTime) override;

private:

	struct ModelInfo {
		glm::mat4x4 mvp;
		glm::mat4x4 normalMatrix;
		glm::mat4x4 modelViewMatrix;
	};

	struct LightInfo {
		glm::vec4 direction;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
	};

	struct MaterialInfo {
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
		float specularPower;
	};

	struct Info {
		LightInfo lightInfo;
		MaterialInfo matInfo;
	};

	std::vector<vk::CommandBuffer> mCommandBuffers;
	vkDisplay::Model mModel;
	vkDisplay::Buffer mModelBuffer;
	vkDisplay::Buffer mUniformBuffer;
	vk::Pipeline mPipeline;
	vk::PipelineLayout mPipelineLayout;
	vk::DescriptorSetLayout mDescriptorSetLayout;
	vk::Semaphore mRenderFinishSemaphore;
	vk::Semaphore mImageAquireSemaphore;
	ModelInfo mModelInfo;
	Info mInfo;
};

vk::Result
PhongShadingApplication::createResources()
{
	vk::Result result;

	mUniformBuffer = createCoherantBuffer(&mModelInfo, sizeof(mModelInfo), vk::BufferUsageFlagBits::eUniformBuffer);

	//buffer data
	mModel = createModel("../models/dragon/dragon.obj");

	mModelBuffer = createDeviceBuffer(mModel.data.data(),
		mModel.data.size() * sizeof(float),
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);

	return vk::Result::eSuccess;
}

vk::Result
PhongShadingApplication::createPipeline()
{
	vk::Result result;
	uint32_t stride = 3;
	if (mModel.hasNormal)
		stride += 3;

	vk::VertexInputBindingDescription vertexBinding(0, sizeof(float) * stride, vk::VertexInputRate::eVertex);

	//two vertex input attachments, position at location 0 
	vk::VertexInputAttributeDescription vertexAttributes[2];
	vk::VertexInputAttributeDescription& positionAttribute = vertexAttributes[0];
	positionAttribute.format = vk::Format::eR32G32B32Sfloat;
	positionAttribute.location = 0;
	positionAttribute.offset = 0;
	positionAttribute.binding = vertexBinding.binding;

	vk::VertexInputAttributeDescription& normalAttribute = vertexAttributes[1];
	normalAttribute.format = vk::Format::eR32G32B32Sfloat;
	normalAttribute.location = 1;
	normalAttribute.offset = 3 * sizeof(float);
	normalAttribute.binding = vertexBinding.binding;

	vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eFragment, 0, sizeof(Info));

	//
	vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding[1];
	vk::DescriptorSetLayoutBinding& uniformLayoutBinding = descriptorSetLayoutBinding[0];
	uniformLayoutBinding.binding = 0;
	uniformLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	uniformLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
	uniformLayoutBinding.descriptorCount = 1;
	
	vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo({}, 1, descriptorSetLayoutBinding);
	std::tie(result, mDescriptorSetLayout) = mDevice.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);

	std::fstream vertexFile("../shaders/PhongShading/basic.vert.spv", std::ios::binary | std::ios::in);
	vertexFile.seekg(0, std::ios::end);
	std::size_t vertexSize = vertexFile.tellg();
	vertexFile.seekg(0, std::ios::beg);

	uint32_t* vertexSPV = new uint32_t[vertexSize / sizeof(uint32_t)];
	vertexFile.read(reinterpret_cast<char*>(vertexSPV), vertexSize);

	vertexFile.close();

	std::fstream fragmentFile("../shaders/PhongShading/basic.frag.spv", std::ios::binary | std::ios::in);
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
PhongShadingApplication::createCommandBuffers()
{
	mInfo.matInfo.ambient = glm::vec4(0.05, 0.05, 0.05, 1);
	mInfo.matInfo.diffuse = glm::vec4(0.5, 0, 0, 1);
	mInfo.matInfo.specular = glm::vec4(1, 1, 1, 1);
	mInfo.matInfo.specularPower = 3;

	mInfo.lightInfo.ambient = glm::vec4(1, 1, 1, 1);
	mInfo.lightInfo.diffuse = glm::vec4(1, 1, 1, 1);
	mInfo.lightInfo.specular = glm::vec4(1, 1, 1, 1);
	
	glm::vec3 eye(0, 0, -2);
	glm::vec3 up(0, -1, 0);
	glm::vec3 center(0, 0, 0);

	glm::mat4x4 lookMatrix = glm::lookAt(eye, center, up);
	glm::vec4 lightDirection = glm::vec4(0, -1, 1, 0);
	lightDirection = glm::normalize(lightDirection);
	mInfo.lightInfo.direction = lookMatrix * lightDirection;
	mInfo.lightInfo.direction = glm::normalize(mInfo.lightInfo.direction);

	vk::Result result;
	//create command buffer equal to the swapchain images
	vk::CommandBufferAllocateInfo renderCommandBuffersCreateInfo(mCommandPool, vk::CommandBufferLevel::ePrimary, mSwapchainImageViews.size());
	std::tie(result, mCommandBuffers) = mDevice.allocateCommandBuffers(renderCommandBuffersCreateInfo);

	vk::DescriptorPoolSize poolSize[1];
	vk::DescriptorPoolSize& uniformPoolSize = poolSize[0];
	uniformPoolSize.descriptorCount = 1;
	uniformPoolSize.type = vk::DescriptorType::eUniformBuffer;

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo({}, 1, 1, poolSize);

	vk::DescriptorPool descriptorPool;
	std::tie(result, descriptorPool) = mDevice.createDescriptorPool(descriptorPoolCreateInfo);
	vk::DescriptorSetAllocateInfo allocateInfo(descriptorPool, 1, &mDescriptorSetLayout);
	std::vector<vk::DescriptorSet> sets;
	std::tie(result, sets) = mDevice.allocateDescriptorSets(allocateInfo);

	vk::DescriptorBufferInfo bufferInfo(mUniformBuffer.buffer, 0, mUniformBuffer.size);
	vk::WriteDescriptorSet writeDescriptorSet[1];
	vk::WriteDescriptorSet& bufferWrite = writeDescriptorSet[0];
	bufferWrite.dstSet = sets[0];
	bufferWrite.descriptorCount = 1;
	bufferWrite.dstArrayElement = 0;
	bufferWrite.dstBinding = 0;
	bufferWrite.descriptorType = vk::DescriptorType::eUniformBuffer; 
	bufferWrite.pBufferInfo = &bufferInfo;

	mDevice.updateDescriptorSets(1, writeDescriptorSet, 0, nullptr);

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

		commandBuffer.pushConstants(mPipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, sizeof(mInfo), &mInfo);

		vk::RenderPassBeginInfo renderpassBeginInfo(mRenderpass, mFramebuffers[i], renderArea, 2, clearValues);
		commandBuffer.beginRenderPass(renderpassBeginInfo, vk::SubpassContents::eInline);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline);
		commandBuffer.bindVertexBuffers(0, { mModelBuffer.buffer }, { 0 });
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, sets, {});
		commandBuffer.draw(mModel.vertexCount, 1, 0, 0);
		commandBuffer.endRenderPass();
		commandBuffer.end();
	}

	//create a semaphore to aquire next image from the swapchain images
	std::tie(result, mImageAquireSemaphore) = mDevice.createSemaphore(vk::SemaphoreCreateInfo());
	std::tie(result, mRenderFinishSemaphore) = mDevice.createSemaphore(vk::SemaphoreCreateInfo());

	return result;
}

void
PhongShadingApplication::render(double frameTime, double totalTime)
{
	glm::vec3 eye(0, 0, -2);
	glm::vec3 up(0, -1, 0);
	glm::vec3 center(0, 0, 0);

	float angle = glm::quarter_pi<float>() * totalTime;
	glm::mat4x4 modelMatrix = glm::rotate(glm::mat4x4(), angle, glm::vec3(0, 1, 0));
	glm::mat4x4 lookMatrix = glm::lookAt(eye, center, up);
	glm::mat4x4 projectionMatrix = glm::perspective(glm::radians(60.0f), (float)mSwapchainExtent.width / (float)mSwapchainExtent.height, 0.01f, 100.0f);
	mModelInfo.mvp = projectionMatrix * lookMatrix * modelMatrix;
	mModelInfo.normalMatrix = glm::transpose(glm::inverse(mModelInfo.normalMatrix));
	mModelInfo.modelViewMatrix = lookMatrix * modelMatrix;

	void* data = nullptr;
	vk::Result result;
	std::tie(result, data) = mDevice.mapMemory(mUniformBuffer.memory, mUniformBuffer.offset, mUniformBuffer.size);
	memcpy(data, &mModelInfo, sizeof(mModelInfo));
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

	PhongShadingApplication application;
	result = application.createInstance("Phong Shading", VK_MAKE_VERSION(1, 0, 0));
	result = application.createDevice();
	application.createWindow("Phong Shading", 800, 600);
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