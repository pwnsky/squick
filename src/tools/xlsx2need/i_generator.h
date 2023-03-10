#pragma once


#include "config_generator_data.h"
#include "config_generator_help.h"
namespace squick::tools::file_process {
class IGenerator
{
public:

	void SetPath(const std::string &excelPath, const std::string &outPath) {
		
		this->outPath = outPath;
		strXMLStructPath = outPath + "/struct/";
		strXMLIniPath = outPath + "/ini/";
		strExcelIniPath = excelPath;
		
	}

	virtual bool Generate(const std::map<std::string, ClassData*>& classData) = 0;

	std::string strExcelIniPath;
	std::string strXMLStructPath;
	std::string strXMLIniPath;
	std::string outPath;

};

}
