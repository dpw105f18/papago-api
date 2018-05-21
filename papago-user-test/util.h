#pragma once
#include <fstream>
#include <vector>

//STB - for image loading
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"

static std::string readFile(const std::string& file_path) {
	
	std::ifstream stream(file_path, std::ios::ate);
	if (stream.good()) {

		auto size = stream.tellg();
		stream.seekg(0);
		std::string result = std::string(size, '\0');
		stream.read(&result[0], size);
		return result;
	}
	else {
		auto msg = "Error reading file \"" + file_path + "\". Does the file exist?";
		throw std::runtime_error(msg);
	}
}

//Returns pixel data in a std::vector<char>. Pixels are loaded in a RGBA format.
//the height and width of the image is provided through the parameters outWidth and outHeight.
static std::vector<char> readPixels(const std::string& localPath, int& outWidth, int& outHeight)
{
	int texChannels;
	auto pixels = stbi_load(localPath.c_str(), &outWidth, &outHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("Failed to load texture image!");
	}

	auto result = std::vector<char>();
	result.resize(outWidth * outHeight * 4);
	memcpy(result.data(), pixels, result.size());
	stbi_image_free(pixels);

	return result;
}