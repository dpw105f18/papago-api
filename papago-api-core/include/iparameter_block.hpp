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

union ParameterBinding {
	~ParameterBinding() {

	};
	
	ParameterBinding(const std::string& name, std::reference_wrapper<IBufferResource> buf) : name(name), type(0), bufResource(buf) {};
	ParameterBinding(const std::string& name, std::reference_wrapper<IDynamicBufferResource> dBuf) : name(name), type(1), dBufResource(dBuf) {};
	ParameterBinding(const std::string& name, std::reference_wrapper<IImageResource> imgRes, std::reference_wrapper<ISampler> sam) : name(name), type(2), imgResource(imgRes), sampler(sam) {};

	struct { char type; std::string name;  std::reference_wrapper<IBufferResource> bufResource; };
	struct { char type; std::string name; std::reference_wrapper<IDynamicBufferResource> dBufResource; };
	struct { char type; std::string name; std::reference_wrapper<IImageResource> imgResource; std::reference_wrapper<ISampler> sampler; };
};