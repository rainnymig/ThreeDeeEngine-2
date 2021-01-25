#pragma once

#include <string>
#include <memory>
#include <unordered_map>

namespace tde 
{
	class Configuration
	{
	public:
		class UninitializedException: public std::exception
		{};
		class KeyNotFoundException : public std::exception
		{};
		class OpenConfigFileFailedException: public std::exception
		{};

		~Configuration();

		static void Initialize();
		static std::shared_ptr<Configuration> GetInstance();
		
		void LoadCfgFile(const std::string& aConfigFilePath, const bool aClear = false);
		std::string GetValue(const std::string& aKey) const;

	private:

		static std::shared_ptr<Configuration> mInstance;
		
		std::unordered_map<std::string, std::string> mStorage;
		
		Configuration();
	};
}