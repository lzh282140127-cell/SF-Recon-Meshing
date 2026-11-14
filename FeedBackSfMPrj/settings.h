#include <map>
#include <string>

// Thread-safe singleton for reading settings.ini.
class EasyIni {
public:
	static EasyIni* GetPtr();
	std::string get(const std::string& section , const std::string& key , const std::string& defaultValue = "");
	void fresh();

private:
	static std::map<std::string , std::map<std::string , std::string>> data_;
	explicit EasyIni();
	explicit EasyIni(const std::string& filename);
	~EasyIni();
	EasyIni(const EasyIni& single) = delete;
	const EasyIni& operator=(const EasyIni& single) = delete;
};

/* Example
% Comment
[SfM]
% Use global bundle adjustment
useGlobalBA = true

[Default]
datasets = C: / path_to_datasets

[Python]
PythonHome = D:/ENV/Python/
PythonWorkspace = D:/CS/
*/
