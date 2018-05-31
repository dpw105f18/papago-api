#pragma once
#include <string>
class IBufferResource;
class IDynamicBufferResource;
class IImageResource;
class ISampler;

enum class BindingType 
{ 
  eBufferResource, 
  eDynamicBufferResource, 
  eCombinedImageSampler 
}; 

class IParameterBlock {
public:
	virtual ~IParameterBlock() = default;
};

struct ParameterBinding {
	~ParameterBinding() { }

	ParameterBinding(const std::string& name, IBufferResource* buf) 
		: type(BindingType::eBufferResource)
		, name(name)
		, bufResource(buf)
	{ }

	ParameterBinding(const std::string& name, IDynamicBufferResource* dBuf) 
		: type(BindingType::eDynamicBufferResource)
		, name(name)
		, dBufResource(dBuf) 
	{ }
	
	ParameterBinding(const std::string& name, IImageResource* imgRes, ISampler* sam) 
		: type(BindingType::eCombinedImageSampler)
		, name(name)
		, imgResource(imgRes)
		, sampler(sam) 
	{ }
	BindingType type;
	const std::string name;
	union{
		IBufferResource* bufResource; 
		IDynamicBufferResource* dBufResource; 
		struct { IImageResource* imgResource; ISampler* sampler; };
	};

};
