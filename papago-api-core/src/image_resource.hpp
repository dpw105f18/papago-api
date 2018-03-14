#pragma once
#include "resource.hpp"

class ImageResource : public Resource
{
public:
	enum ImageType {
		IMAGE_DEPTH_BUFFER
	};

	// Inherited via Resource
	virtual void upload(std::vector<char> data) override;
	virtual void destroy() override;
	virtual void download() override;
};