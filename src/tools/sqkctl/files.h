#pragma once

#include "third_party/common/lexical_cast.hpp"
#include <squick/core/platform.h>

#if PLATFORM != PLATFORM_WIN
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#else if PLATFORM == PLATFORM_WIN
#include <corecrt_io.h>
#include <io.h>
#endif

namespace sqkctl {
class Files {
  public:
    static void StringReplace(std::string &strBig, const std::string &strsrc, const std::string &strdst) {
        std::string::size_type pos = 0;
        std::string::size_type srclen = strsrc.size();
        std::string::size_type dstlen = strdst.size();

        while ((pos = strBig.find(strsrc, pos)) != std::string::npos) {
            strBig.replace(pos, srclen, strdst);
            pos += dstlen;
        }
    }

    static bool IsStartWith(const std::string &str, const std::string &substr) {
        if (substr.size() > str.size()) {
            return false;
        }
        size_t size = substr.size();
        for (int i = 0; i < size; ++i) {
            if (str[i] != substr[i]) {
                return false;
            }
        }

        return true;
    }

    static bool IsInclude(const std::string &str, const std::string &substr) { return false; }

    static std::string GetFileNameByPath(const std::string &filePath) {
        std::string fileName = filePath.substr(filePath.find_last_of('/') + 1, filePath.find_last_of('.') - filePath.find_last_of('/') - 1);
        return fileName;
    }

    static std::string GetFilePathByPath(const std::string &filePath) {
        int pos = filePath.find_last_of('/');
        if (pos < 0) {
            return ".";
        }
        std::string path = filePath.substr(0, pos);
        return path;
    }

    static std::string GetFileNameExtByPath(const std::string &filePath) {
        std::string ext = filePath.substr(filePath.find_last_of('.'), filePath.length() - filePath.find_last_of('.'));
        return ext;
    }

    static std::vector<std::string> GetFileListInFolder(std::string folderPath, int depth = 5) {
        depth--;

        std::vector<std::string> result;
#if PLATFORM == PLATFORM_WIN
        _finddata_t FileInfo;
        std::string strfind = folderPath + "\\*";
        long long Handle = _findfirst(strfind.c_str(), &FileInfo);
        if (Handle == -1L) {
            // std::cerr << "can not match the folder path:" << folderPath << std::endl;
            return result;
        }
        do {

            if (FileInfo.attrib & _A_SUBDIR) {

                if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0)) {
                    std::string newPath = folderPath + "\\" + FileInfo.name;
                    if (depth >= 0) {
                        std::vector<std::string> childResult = GetFileListInFolder(newPath, depth);
                        result.insert(result.end(), childResult.begin(), childResult.end());
                    }
                }
            } else {

                std::string filename = (folderPath + "\\" + FileInfo.name);
                result.push_back(filename);
            }
        } while (_findnext(Handle, &FileInfo) == 0);

        _findclose(Handle);
#else
        DIR *pDir;
        struct dirent *ent;
        char childpath[512];
        char absolutepath[512];
        pDir = opendir(folderPath.c_str());
        if (pDir) {
            memset(childpath, 0, sizeof(childpath));
            while ((ent = readdir(pDir)) != NULL) {
                if (ent->d_type & DT_DIR) {
                    if ((strcmp(ent->d_name, ".") != 0) && (strcmp(ent->d_name, "..") != 0)) {
                        sprintf(absolutepath, "%s/%s", folderPath.c_str(), ent->d_name);
                        if (depth >= 0) {
                            std::vector<std::string> childResult = GetFileListInFolder(absolutepath, depth);
                            result.insert(result.end(), childResult.begin(), childResult.end());
                        }
                    }
                } else {
                    sprintf(absolutepath, "%s/%s", folderPath.c_str(), ent->d_name);
                    result.push_back(absolutepath);
                }
            }

            sort(result.begin(), result.end());
        }
#endif
        return result;
    }

    static std::vector<std::string> GetFolderListInFolder(std::string folderPath) {
        std::vector<std::string> result;
#if PLATFORM == PLATFORM_WIN
        _finddata_t FileInfo;
        std::string strfind = folderPath + "\\*";
        long long Handle = _findfirst(strfind.c_str(), &FileInfo);
        if (Handle == -1L) {
            std::cerr << "can not match the folder path" << std::endl;
            return result;
        }
        do {

            if (FileInfo.attrib & _A_SUBDIR) {
                if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0)) {
                    std::string newPath = folderPath + "\\" + FileInfo.name;
                    result.push_back(FileInfo.name);
                }
            }

        } while (_findnext(Handle, &FileInfo) == 0);

        _findclose(Handle);
#else
        DIR *pDir;
        struct dirent *ent;
        char childpath[512];
        char absolutepath[512];
        pDir = opendir(folderPath.c_str());
        memset(childpath, 0, sizeof(childpath));
        while ((ent = readdir(pDir)) != NULL) {
            if (ent->d_type & DT_DIR) {
                if ((strcmp(ent->d_name, ".") != 0) && (strcmp(ent->d_name, "..") != 0)) {
                    sprintf(absolutepath, "%s/%s", folderPath.c_str(), ent->d_name);

                    result.push_back(ent->d_name);
                }
            }
        }

        sort(result.begin(), result.end());
#endif
        return result;
    }
};

} // namespace sqkctl