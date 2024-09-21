#pragma once

#include "cpp_generator.h"
#include "cs_generator.h"
#include "ini_generator.h"
#include "logic_class_generator.h"
#include "lua_generator.h"
#include "mini_excel_reader.h"
#include "struct_generator.h"
#include "third_party/common/lexical_cast.hpp"
#include <squick/core/platform.h>

namespace sqkctl {
class Test {
  public:
    Test();
};

class ConfigGenerator {
  public:
    ConfigGenerator(const std::string &excelPath, const std::string &out_path);
    virtual ~ConfigGenerator();
    bool LoadDataFromExcel();
    void SetUTF8(const bool b);
    bool GenerateData();
    void PrintData();
    void PrintData(ClassData *data);

  private:
    bool LoadDataFromExcel(const std::string &filePath, const std::string &fileName);
    bool LoadIncludeExcel(ClassData *pClassData, const std::string &strFile, const std::string &fileName);

    bool LoadDataFromExcel(mini_excel_reader::Sheet &sheet, ClassData *pClassData);

    bool LoadIniData(mini_excel_reader::Sheet &sheet, ClassData *pClassData);
    bool LoadDataAndProcessProperty(mini_excel_reader::Sheet &sheet, ClassData *pClassData);
    bool LoadDataAndProcessComponent(mini_excel_reader::Sheet &sheet, ClassData *pClassData);
    bool LoadDataAndProcessRecord(mini_excel_reader::Sheet &sheet, ClassData *pClassData);
    bool LoadDataAndProcessIncludes(mini_excel_reader::Sheet &sheet, ClassData *pClassData);
    bool LoadDataAndProcessRef(mini_excel_reader::Sheet &sheet, ClassData *pClassData);

    void ProcessParts();
    void ProcessIncludeFiles();
    void ProcessRefFiles();

  private:
    bool bConvertIntoUTF8 = false;

    // Excel base header attr line define
    const int nPropertyHeight = 3;   // property line
    const int nRecordHeight = 6;     // record line
    const int nRecordDescHeight = 2; // record line

    std::string outPath;
    std::string strExcelIniPath;  // = "../excel/";
    std::string strXMLStructPath; // = "../struct/";
    std::string strXMLIniPath;    // = "../ini/";

    std::map<std::string, ClassData *> mxClassData;
    std::vector<IGenerator *> mxGenerators;
};

} // namespace sqkctl
