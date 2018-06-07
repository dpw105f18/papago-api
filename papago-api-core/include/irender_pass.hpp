#pragma once

class IBufferResource;
class IDynamicBufferResource;
class IImageResource;
class ISampler;

class IRenderPass {
public:
	virtual ~IRenderPass() = default;
};
