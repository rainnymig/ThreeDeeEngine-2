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
			throw OpenConfigFileFailedException();
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

	std::string Configuration::GetValue(const std::string& aKey) const
	{
		auto iter = mStorage.find(aKey);
		if (iter == mStorage.end())
		{
			throw KeyNotFoundException();
		}
		return iter->second;
	}

	std::shared_ptr<Configuration> Configuration::GetInstance()
	{
		if (mInstance == nullptr)
		{
			throw UninitializedException();
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

