#include "config_generator.h"
#include <iostream>
#include <chrono>
#include <string>

#include <squick/core/platform.h>

using namespace squick::tools::file_process;

int main(int argc, const char *argv[])
{
	if(argc < 3) {
		std::cout << "Usague: ./xlsx2need [excel path] [out path]\n";
		std::cout << "Example: ./xlsx2need ../resource/excel ../config\n";
		return 1;
	}
	std::string excelPath = argv[1];
	std::string outPath = argv[2];
	std::cout << "Excel path: " << excelPath << std::endl;
	std::cout << "Out path: " << outPath << std::endl;
	auto t1 = SquickGetTimeMS();

	ConfigGenerator fp(excelPath, outPath);
	fp.SetUTF8(false);//set it true to convert UTF8 to GBK which is to show Chinese words in Squick
	fp.LoadDataFromExcel();

	fp.PrintData();

	fp.GenerateData();

	auto t2 = SquickGetTimeMS();
	std::cout << "Timespan: "  << (t2 - t1) << " ms" << std::endl;

	return 0;
}
