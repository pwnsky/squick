#include "excel/config_generator.h"
#include <iostream>
#include <chrono>
#include <string>

#include <squick/core/platform.h>
#include "init/init.h"
#include "diff/diff.h"
#include "add/add.h"

using namespace sqkctl;

void help() {
	std::cout << "Usague: sqkctl [cmd] [arg1] [arg2] ...\n"
		<< "     excel:  \n"
		<< "     init \n"
		<< "     diff \n"
		<< "     add \n"
		<< "     patch \n"
		<< "     pull \n"
		<< "     update \n"
		<< "     version \n"
		<< "  Examples: \n"
		<< "  sqkctl excel ../resource/excel ../config \n"
		;
}

int main(int argc, const char *argv[])
{
	if(argc < 2) {
		help();
		return 1;
	}

	const std::string cmd = argv[1];

	if (cmd == "excel") {
		if (argc < 4) {
			help();
			return 1;
		}
		std::string excelPath = argv[2];
		std::string outPath = argv[3];
		std::cout << "Excel path: " << excelPath << std::endl;
		std::cout << "Out path: " << outPath << std::endl;
		auto t1 = SquickGetTimeMS();

		ConfigGenerator fp(excelPath, outPath);
		fp.SetUTF8(false);//set it true to convert UTF8 to GBK which is to show Chinese words in Squick
		fp.LoadDataFromExcel();
		fp.PrintData();
		fp.GenerateData();
		auto t2 = SquickGetTimeMS();
		std::cout << "Timespan: " << (t2 - t1) << " ms" << std::endl;
	}
	else if (cmd == "init") {
		init::Init i;
		i.Exec();
	}
	else if (cmd == "diff") {
		diff::Diff d;
		d.Exec();
	}
	else if (cmd == "update") {

	}
	else if (cmd == "version") {

	}
	else if (cmd == "add") {
		add::Add a;
		a.Exec();
	}
	else if (cmd == "patch") {

	}
	else if (cmd == "pull") {

	}
	else {
		help();
	}

	

	return 0;
}
