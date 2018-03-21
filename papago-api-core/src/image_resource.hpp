#pragma once
#include "standard_header.hpp"
#include "resource.hpp"
#include "device.hpp"

class ImageResource : public Resource
{
public:

	// Inherited via Resource
	void upload(std::vector<char> data) override;
	void destroy() override;
	void* download(void * buffer, size_t size, size_t offset) override;

private:
	ImageResource();
	friend ImageResource Device::createImageResource(size_t width, size_t height, TypeEnums, ImageType);
};