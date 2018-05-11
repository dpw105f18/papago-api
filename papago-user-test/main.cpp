#include "util.h"
#include "papago.hpp"

int main()
{
	auto hwnd = StartWindow(800, 600);
	auto surface = ISurface::createWin32Surface(800, 600, hwnd);

	auto& device = IDevice::enumerateDevices(*surface, {}, {})[0];
}