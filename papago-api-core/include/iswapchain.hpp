#pragma once
class ISwapchain {
public:
	virtual ~ISwapchain() = default;

	enum class PresentMode {
		eMailbox
	};
};