#pragma once
#include <vector>

class Resource
{
public:
	virtual void upload(std::vector<char> data) = 0;
	virtual void destroy() = 0;
	virtual void download() = 0;
};