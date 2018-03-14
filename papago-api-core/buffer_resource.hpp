#pragma once
#include "resource.hpp"

class BufferResource : public Resource
{
	// Inherited via Resource
	virtual void upload(std::vector<char> data) override;
	virtual void destroy() override;
	virtual void download() override;
};