#pragma once
#include "../squick_files.h"
#include <fstream>
#include <third_party/common/md5.hpp>
#include <third_party/nlohmann/json.hpp>
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
        } catch (json::exception e) {
            cout << e.what() << endl;
            return 2;
        }

        map<string, string> baseFilesMap;
        json bfiles = baseFiles["files"];
        for (auto jf : bfiles) {
            std::string md5 = jf["md5"];
            std::string path = jf["path"];
            // std::cout << "path: " << path << " md5: " <<  md5 << std::endl;
            baseFilesMap[path] = md5;
        }

        // get current squick dir files
        map<string, string> currentSquickFilesMap;
        string squick_path = "squick";
        auto cfiles = SquickFiles::GetFiles(squick_path);
        for (auto fn : cfiles) {
            currentSquickFilesMap[fn] = md5file(fn.c_str());
        }

        // Get files
        // 计算
        map<string, string> currentFilesMap;
        auto files = Files::GetFileListInFolder("files");
        for (auto fn : files) {
            string filepath = fn;
            Files::StringReplace(filepath, "\\", "/");
            Files::StringReplace(filepath, "files/", "");
            currentFilesMap[filepath] = md5file(fn.c_str());
        }

        // 基于base
        map<string, string> addedFilesMap;   // 增量文件
        map<string, string> reducedFilesMap; // 减量文件
        map<string, string> changedFilesMap; // 改变的文件

        // 基于拥有files之后
        // map<string, string> unaddedFilesMap;   // 未增加的文件
        map<string, string> unpatchedFilesMap; // 未打patch的文件

        for (auto &f : baseFilesMap) {
            auto iter = currentSquickFilesMap.find(f.first);
            if (iter == currentSquickFilesMap.end()) {
                // not found
                reducedFilesMap[f.first] = f.second;
                continue;
            }

            if (iter->second != f.second) {
                changedFilesMap[iter->first] = iter->second;
            }
        }

        // 计算增量文件
        for (auto &f : currentSquickFilesMap) {
            auto iter = baseFilesMap.find(f.first);
            if (iter == baseFilesMap.end()) {
                // not found
                addedFilesMap[f.first] = f.second;
            }
        }

        // 计算未打patch的文件,在files里找squick的文件
        for (auto &f : currentFilesMap) {
            auto iter = currentSquickFilesMap.find(f.first);
            if (iter == currentSquickFilesMap.end()) {
                unpatchedFilesMap[f.first] = f.second;
                continue;
            }
            if (iter->second != f.second) {
                unpatchedFilesMap[iter->first] = iter->second;
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

        cout << "\n\n Unpatched files in files directory: " << unpatchedFilesMap.size() << endl;
        for (auto f : unpatchedFilesMap) {
            cout << "   " << f.first << endl;
        }
    }
};
} // namespace sqkctl::diff