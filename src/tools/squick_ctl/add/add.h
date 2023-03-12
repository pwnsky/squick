#pragma once
#include "../files.h"
#include <third_party/common/md5.hpp>
#include <third_party/nlohmann/json.hpp>
#include <fstream>
using namespace nlohmann;
using namespace std;
namespace squick_ctl::add {

	class Add {
	public:

		
		int Exec() {

			ifstream file;
			file.open("base.json", ios::in);
			if (!file.is_open()) {
				cout << " Open base.json file failed!\n";
				return 1;
			}

			json baseFiles;
			try {
				baseFiles = json::parse(file);
			}
			catch (json::exception e) {
				cout << e.what() << endl;
				return 2;
			}

			map<string, string> baseFilesMap;
			json bfiles = baseFiles["files"];
			for (auto jf : bfiles) {
				std::string md5 = jf["md5"];
				std::string path = jf["path"];
				//std::cout << "path: " << path << " md5: " <<  md5 << std::endl;
				baseFilesMap[path] = md5;
			}


			// get current squick dir files
			map<string, string> currentFilesMap;
			string squick_path = "squick";
			auto cfiles = Files::GetUnblackedFiles(squick_path);
			for (auto fn : cfiles) {
				currentFilesMap[fn] = md5file(fn.c_str());
			}


			// Calc
			map<string, string> addedFilesMap; // 增量文件
			map<string, string> reducedFilesMap; // 减量文件
			map<string, string> changedFilesMap; // 改变的文件

			for (auto& f : baseFilesMap) {
				auto iter = currentFilesMap.find(f.first);
				if (iter == currentFilesMap.end()) {
					// not found
					reducedFilesMap[f.first] = f.second;
					continue;
				}

				if (iter->second != f.second) {
					changedFilesMap[iter->first] = iter->second;
				}
			}

			// 计算增量文件
			for (auto& f : currentFilesMap) {
				auto iter = baseFilesMap.find(f.first);
				if (iter == baseFilesMap.end()) {
					// not found
					addedFilesMap[f.first] = f.second;
				}
			}

			// ----------- above code from diff.h

			// 清除
			Clean();

			json record;
			json recordAdded;
			cout << " Added files: " << addedFilesMap.size() << endl;
			for (auto f : addedFilesMap) {
				json jf;
				jf["path"] = f.first;
				jf["md5"] = f.second;
				recordAdded.push_back(jf);
				cout << "   " << f.first << endl;
				Copy(f.first);
			}

			//record["added"] = recordAdded;

			json recordReduced;
			cout << "\n\n Reduced files: " << reducedFilesMap.size() << endl;
			for (auto f : reducedFilesMap) {
				json jf;
				jf["path"] = f.first;
				jf["md5"] = f.second;
				recordReduced.push_back(jf);
				cout << "   " << f.first << endl;

			}
			record["reduced"] = recordReduced;


			cout << "\n\n Changed files: " << changedFilesMap.size() << endl;
			json recordChanged;
			for (auto f : changedFilesMap) {
				json jf;
				jf["path"] = f.first;
				jf["md5"] = f.second;
				recordChanged.push_back(jf);
				cout << "   " << f.first << endl;

				Copy(f.first);
			}
			//record["changed"] = recordChanged;
			
			fstream base;
			base.open("changed.json", ios::out);
			std::string outContent = record.dump();
			base.write(outContent.c_str(), outContent.size());
			base.close();
			
			cout << " Add files to files directory finished!\n";
		}
	private:
		void Clean() {
			// files目录下的所有文件
#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN
			system("del /f /q /s  files\\*");
			system("del /f /q /s  changed.json");
#else
			system("rm -rf files/*");
			system("rm -rf changed.json");
#endif
		}

		void Copy(const std::string &file) {
			// 将文件复制到 files 下
			string  cmd = "";
			string targetPath = "files/" + Files::GetFilePathByPath(file);
			string sourcePath = file;

			sourcePath = "\"" + sourcePath + "\"";
			targetPath = "\"" + targetPath + "\"";
#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN
			Files::StringReplace(targetPath, "/", "\\");
			Files::StringReplace(sourcePath, "/", "\\");
			cmd = "mkdir " + targetPath;
			system(cmd.c_str());
			cmd = "copy " + sourcePath + " " + targetPath;
			//cout << " cmd: " << cmd << "\n";
			system(cmd.c_str());
#else
			cmd = "mkdir -p " + targetPath;
			system(cmd.c_str());
			cmd = "cp " + sourcePath + " " + targetPath;
			system(cmd.c_str());
#endif
		}
	};
}