#include "settings.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

std::map<std::string , std::map<std::string , std::string>> EasyIni::data_;

EasyIni::EasyIni()
{
	// Load settings.ini from the application's working directory.
	std::ifstream file("./settings.ini");
	if (!file) {
		throw std::runtime_error("Could not open file");
	}
	std::string line;
	std::string currentSection;
	while (std::getline(file , line)) {
		line.erase(0 , line.find_first_not_of(" \t"));
		line.erase(line.find_last_not_of(" \t") + 1);

		if (line.empty() || line[0] == '%') {
			continue;
		}

		if (line[0] == '[' && line.back() == ']') {
			currentSection = line.substr(1 , line.size() - 2);
			continue;
		}

		std::string::size_type pos = line.find('=');
		if (pos != std::string::npos) {
			std::string key = line.substr(0 , pos);
			std::string value = line.substr(pos + 1);

			key.erase(0 , key.find_first_not_of(" \t"));
			key.erase(key.find_last_not_of(" \t") + 1);
			value.erase(0 , value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t") + 1);

			data_[currentSection][key] = value;
		}
	}
}

EasyIni::EasyIni(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file) {
		throw std::runtime_error("Could not open file");
	}
	std::string line;
	std::string currentSection;
	while (std::getline(file , line)) {
		line.erase(0 , line.find_first_not_of(" \t"));
		line.erase(line.find_last_not_of(" \t") + 1);

		if (line.empty() || line[0] == '%') {
			continue;
		}

		if (line[0] == '[' && line.back() == ']') {
			currentSection = line.substr(1 , line.size() - 2);
			continue;
		}

		std::string::size_type pos = line.find('=');
		if (pos != std::string::npos) {
			std::string key = line.substr(0 , pos);
			std::string value = line.substr(pos + 1);

			key.erase(0 , key.find_first_not_of(" \t"));
			key.erase(key.find_last_not_of(" \t") + 1);
			value.erase(0 , value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t") + 1);

			data_[currentSection][key] = value;
		}
	}
}

EasyIni::~EasyIni()
{

}

EasyIni* EasyIni::GetPtr()
{
	static EasyIni* easyini = new EasyIni();
	return easyini;
}

std::string EasyIni::get(const std::string& section , const std::string& key , const std::string& defaultValue)
{
	auto sectionIt = data_.find(section);
	if (sectionIt != data_.end()) {
		auto keyIt = sectionIt->second.find(key);
		if (keyIt != sectionIt->second.end()) {
			return keyIt->second;
		}
	}
	return defaultValue;
}

void EasyIni::fresh() {
	std::ifstream file("./settings.ini");
	if (!file) {
		throw std::runtime_error("Could not open file");
	}
	std::string line;
	std::string currentSection;
	while (std::getline(file, line)) {
		line.erase(0, line.find_first_not_of(" \t"));
		line.erase(line.find_last_not_of(" \t") + 1);

		if (line.empty() || line[0] == '%') {
			continue;
		}

		if (line[0] == '[' && line.back() == ']') {
			currentSection = line.substr(1, line.size() - 2);
			continue;
		}

		std::string::size_type pos = line.find('=');
		if (pos != std::string::npos) {
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);

			key.erase(0, key.find_first_not_of(" \t"));
			key.erase(key.find_last_not_of(" \t") + 1);
			value.erase(0, value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t") + 1);

			data_[currentSection][key] = value;
		}
	}
}
