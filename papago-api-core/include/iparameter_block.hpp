#pragma once
#include <string>
#include <memory>
class IBufferResource;
class IDynamicBufferResource;
class IImageResource;
class ISampler;

class IParameterBlock {
public:
	virtual ~IParameterBlock() = default;
};

struct ParameterBinding {
	~ParameterBinding() {

	};

	ParameterBinding(const std::string& name, IBufferResource* buf) 
		: name(name)
		, type(0)
		, bufResource(buf)
	{};

	ParameterBinding(const std::string& name, IDynamicBufferResource* dBuf) : name(name), type(1), dBufResource(dBuf) {};
	ParameterBinding(const std::string& name, IImageResource* imgRes, ISampler* sam) : name(name), type(2), imgResource(imgRes), sampler(sam) {};
	char type;
	const std::string name;
	
	struct { IBufferResource* bufResource; };
	struct { IDynamicBufferResource* dBufResource; };
	struct { IImageResource* imgResource; ISampler* sampler; };

};