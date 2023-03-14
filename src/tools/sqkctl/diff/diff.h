#pragma once
#include "../files.h"
#include <third_party/common/md5.hpp>
#include <third_party/nlohmann/json.hpp>
#include <fstream>
using namespace nlohmann;
using namespace std;
namespace sqkctl::diff {

	class Diff {
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
			catch(json::exception e) {
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

			// 显示
			cout << " Added files: " << addedFilesMap.size() << endl;
			for (auto f : addedFilesMap) {
				cout << "   " << f.first << endl;
			}

			cout << "\n\n Reduced files: " << reducedFilesMap.size() << endl;
			for (auto f : reducedFilesMap) {
				cout << "   " << f.first << endl;
			}

			cout << "\n\n Changed files: " << changedFilesMap.size() << endl;
			for (auto f : changedFilesMap) {
				cout << "   " << f.first << endl;
			}
			
		}
	};
}