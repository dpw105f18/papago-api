#pragma once
#include "common.hpp"

class IImageResource {
public:
	~IImageResource() = default;

	virtual void upload(const std::vector<char>& data) = 0;
};