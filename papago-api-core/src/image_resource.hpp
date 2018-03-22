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
	std::vector<char> download() override;

private:
	ImageResource();
	friend ImageResource Device::createImageResource(size_t width, size_t height, TypeEnums, ImageType);
};