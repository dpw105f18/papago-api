#pragma once
#include <vector>

class Resource
{
public:
	virtual void upload(std::vector<char> data) = 0;
	virtual void destroy() = 0;
	virtual void* download(void* buffer, size_t size, size_t offset) = 0;
};