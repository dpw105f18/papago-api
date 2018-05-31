#pragma once
#include <iostream>
#include <ostream>

enum class LogLevel
{
	eError			= 0,
	eWarning		= 1,
	eInformation	= 2,
	eDebug			= 3,
	eVerbose		= 4,
};

inline std::ostream& operator <<(std::ostream& stream, const LogLevel& level)
{
	switch(level) { 
		case LogLevel::eVerbose:		stream << "Verbose";		break;
		case LogLevel::eDebug:			stream << "Debug";			break;
		case LogLevel::eInformation:	stream << "Information";	break;
		case LogLevel::eWarning:		stream << "Warning";		break;
		case LogLevel::eError:			stream << "Error";			break;
	}
	return stream;
}

class Logger
{
	Logger() = default; // hide constructor

	std::ostream* m_stream = &std::cout;
	LogLevel m_logLevel = LogLevel::eInformation;
	bool m_enabled = false;
public:
	static Logger& instance() 
	{
		static Logger instance;
		return instance;
	}

	void setOutput(std::ostream& stream) { m_stream = &stream; }

	void setLogLevel(LogLevel log_level) { m_logLevel = log_level; }

	void setLogEnabled(bool log_enabled) { m_enabled = log_enabled; }

	void log(const LogLevel level, const std::string& message) const
	{
		if(m_enabled && !m_stream && m_logLevel < level) return;
		*m_stream << level << ": " << message << std::endl;
	}
};
