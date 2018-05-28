#include "standard_header.hpp"
#include "parameter_block.hpp"
#include "render_pass.hpp"
#include "buffer_resource.hpp"
#include "image_resource.hpp"
#include "sampler.hpp"

ParameterBlock::ParameterBlock(const vk::UniqueDevice& device, RenderPass & renderPass, std::vector<ParameterBinding>& bindings)
	: m_mask(0), m_renderPass(renderPass)
{
	for (auto& binding : bindings) {
		auto bit = renderPass.getBinding(binding.name);
		if (binding.type == 1) {
			m_mask |= (0x01 << bit);
			++m_dynamicBufferCount;
			m_namedAlignments[binding.name] = dynamic_cast<DynamicBufferResource&>(*binding.dBufResource).m_alignment;
		}
		else {
			m_namedAlignments[binding.name] = 0;
		}
	}

	if (renderPass.m_vkDescriptorSetLayouts.find(m_mask) == renderPass.m_vkDescriptorSetLayouts.end()) {
		renderPass.cacheNewPipeline(m_mask);
	}

	makeVkDescriptorSet(device, bindings);

	bindResources(device, bindings);
}

void ParameterBlock::makeVkDescriptorSet(const vk::UniqueDevice& device, std::vector<ParameterBinding>& bindings)
{
	//Descriptor Pool:
	auto poolSizes = std::vector<vk::DescriptorPoolSize>(bindings.size());
	for (auto i = 0; i < bindings.size(); ++i) {
		auto& binding = bindings[i];
		vk::DescriptorType type;

		switch (binding.type) {
		case 0:
			type = vk::DescriptorType::eUniformBuffer;
			break;
		case 1:
			type = vk::DescriptorType::eUniformBufferDynamic;
			break;
		case 2:
			type = vk::DescriptorType::eCombinedImageSampler;
			break;
		default:
			//TODO: log error
			break;
		}

		poolSizes[i].setDescriptorCount(1)
			.setType(type);
	}

	vk::DescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.setPoolSizeCount(poolSizes.size())
		.setPPoolSizes(poolSizes.data())
		.setMaxSets(1)	//TODO: keep this default value? -AM.
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

	m_vkPool = std::move(device->createDescriptorPoolUnique(poolCreateInfo));

	//Descriptor Set:
	vk::DescriptorSetAllocateInfo allocateInfo = {};
	allocateInfo.setDescriptorPool(*m_vkPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(&m_renderPass.getDescriptorSetLayout(m_mask).get());

	m_vkDescriptorSet = std::move(device->allocateDescriptorSetsUnique(allocateInfo)[0]);
}

void ParameterBlock::bindResources(const vk::UniqueDevice & device, std::vector<ParameterBinding>& bindings)
{
	std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
	std::vector<vk::DescriptorBufferInfo> bufferInfos;
	bufferInfos.reserve(bindings.size());
	std::vector<vk::DescriptorImageInfo> imageInfos;
	imageInfos.reserve(bindings.size());

	for (auto i = 0; i < bindings.size(); ++i) {
		
		auto& binding = bindings[i];
		switch (binding.type)
		{
		case 0:
			bufferInfos.push_back({});
			writeDescriptorSets.emplace_back(createWriteDescriptorSet(
				device, 
				bufferInfos.back(),
				binding.name, 
				dynamic_cast<BufferResource&>(*binding.bufResource))
			);
			break;
		case 1:
			bufferInfos.push_back({});
			writeDescriptorSets.emplace_back(createWriteDescriptorSet(
				device, 
				bufferInfos.back(),
				binding.name, 
				dynamic_cast<DynamicBufferResource&>(*binding.dBufResource))
			);
			break;
		case 2:
			imageInfos.push_back({});
			writeDescriptorSets.emplace_back(createWriteDescriptorSet(
				device, 
				imageInfos.back(),
				binding.name, 
				dynamic_cast<ImageResource&>(*binding.imgResource), 
				dynamic_cast<Sampler&>(*binding.sampler))
			);
			break;
		default:
			//TODO: log error.
			break;
		}
	}

	device->updateDescriptorSets(writeDescriptorSets, {});
}

vk::WriteDescriptorSet ParameterBlock::createWriteDescriptorSet(const vk::UniqueDevice & device, vk::DescriptorBufferInfo& info, const std::string & name, BufferResource & buffer)
{
	info = buffer.m_vkInfo;
	info.setOffset(m_renderPass.m_shaderProgram.getOffset(name));

	auto writeDescriptorSet = vk::WriteDescriptorSet()
		.setDstSet(*m_vkDescriptorSet)
		.setDstBinding(m_renderPass.getBinding(name))
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setPBufferInfo(&info);

	return writeDescriptorSet;
}

vk::WriteDescriptorSet ParameterBlock::createWriteDescriptorSet(const vk::UniqueDevice & device, vk::DescriptorBufferInfo& info, const std::string & name, DynamicBufferResource & buffer)
{
	auto& internalBuffer = dynamic_cast<BufferResource&>(*buffer.m_buffer);

	info = internalBuffer.m_vkInfo;
	info.setRange(buffer.m_alignment);

	auto writeDescriptorSet = vk::WriteDescriptorSet()
		.setDstSet(*m_vkDescriptorSet)
		.setDstBinding(m_renderPass.getBinding(name))
		.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
		.setDescriptorCount(1)
		.setPBufferInfo(&info);

	return writeDescriptorSet;
}

vk::WriteDescriptorSet ParameterBlock::createWriteDescriptorSet(const vk::UniqueDevice & device, vk::DescriptorImageInfo& info, const std::string & name, ImageResource & image, Sampler & sampler)
{
	auto& backendImage = dynamic_cast<ImageResource&>(image);
	auto binding = m_renderPass.getBinding(name);

	info = {};
	info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setImageView(*backendImage.m_vkImageView)
		.setSampler(static_cast<vk::Sampler>(sampler));

	auto writeDescriptorSet = vk::WriteDescriptorSet()
		.setDstSet(*m_vkDescriptorSet)
		.setDstBinding(binding)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setPImageInfo(&info);

	return writeDescriptorSet;
}
