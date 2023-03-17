#pragma once
#include <vector>
#include <string>
#include "files.h"

namespace sqkctl {
	class SquickFiles {
	public:
		// 过滤后的文件
		static std::vector<std::string> GetFiles(const std::string& squick_path) {
			std::vector<std::string> result;
			auto files = Files::GetFileListInFolder(squick_path, 5);
			// 黑名单过滤
			for (auto& file : files) {
				Files::StringReplace(file, "\\", "/");
				Files::StringReplace(file, "//", "/");

				if (Files::IsStartWith(file, squick_path + "/third_party")) {
					continue;
				}
				else if (Files::IsStartWith(file, squick_path + "/.git")) {
					continue;
				}
				else if (Files::IsStartWith(file, squick_path + "/bin")) {
					continue;
				}
				else if (Files::IsStartWith(file, squick_path + "/cache")) {
					continue;
				}
				else if (Files::IsStartWith(file, squick_path + "/client")) {
					continue;
				}
				else if (Files::IsStartWith(file, squick_path + "/deploy")) {
					continue;
				}
				else if (Files::IsStartWith(file, squick_path + "/src/www/admin/node_modules")) {
					continue;
				}
				else if (Files::IsStartWith(file, squick_path + "/src/www/admin/dist")) {
					continue;
				}
				else if (Files::IsStartWith(file, squick_path + "/src/www/server/build")) {
					continue;
				}
				else if (Files::IsStartWith(file, squick_path + "/.vscode")) {
					continue;
				}
				else if (Files::IsStartWith(file, squick_path + "/tools/bin")) {
					continue;
				}
				else if (Files::IsStartWith(file, squick_path + "/config/ini")) {
					continue;
				}
				else if (Files::IsStartWith(file, squick_path + "/config/struct")) {
					continue;
				}
				else if (Files::IsStartWith(file, squick_path + "/src/squick/struct/")) {
					int pos = file.find(".");
					if (pos > 0) {
						string sub = file.substr(pos, file.length() - pos); // PB文件
						if (sub.find("pb") > 0) {
							continue;
						}
					}
				}
				result.push_back(file);
			}
			return result;
		}

		// 
		static void CleanFiles() {
			// files目录下的所有文件
#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN
			system("del /f /q /s  files\\*");
			system("del /f /q /s  changed.json");
#else
			system("rm -rf files/*");
			system("rm -rf changed.json");
#endif
		}

		// 将改动的文件从squick复制到files
		static void CopySquickToFiles(const std::string& file) {
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

		// 将改动的文件从files复制到squick
		static void CopyFilesToSquick(const std::string& file) {

			string filepath = file;
			Files::StringReplace(filepath, "\\", "/");
			Files::StringReplace(filepath, "files/squick/", "");

			string  cmd = "";
			string targetPath = "squick/" + Files::GetFilePathByPath(filepath);
			string sourcePath = "files/squick/" + filepath;

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

		static void BackupFiles() {
#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN
			system("mkdir backup");
			system("xcopy /s /e /y files backup");
#else
			system("mkdir -p backup");
			system("cp -r files/squick backup");
#endif
		}



	};
}