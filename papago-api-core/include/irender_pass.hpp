#pragma once

class IBufferResource;
class DynamicBuffer;
class IImageResource;
class ISampler;

class IRenderPass {
public:
	virtual ~IRenderPass() = default;

	virtual void bindResource(const std::string& name, IBufferResource&) = 0;
	virtual void bindResource(const std::string& name, DynamicBuffer&) = 0;
	virtual void bindResource(const std::string& name, IImageResource&, ISampler&) = 0;
};
