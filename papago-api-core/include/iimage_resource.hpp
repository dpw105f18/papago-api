#pragma once
#include "common.hpp"

class IImageResource {
public:
	virtual ~IImageResource() = default;

	virtual void upload(const std::vector<char>& data) = 0;
};