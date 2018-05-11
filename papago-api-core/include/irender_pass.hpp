#pragma once

class IBufferResource;
class IDynamicBufferResource;
class IImageResource;
class ISampler;

class IRenderPass {
public:
	virtual ~IRenderPass() = default;

	virtual void bindResource(const std::string& name, IBufferResource&) = 0;
	virtual void bindResource(const std::string& name, IDynamicBufferResource&) = 0;
	virtual void bindResource(const std::string& name, IImageResource&, ISampler&) = 0;
};
