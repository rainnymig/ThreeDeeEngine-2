#include "pch.h"
#include "Configuration.h"

#include <iostream>
#include <fstream>
#include <sstream>

namespace tde
{
	std::shared_ptr<Configuration> Configuration::mInstance;

	void Configuration::Initialize()
	{
		if (mInstance)
		{
			return;
		}
		mInstance = std::shared_ptr<Configuration>(new Configuration());
	}

	void Configuration::LoadCfgFile(const std::string& aConfigFilePath, const bool aClear)
	{
		// load config file
		std::ifstream cfgFile(aConfigFilePath.c_str());
		if (!cfgFile.is_open())
		{
			throw std::runtime_error("failed to open configuration file");
		}
		std::string line;
		while (std::getline(cfgFile, line))
		{
			std::istringstream iss(line);
			std::string key;
			std::string val;
			if (!(iss >> key))			// empty line
			{
				continue;
			}
			if (key[0] == '#')			// comment line
			{
				continue;
			}
			if (!(iss >> val))			// empty value
			{
				continue;
			}
			mStorage[key] = val;
		}
	}

	std::string Configuration::GetString(const std::string& aKey) const
	{
		auto iter = mStorage.find(aKey);
		if (iter == mStorage.end())
		{
			throw std::runtime_error("configuration key not found");
		}
		return iter->second;
	}

	int32_t Configuration::GetInt(const std::string& aKey) const
	{
		auto iter = mStorage.find(aKey);
		if (iter == mStorage.end())
		{
			throw std::runtime_error("configuration key not found");
		}
		return std::stoi(iter->second); // note: std::stoi does not perform full error check, but it's ok here
										// https://stackoverflow.com/questions/11598990/is-stdstoi-actually-safe-to-use
	}

	float Configuration::GetFloat(const std::string& aKey) const
	{
		auto iter = mStorage.find(aKey);
		if (iter == mStorage.end())
		{
			throw std::runtime_error("configuration key not found");
		}
		return std::stof(iter->second); // note: similar to std::stoi
	}

	bool Configuration::GetBool(const std::string& aKey) const
	{
		auto iter = mStorage.find(aKey);
		if (iter == mStorage.end())
		{
			throw std::runtime_error("configuration key not found");
		}
		if (iter->second == "true")
		{
			return true;
		}
		else if (iter->second == "false")
		{
			return false;
		}
		else
		{
			throw std::invalid_argument("configuation value is not boolean");
		}
	}

	std::string Configuration::GetStringOrDefault(const std::string& aKey, const std::string& aDefault) const
	{
		auto iter = mStorage.find(aKey);
		if (iter == mStorage.end())
		{
			return aDefault;
		}
		return iter->second;
	}

	int32_t Configuration::GetIntOrDefault(const std::string& aKey, const int32_t aDefault) const
	{
		auto iter = mStorage.find(aKey);
		if (iter == mStorage.end())
		{
			return aDefault;
		}
		return std::stoi(iter->second); // note: std::stoi does not perform full error check, but it's ok here
										// https://stackoverflow.com/questions/11598990/is-stdstoi-actually-safe-to-use
	}

	float Configuration::GetFloatOrDefault(const std::string& aKey, const float aDefault) const
	{
		auto iter = mStorage.find(aKey);
		if (iter == mStorage.end())
		{
			return aDefault;
		}
		return std::stof(iter->second); // note: similar to std::stoi
	}

	bool Configuration::GetBoolOrDefault(const std::string& aKey, const bool aDefault) const
	{
		auto iter = mStorage.find(aKey);
		if (iter == mStorage.end())
		{
			return aDefault;
		}
		if (iter->second == "true")
		{
			return true;
		}
		else if (iter->second == "false")
		{
			return false;
		}
		else
		{
			return aDefault;
		}
	}

	std::shared_ptr<Configuration> Configuration::GetInstance()
	{
		if (mInstance == nullptr)
		{
			throw std::runtime_error("configuration uninitialized");
		}
		return mInstance;
	}

	Configuration::Configuration()
	{
	}

	Configuration::~Configuration()
	{
		mStorage.clear();
	}

}

