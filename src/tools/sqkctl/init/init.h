#pragma once

#include "../squick_files.h"
#include <fstream>
#include <iostream>
#include <third_party/common/md5.hpp>
#include <third_party/nlohmann/json.hpp>
using namespace nlohmann;
using namespace std;
namespace sqkctl::init {
class Init {
  public:
    Init() { std::cout << "Init\n"; }

    ~Init() {}

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

        const char *squick_install_path = getenv("squick_path");
        if (squick_install_path == NULL) {
            cout << " Squick path is not set\n";
            return 2;
        }

        const char *squick_version = getenv("squick_version");
        if (squick_version == NULL) {
            squick_version = "1.0.0";
        }

        cout << " Squick install path: " << squick_install_path << std::endl;

#if PLATFORM == PLATFORM_WIN
        system("mkdir squick");
        string copy_cmd = "xcopy /s /e /y /h " + std::string(squick_install_path) + " squick";
#else
        string copy_cmd = "cp -r " + std::string(squick_install_path) + " ./squick";
#endif
        system(copy_cmd.c_str());

        j_all["version"] = squick_version;
        string squick_path = "squick";
        auto files = SquickFiles::GetFiles(squick_path);
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
        std::string gitignore_file_content =
            R"(/backup
sqkctl
sqkctl.exe
)";
        gitignore_file.write(gitignore_file_content.c_str(), gitignore_file_content.size());
        gitignore_file.close();

        // submodule
        fstream submodule_file;
        submodule_file.open(".gitmodules", ios::out);
        std::string submodule_file_content =
            R"([submodule "squick"]
	path = squick
	url = https://github.com/pwnsky/squick.git
)";
        submodule_file.write(submodule_file_content.c_str(), submodule_file_content.size());
        submodule_file.close();

        // readme
        fstream readme_file;
        readme_file.open("README.md", ios::out);
        std::string readme_file_content =
            R"(
# Squick Project

## 1 拉取项目

```
git clone https://xxx.com/xxx/xxx.git
cd server
git submodule init
git submodule update
```


## 2 安装编译squick

请查看 https://github.com/pwnsky/squick/tree/main#%E5%AE%89%E8%A3%85

将所有文件编译完毕后，拷贝sqkctl文件到项目根目录下

```
cp {project_path}/squick/tools/bin/sqkctl {project_path}
```



## 3 恢复工程文件

在安装好squick之后，采用工具sqkctl恢复全部文件，执行命令如下：

```
cd {project_path}
sqkctl patch
```

)";

        readme_file.write(readme_file_content.c_str(), readme_file_content.size());
        readme_file.close();
        return 0;
    }

  private:
};
} // namespace sqkctl::init
