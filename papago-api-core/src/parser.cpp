#include "standard_header.hpp"
#include "parser.hpp"

Parser::Parser(const std::string & filePath)
{
	//TODO: implement proper constructor of Parser

	populateLocations();
}

size_t Parser::getLocation(const std::string & name)
{
	//TODO: error handling if [name] is not in [m_locations]
	return m_locations[name];
}

//meant for testing purposes untill Parser is properly implemented
void Parser::populateLocations()
{
	m_locations.insert({ "test", 0 });
}
