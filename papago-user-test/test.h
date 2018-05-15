#pragma once
#include <Windows.h>
#include <memory>
#include "external\papago\papago.hpp"

class Test
{
public:
	void Init(const HWND &win);
	void Loop();
private:
	//TODO: add state here:
	std::unique_ptr<IGraphicsQueue> GQ = nullptr;
};