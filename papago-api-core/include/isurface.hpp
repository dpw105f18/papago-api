#pragma once
#include "common.hpp"

class ISurface {
public:
	virtual ~ISurface() = default;

	size_t getWidth() const { return m_width; }
	size_t getHeight() const { return m_height; }

	PAPAGO_API static std::unique_ptr<ISurface> createWin32Surface(size_t, size_t, HWND);

protected:
	ISurface(size_t width, size_t height) : m_width(width), m_height(height) { }

private:
	size_t m_width;
	size_t m_height;
};