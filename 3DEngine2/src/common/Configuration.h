#pragma once

#include <string>
#include <memory>
#include <unordered_map>

namespace tde 
{
	class Configuration
	{
	public:

													~Configuration();

		static void									Initialize();
		static std::shared_ptr<Configuration>		GetInstance();
		
		void										LoadCfgFile(
														const std::string& aConfigFilePath, 
														const bool aClear = false);
		std::string									GetString(
														const std::string& aKey) const;
		int32_t										GetInt(
														const std::string& aKey) const;
		float										GetFloat(
														const std::string& aKey) const;
		bool										GetBool(
														const std::string& aKey) const;
		std::string									GetStringOrDefault(
														const std::string& aKey, 
														const std::string& aDefault = "") const;
		int32_t										GetIntOrDefault(
														const std::string& aKey, 
														const int32_t aDefault = 0) const;
		float										GetFloatOrDefault(
														const std::string& aKey, 
														const float aDefault = 0.0f) const;
		bool										GetBoolOrDefault(
														const std::string& aKey, 
														const bool aDefault = false) const;
		
	private:

		static std::shared_ptr<Configuration>		mInstance;
		
		std::unordered_map<std::string, std::string> mStorage;
		
													Configuration();
	};
}