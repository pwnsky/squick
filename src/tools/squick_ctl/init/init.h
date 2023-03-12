#pragma once

#include <iostream>
#include "../files.h"
#include <third_party/common/md5.hpp>
#include <third_party/nlohmann/json.hpp>
#include<fstream> 
using namespace nlohmann;
using namespace std;
namespace squick_ctl::init {
	class Init {
	public:
		Init() {
			std::cout << "Init\n";
		}

		~Init() {

		}


		bool IsEmptyFolder() {
			auto allfiles = Files::GetFileListInFolder(".", 1);
			return allfiles.size() == 0;
		}

		// White
		int Exec() {
			// 校验当前文件夹下是否为空

			if (!IsEmptyFolder()) {
				cout << "Current folder is not empty\n";
				return 1;
			}

			cout << "Start to copy squick files into here\n";
			
			json j_all;
			json j_files;
			

			const char* squick_install_path = getenv("squick_path");
			if (squick_install_path == NULL) {
				cout << " Squick path is not set\n";
				return 2;
			}

			const char* squick_version = getenv("squick_version");
			if (squick_version == NULL) {
				squick_version = "1.0.0";
			}


			cout << " Squick install path: " << squick_install_path << std::endl;
			system("mkdir squick");
#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN
			string copy_cmd = "xcopy /s /e /y " + std::string(squick_install_path) + " squick";
#else
			string copy_cmd = "cp -r " + std::string(squick_install_path) + "/* ./squick";
#endif
			system(copy_cmd.c_str());



			j_all["version"] = squick_version;
			string squick_path = "squick";
			auto files = Files::GetUnblackedFiles(squick_path);
			for (auto file : files) {
				json jf;
				jf["path"] = file;
				jf["md5"] = md5file(file.c_str());
				j_files.push_back(jf);
			}

			j_all["files"] = j_files;
			
			fstream base;
			base.open("base.json", ios::out);
			std::string outContent = j_all.dump();
			base.write(outContent.c_str(), outContent.size());
			base.close();


			fstream gitignore_file;
			gitignore_file.open(".gitignore", ios::out);
			std::string gitignore_file_content;
			gitignore_file_content = "/squick\n"
				"/backup\n"
				;
			gitignore_file.write(gitignore_file_content.c_str(), gitignore_file_content.size());
			gitignore_file.close();

			
			system("mkdir files");
			fstream files_keep;
			files_keep.open("files/.gitkeep", ios::out);
			files_keep.close();

			
			// readme
			fstream readme_file;
			readme_file.open("README.md", ios::out);
			std::string readme_file_content;
			readme_file_content = "# Squick project \n";
			readme_file_content += " managed by squick_ctl\n";
			readme_file_content += " Github: https://github.com/pwnsky/squick";

			readme_file.write(readme_file_content.c_str(), readme_file_content.size());
			readme_file.close();

		}

	private:

	};
}