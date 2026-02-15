#include "utility-json.hpp"

#include "utility-exceptions.hpp"

std::string Utility::JSON::ReadStringRequired(nlohmann::json json, std::string key)
{
	if (!json.contains(key))
	{
		std::stringstream ss;
		ss << "Unable to parse JSON. Missing key '" << key << "'.";
		throw JSONParseException(ss.str());
	}

	if (!json[key].is_string()) {
		std::stringstream ss;
		ss << "Unable to parse JSON. Value for key '" << key << "' was not a valid string.";
		throw JSONParseException(ss.str());
	}

	return json[key].template get<std::string>();
}

std::string Utility::JSON::ReadStringOptional(nlohmann::json json, std::string key, std::string defaultValue)
{
	if (!json.contains(key))
	{
		return defaultValue;
	}

	if (!json[key].is_string()) {
		std::stringstream ss;
		ss << "Unable to parse JSON. Value for key '" << key << "' was not a valid string.";
		throw JSONParseException(ss.str());
	}

	return json[key].template get<std::string>();
}

int Utility::JSON::ReadIntegerRequired(nlohmann::json json, std::string key)
{
	if (!json.contains(key))
	{
		std::stringstream ss;
		ss << "Unable to parse JSON. Missing key '" << key << "'.";
		throw JSONParseException(ss.str());
	}

	if (!json[key].is_number_integer()) {
		std::stringstream ss;
		ss << "Unable to parse JSON. Value for key '" << key << "' was not a valid integer.";
		throw JSONParseException(ss.str());
	}

	return json[key].get<int>();
}

int Utility::JSON::ReadIntegerOptional(nlohmann::json json, std::string key, int defaultValue)
{
	if (!json.contains(key))
	{
		return defaultValue;
	}

	if (!json[key].is_number_integer()) {
		std::stringstream ss;
		ss << "Unable to parse JSON. Value for key '" << key << "' was not a valid integer.";
		throw JSONParseException(ss.str());
	}

	return json[key].get<int>();
}

bool Utility::JSON::ReadBoolRequired(nlohmann::json json, std::string key)
{
	if (!json.contains(key))
	{
		std::stringstream ss;
		ss << "Unable to parse JSON. Missing key '" << key << "'.";
		throw JSONParseException(ss.str());
	}

	if (!json[key].is_boolean()) {
		std::stringstream ss;
		ss << "Unable to parse JSON. Value for key '" << key << "' was not a valid boolean.";
		throw JSONParseException(ss.str());
	}

	return json[key].template get<bool>();
}

bool Utility::JSON::ReadBoolOptional(nlohmann::json json, std::string key, bool defaultValue)
{
	if (!json.contains(key))
	{
		return defaultValue;
	}

	if (!json[key].is_boolean()) {
		std::stringstream ss;
		ss << "Unable to parse JSON. Value for key '" << key << "' was not a valid boolean.";
		throw JSONParseException(ss.str());
	}

	return json[key].template get<bool>();
}

nlohmann::json Utility::JSON::ReadChildObject(nlohmann::json json, std::string key)
{
	if (!json.contains(key)) // Key does not exist
	{
		std::stringstream ss;
		ss << "Unable to parse JSON. Missing object key '" << key << "'.";
		throw JSONParseException(ss.str());
	}

	if (!json[key].is_object())
	{
		std::stringstream ss;
		ss << "Unable to parse JSON. Value at key '" << key << "' is not an object.";
		throw JSONParseException(ss.str());
	}

	return json[key];
}