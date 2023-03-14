#pragma once

#include "i_generator.h"
namespace sqkctl {
class PBGenerator : public IGenerator
{
public:
	PBGenerator(const std::string &excelPath, const std::string &outPath)
	{
		SetPath(excelPath, outPath);
	}

	virtual bool Generate(const std::map<std::string, ClassData *> &classData) override
	{
		return false;
	}
};
}
