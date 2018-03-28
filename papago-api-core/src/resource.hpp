#pragma once
#include <vector>

class Resource
{
public:
	virtual void upload(const std::vector<char>& data) = 0;
	virtual void destroy() = 0;
	virtual std::vector<char> download() = 0;
};