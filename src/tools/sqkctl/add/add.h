#pragma once
#include "../squick_files.h"
#include <fstream>
#include <third_party/common/md5.hpp>
#include <third_party/nlohmann/json.hpp>
using namespace nlohmann;
using namespace std;
namespace sqkctl::add {

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
        map<string, string> currentFilesMap;
        string squick_path = "squick";
        auto cfiles = SquickFiles::GetFiles(squick_path);
        for (auto fn : cfiles) {
            currentFilesMap[fn] = md5file(fn.c_str());
        }

        // Calc
        map<string, string> addedFilesMap;   // 增量文件
        map<string, string> reducedFilesMap; // 减量文件
        map<string, string> changedFilesMap; // 改变的文件

        for (auto &f : baseFilesMap) {
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
        for (auto &f : currentFilesMap) {
            auto iter = baseFilesMap.find(f.first);
            if (iter == baseFilesMap.end()) {
                // not found
                addedFilesMap[f.first] = f.second;
            }
        }

        // ----------- above code from diff.h
        // 在执行Clean的时候，先将files下的所有文件做一个备份，防止在更新squick所有文件的时候，将files目录下的所有文件给删除了。
        SquickFiles::BackupFiles();

        // 清除
        SquickFiles::CleanFiles();

        json record;
        json recordAdded;
        cout << " Added files: " << addedFilesMap.size() << endl;
        for (auto f : addedFilesMap) {
            json jf;
            jf["path"] = f.first;
            jf["md5"] = f.second;
            recordAdded.push_back(jf);
            cout << "   " << f.first << endl;
            SquickFiles::CopySquickToFiles(f.first);
        }

        // record["added"] = recordAdded;

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

            SquickFiles::CopySquickToFiles(f.first);
        }
        // record["changed"] = recordChanged;

        fstream base;
        base.open("changed.json", ios::out);
        std::string outContent = record.dump();
        base.write(outContent.c_str(), outContent.size());
        base.close();

        cout << " Add files to files directory finished!\n";
        return 0;
    }

  private:
};
} // namespace sqkctl::add
