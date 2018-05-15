#pragma once
#include <fstream>
#include <vector>

//STB - for image loading
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb/stb_image.h"

static std::string readFile(const std::string& file_path) {
	std::ifstream stream(file_path, std::ios::ate);
	auto size = stream.tellg();
	stream.seekg(0);
	std::string result = std::string(size, '\0');
	stream.read(&result[0], size);
	return result;
}

static std::vector<char> readPixels()
{
	int texWidth, texHeight, texChannels;
	auto pixels = stbi_load("textures/eldorado.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels) {
		throw new std::runtime_error("Failed to load texture image!");
	}

	auto result = std::vector<char>();
	result.resize(texWidth * texHeight * 4);
	memcpy(result.data(), pixels, result.size());
	stbi_image_free(pixels);

	return result;
}