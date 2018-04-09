#pragma once
#include <string>
#include <map>

class Parser
{
public:
	Parser(const std::string& filePath);
	size_t getLocation(const std::string& name);
private:
	void populateLocations();
	std::map<std::string, size_t> m_locations;

};