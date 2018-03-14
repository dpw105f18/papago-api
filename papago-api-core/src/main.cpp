#include "standard_header.hpp"
#include "device.hpp"

int main()
{
	/*
	std::cout << "Hello, world!" << std::endl;
	auto instance = vk::createInstance(vk::InstanceCreateInfo());
	std::cout << "Created a vulkan instance." << std::endl;
	char ch;
	std::cin >> ch;

	instance.destroy();
	*/

	Device::enumerateDevices();

	std::system("PAUSE");
}