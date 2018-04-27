#pragma once
#include "api_enums.hpp"


class ISwapchain {
public:
	virtual ~ISwapchain() = default;

	virtual uint32_t getWidth() const = 0;
	virtual uint32_t getHeight() const = 0;
	virtual Format getFormat() const = 0;

	enum class PresentMode {
		eMailbox
	};
};