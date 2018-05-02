#pragma once
#include "common.hpp"
#include "api_enums.hpp"

class IImageResource {
public:
	virtual ~IImageResource() = default;

	virtual std::vector<char> download() = 0;
	virtual void upload(const std::vector<char>& data) = 0;
	virtual bool inUse() = 0;
	virtual Format getFormat() const = 0;
	virtual uint32_t getWidth() const = 0;
	virtual uint32_t getHeight() const = 0;
};
