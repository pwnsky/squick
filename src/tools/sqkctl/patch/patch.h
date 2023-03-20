#pragma once

#include "../squick_files.h"
#include <fstream>
#include <iostream>
#include <third_party/common/md5.hpp>
#include <third_party/nlohmann/json.hpp>
using namespace nlohmann;
using namespace std;
namespace sqkctl::patch {
class Patch {
  public:
    Patch() {}

    ~Patch() {}

    int Exec() {
        // Patch时直接复制
        auto files = Files::GetFileListInFolder("files");
        for (auto file : files) {

            SquickFiles::CopyFilesToSquick(file);
        }

        return 0;
    }
};
} // namespace sqkctl::patch