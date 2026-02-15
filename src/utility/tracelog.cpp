#include "tracelog.hpp"

Tracelog::LogLevel Tracelog::tracelog_level = Tracelog::LogLevel::LL_DEBUG;

void Tracelog::SetLogLevel(LogLevel level)
{
	tracelog_level = level;
}
