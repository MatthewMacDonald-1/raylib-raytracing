#pragma once
#include <string>
//#include <format>
#include <iostream>
#include <cstdio>

class Tracelog {
public:
	enum LogLevel {
		LL_DEBUG = 0,
		LL_INFO = 1,
		LL_WARNING = 2,
		LL_ERROR = 3
	};

	static void SetLogLevel(LogLevel level);

	template <typename... Args>
	static void Debug(std::string message, Args&&... formatArgs)
	{
		WriteMessage(LogLevel::LL_DEBUG, message, std::forward<Args>(formatArgs)...);
	}

	template <typename... Args>
	static void Info(std::string message, Args&&... formatArgs)
	{
		WriteMessage(LogLevel::LL_INFO, message, std::forward<Args>(formatArgs)...);
	}

	template <typename... Args>
	static void Warning(std::string message, Args&&... formatArgs)
	{
		WriteMessage(LogLevel::LL_WARNING, message, std::forward<Args>(formatArgs)...);
	}
	template <typename... Args>
	static void Error(std::string message, Args&&... formatArgs)
	{
		WriteMessage(LogLevel::LL_ERROR, message, std::forward<Args>(formatArgs)...);
	}
private:
	static LogLevel tracelog_level;

	template <typename... Args>
	static void WriteMessage(LogLevel level, std::string message, Args&&... formatArgs)
	{
		if (level < tracelog_level) return;

		printf("%s: ", GetLogLevelString(level).c_str());

		printf(message.c_str(), std::forward<Args>(formatArgs)...);

		printf("\n");

		//std::string final_message = std::vformat(message, std::make_format_args(std::forward<Args&>(formatArgs)...));

		/*switch (level) {
		case 0:
			std::cout << "Debug  : " << final_message << std::endl;
			break;
		case 1:
			std::cout << "Info   : " << final_message << std::endl;
			break;
		case 2:
			std::cout << "Warning: " << final_message << std::endl;
			break;
		case 3:
			std::cout << "Error  : " << final_message << std::endl;
			break;
		}*/
	}

	static std::string GetLogLevelString(LogLevel level) {
		switch (level) {
			case 0:
				return "DEBUG  ";
				break;
			case 1:
				return "INFO   ";
				break;
			case 2:
				return "WARNING";
				break;
			case 3:
				return "ERROR  ";
				break;
		}
		return "MSG   ";
	}
};
