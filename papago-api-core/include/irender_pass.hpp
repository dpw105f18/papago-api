#pragma once

class IBufferResource;
class IDynamicBuffer;
class IImageResource;
class ISampler;

class IRenderPass {
public:
	virtual ~IRenderPass() = default;

	virtual void bindResource(const std::string& name, IBufferResource&) = 0;
	virtual void bindResource(const std::string& name, IDynamicBuffer&) = 0;
	virtual void bindResource(const std::string& name, IImageResource&, ISampler&) = 0;
};
