//
// Created by James on 7/02/21.
//

#ifndef NFPBGENERATOR_H
#define NFPBGENERATOR_H
#include "../i_generator.h"
namespace squick::tools::file_process {
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

#endif
