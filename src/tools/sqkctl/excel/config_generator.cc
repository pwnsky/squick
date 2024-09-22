
/*

*/
#include "config_generator.h"
#include "utf8_to_gbk.h"
#include <iostream>

// #include "config_generator.h"

namespace sqkctl {
Test::Test() {}

ConfigGenerator::ConfigGenerator(const std::string &excelPath, const std::string &outPath) {
    mxGenerators.push_back(new CPPGenerator(excelPath, outPath));
    mxGenerators.push_back(new CSGenerator(excelPath, outPath));
    mxGenerators.push_back(new IniGenerator(excelPath, outPath));
    mxGenerators.push_back(new LogicClassGenerator(excelPath, outPath));
    mxGenerators.push_back(new StructGenerator(excelPath, outPath));
    mxGenerators.push_back(new LuaGenerator(excelPath, outPath));

    strExcelIniPath = excelPath;
    strXMLStructPath = outPath + "/struct";
    strXMLIniPath = outPath + "/ini";
    this->outPath = outPath;
}

ConfigGenerator::~ConfigGenerator() {}

bool ConfigGenerator::LoadDataFromExcel() {
    LoadDataFromExcel(strExcelIniPath + "/common/IObject.xlsx", "IObject");

    auto fileList = Files::GetFileListInFolder(strExcelIniPath, 1);

    ///////////////////////////////////

    for (auto filePath : fileList) {
        Files::StringReplace(filePath, "\\", "/");
        Files::StringReplace(filePath, "//", "/");
        // std::cout << "Open Excel : " << filePath << std::endl;
        if ((int)(filePath.find("$")) != -1) {
            continue;
        }

        auto strExt = Files::GetFileNameExtByPath(filePath);
        if (strExt != ".xlsx") {
            continue;
        }

        auto fileName = Files::GetFileNameByPath(filePath);
        if (fileName == "IObject") {
            continue;
        }
        if (fileName == "Include") {
            continue;
        }
        if (!LoadDataFromExcel(filePath, fileName)) {
            std::cout << "Create " + fileName + " failed!" << std::endl;
            continue;
        }
    }

    // auto sideFolderList = GetFolderListInFolder(strExcelIniPath);

    ProcessParts();
    ProcessIncludeFiles();
    ProcessRefFiles();

    return true;
}

bool ConfigGenerator::LoadDataFromExcel(const std::string &filePath, const std::string &fileName) {
    if (mxClassData.find(fileName) != mxClassData.end()) {
        std::cout << filePath << " exist!!!" << std::endl;
        return false;
    }

    std::cout << filePath << std::endl;

    ClassData *pClassData = new ClassData();
    pClassData->xStructData.className = fileName;
    mxClassData[pClassData->xStructData.className] = pClassData;

    pClassData->filePath = filePath;
    Files::StringReplace(pClassData->filePath, Files::GetFileNameExtByPath(filePath), "");

    pClassData->fileFolder = pClassData->filePath;
    Files::StringReplace(pClassData->fileFolder, fileName, "");
    ///////////////////////////////////////
    // load

    mini_excel_reader::ExcelFile *xExcel = new mini_excel_reader::ExcelFile();
    if (!xExcel->open(filePath.c_str())) {
        std::cout << "can't open" << filePath << std::endl;
        return false;
    }

    std::vector<mini_excel_reader::Sheet> &sheets = xExcel->sheets();
    for (mini_excel_reader::Sheet &sh : sheets) {
        LoadDataFromExcel(sh, pClassData);
    }

    // load parts

    auto fileList = Files::GetFileListInFolder(pClassData->filePath, 0);
    for (auto filePath : fileList) {
        Files::StringReplace(filePath, "\\", "/");
        Files::StringReplace(filePath, "//", "/");

        std::string fileName = Files::GetFileNameByPath(filePath);
        if (!fileName.empty()) {
            std::string compareName = fileName;
            transform(compareName.begin(), compareName.end(), compareName.begin(), ::tolower);

            if (compareName == "include") {
                LoadIncludeExcel(pClassData, filePath, fileName);
            } else if (compareName == "ref") {
                pClassData->ref.push_back(fileName);
            } else {
                pClassData->parts.push_back(fileName);
            }
        }
    }

    return true;
}

bool ConfigGenerator::LoadIncludeExcel(ClassData *pClassData, const std::string &strFile, const std::string &fileName) {
    mini_excel_reader::ExcelFile *xExcel = new mini_excel_reader::ExcelFile();
    if (!xExcel->open(strFile.c_str())) {
        std::cout << "can't open" << strFile << std::endl;
        return false;
    }

    std::vector<mini_excel_reader::Sheet> &sheets = xExcel->sheets();
    for (mini_excel_reader::Sheet &sh : sheets) {
        LoadDataFromExcel(sh, pClassData);
    }

    return true;
}

bool ConfigGenerator::LoadDataFromExcel(mini_excel_reader::Sheet &sheet, ClassData *pClassData) {
    const mini_excel_reader::Range &dim = sheet.getDimension();

    std::string strSheetName = sheet.getName();
    transform(strSheetName.begin(), strSheetName.end(), strSheetName.begin(), ::tolower);

    if (strSheetName.find("property") != std::string::npos) {
        LoadDataAndProcessProperty(sheet, pClassData);
        LoadIniData(sheet, pClassData);
    } else if (strSheetName.find("component") != std::string::npos) {
        LoadDataAndProcessComponent(sheet, pClassData);
    } else if (strSheetName.find("record") != std::string::npos) {
        LoadDataAndProcessRecord(sheet, pClassData);
    } else if (strSheetName.find("include") != std::string::npos) {
        LoadDataAndProcessIncludes(sheet, pClassData);
    } else if (strSheetName.find("ref") != std::string::npos) {
        LoadDataAndProcessRef(sheet, pClassData);
    } else {
        std::cout << pClassData->xStructData.className << " " << strSheetName << std::endl;
        assert(0);
    }

    return true;
}

bool ConfigGenerator::LoadIniData(mini_excel_reader::Sheet &sheet, ClassData *pClassData) {
    const mini_excel_reader::Range &dim = sheet.getDimension();
    std::string strSheetName = sheet.getName();
    transform(strSheetName.begin(), strSheetName.end(), strSheetName.begin(), ::tolower);

    std::map<std::string, int> PropertyIndex; // col index
    for (int c = dim.firstCol + 1; c <= dim.lastCol; c++) {
        mini_excel_reader::Cell *cell = sheet.getCell(dim.firstRow, c);
        if (cell) {
            PropertyIndex[cell->value] = c;
        }
    }
    ////////////
    for (int r = dim.firstRow + nPropertyHeight; r <= dim.lastRow; r++) {
        mini_excel_reader::Cell *pIDCell = sheet.getCell(r, dim.firstCol);
        if (pIDCell && !pIDCell->value.empty()) {
            ClassElement::ElementData *pIniObject = new ClassElement::ElementData();
            pClassData->xIniData.xElementList[pIDCell->value] = pIniObject;

            for (std::map<std::string, int>::iterator itProperty = PropertyIndex.begin(); itProperty != PropertyIndex.end(); ++itProperty) {
                std::string propertyName = itProperty->first;
                int col = itProperty->second;

                mini_excel_reader::Cell *cell = sheet.getCell(r, col);
                if (cell) {
                    pIniObject->xPropertyList[propertyName] = cell->value;
                } else {
                    pIniObject->xPropertyList[propertyName] = "";
                }
            }
        }
    }

    return false;
}

bool ConfigGenerator::LoadDataAndProcessProperty(mini_excel_reader::Sheet &sheet, ClassData *pClassData) {
    const mini_excel_reader::Range &dim = sheet.getDimension();
    std::string strSheetName = sheet.getName();
    transform(strSheetName.begin(), strSheetName.end(), strSheetName.begin(), ::tolower);

    std::map<std::string, int> descIndex;     // row index
    std::map<std::string, int> PropertyIndex; // col index
    for (int r = dim.firstRow + 1; r <= dim.firstRow + nPropertyHeight - 1; r++) {
        mini_excel_reader::Cell *cell = sheet.getCell(r, dim.firstCol);
        if (cell) {
            descIndex[cell->value] = r;
        }
    }

    for (int c = dim.firstCol + 1; c <= dim.lastCol; c++) {
        mini_excel_reader::Cell *cell = sheet.getCell(dim.firstRow, c);
        if (cell) {
            PropertyIndex[cell->value] = c;
        }
    }
    ////////////

    for (std::map<std::string, int>::iterator itProperty = PropertyIndex.begin(); itProperty != PropertyIndex.end(); ++itProperty) {
        std::string propertyName = itProperty->first;
        int col = itProperty->second;

        ////////////
        ClassProperty *pClassProperty = new ClassProperty();
        pClassData->xStructData.xPropertyList[propertyName] = pClassProperty;

        ////////////
        for (std::map<std::string, int>::iterator itDesc = descIndex.begin(); itDesc != descIndex.end(); ++itDesc) {
            std::string descName = itDesc->first;
            int row = itDesc->second;

            mini_excel_reader::Cell *pCell = sheet.getCell(row, col);
            if (pCell) {
                std::string descValue = pCell->value;

                pClassProperty->descList[descName] = descValue;
            }
        }
    }

    return false;
}

bool ConfigGenerator::LoadDataAndProcessComponent(mini_excel_reader::Sheet &sheet, ClassData *pClassData) {
    const mini_excel_reader::Range &dim = sheet.getDimension();
    std::string strSheetName = sheet.getName();
    transform(strSheetName.begin(), strSheetName.end(), strSheetName.begin(), ::tolower);

    std::vector<std::string> colNames;
    for (int c = dim.firstCol; c <= dim.lastCol; c++) {
        mini_excel_reader::Cell *cell = sheet.getCell(dim.firstRow, c);
        if (cell) {
            colNames.push_back(cell->value);
        }
    }
    for (int r = dim.firstRow + 1; r <= dim.lastRow; r++) {
        std::string testValue = "";
        mini_excel_reader::Cell *cell = sheet.getCell(r, dim.firstCol);
        if (cell) {
            testValue = cell->value;
        }
        if (testValue == "") {
            continue;
        }
        // auto componentNode = structDoc.allocate_node(rapidxml::node_element, "Component", NULL);
        // componentNodes->append_node(componentNode);
        std::string strType = "";
        for (int c = dim.firstCol; c <= dim.lastCol; c++) {
            std::string name = colNames[c - 1];
            std::string value = "";
            mini_excel_reader::Cell *cell = sheet.getCell(r, c);
            if (cell) {
                std::string valueCell = cell->value;
                transform(valueCell.begin(), valueCell.end(), valueCell.begin(), ::toupper);
                if (valueCell == "TRUE" || valueCell == "FALSE") {
                    value = valueCell == "TRUE" ? "1" : "0";
                } else {
                    value = cell->value;
                }

                if (name == "Type") {
                    strType = value;
                }
            }
            // componentNode->append_attribute(structDoc.allocate_attribute(NewChar(name), NewChar(value)));
        }
    }
    return false;
}

bool ConfigGenerator::LoadDataAndProcessRecord(mini_excel_reader::Sheet &sheet, ClassData *pClassData) {
    const mini_excel_reader::Range &dim = sheet.getDimension();
    std::string strSheetName = sheet.getName();
    transform(strSheetName.begin(), strSheetName.end(), strSheetName.begin(), ::tolower);

    for (int nIndex = 0; nIndex < 100; nIndex++) {
        int nStartRow = nIndex * nRecordHeight + 1;
        int nEndRow = (nIndex + 1) * nRecordHeight;

        mini_excel_reader::Cell *pTestCell = sheet.getCell(nStartRow, dim.firstCol);
        if (pTestCell) {
            mini_excel_reader::Cell *pNameCell = sheet.getCell(nStartRow, dim.firstCol + 1);
            std::string recordName = pNameCell->value;

            ////////////

            ClassRecord *pClassRecord = new ClassRecord();
            pClassData->xStructData.xRecordList[recordName] = pClassRecord;
            ////////////

            for (int r = nStartRow + 1; r <= nStartRow + nRecordDescHeight; r++) {
                mini_excel_reader::Cell *cellDesc = sheet.getCell(r, dim.firstCol);
                mini_excel_reader::Cell *cellValue = sheet.getCell(r, dim.firstCol + 1);

                pClassRecord->descList[cellDesc->value] = cellValue->value;
            }

            int nRecordCol = atoi(pClassRecord->descList["Col"].c_str());
            for (int c = dim.firstCol; c <= nRecordCol; c++) {
                int r = nStartRow + nRecordDescHeight + 1;
                mini_excel_reader::Cell *pCellColName = sheet.getCell(r, c);
                mini_excel_reader::Cell *pCellColType = sheet.getCell(r + 1, c);
                mini_excel_reader::Cell *pCellColDesc = sheet.getCell(r + 2, c);

                ClassRecord::RecordColDesc *pRecordColDesc = new ClassRecord::RecordColDesc();
                pRecordColDesc->index = c - 1;
                pRecordColDesc->type = pCellColType->value;
                if (pCellColDesc) {
                    pRecordColDesc->desc = pCellColDesc->value;
                }

                pClassRecord->colList[pCellColName->value] = pRecordColDesc;
            }
        }
    }

    return true;
}

bool ConfigGenerator::LoadDataAndProcessIncludes(mini_excel_reader::Sheet &sheet, ClassData *pClassData) {
    const mini_excel_reader::Range &dim = sheet.getDimension();
    std::string strSheetName = sheet.getName();
    transform(strSheetName.begin(), strSheetName.end(), strSheetName.begin(), ::tolower);

    for (int r = dim.firstRow; r <= dim.lastRow; r++) {
        mini_excel_reader::Cell *cell = sheet.getCell(r, dim.firstCol);
        if (cell) {
            std::string valueCell = cell->value;
            bool included = false;
            for (std::string fileName : pClassData->includes) {
                if (valueCell == fileName) {
                    included = true;
                    break;
                }
            }

            if (!included) {
                pClassData->includes.push_back(valueCell);
            }
        }
    }

    return true;
}

bool ConfigGenerator::LoadDataAndProcessRef(mini_excel_reader::Sheet &sheet, ClassData *pClassData) {
    const mini_excel_reader::Range &dim = sheet.getDimension();
    std::string strSheetName = sheet.getName();
    transform(strSheetName.begin(), strSheetName.end(), strSheetName.begin(), ::tolower);

    for (int r = dim.firstRow; r <= dim.lastRow; r++) {
        mini_excel_reader::Cell *cell = sheet.getCell(r, dim.firstCol);
        if (cell) {
            std::string valueCell = cell->value;
            bool included = false;
            for (std::string fileName : pClassData->ref) {
                if (valueCell == fileName) {
                    included = true;
                    break;
                }
            }

            if (!included) {
                pClassData->ref.push_back(valueCell);
            }
        }
    }

    return true;
}

bool ConfigGenerator::GenerateData() {
    for (auto generator : mxGenerators) {
        generator->Generate(mxClassData);
    }

    return false;
}

void ConfigGenerator::SetUTF8(const bool b) { bConvertIntoUTF8 = b; }

void ConfigGenerator::PrintData() {
    for (auto data : mxClassData) {
        PrintData(data.second);
    }
}

void ConfigGenerator::PrintData(ClassData *data) {
    std::cout << "------------";
    std::cout << "Class Name:" << data->xStructData.className << std::endl;
    std::cout << "File Path:" << data->filePath << std::endl;
    std::cout << "File Folder:" << data->fileFolder << std::endl;
    std::cout << "Be Included:" << data->beIncluded << std::endl;

    std::cout << "File Include:" << std::endl;
    for (auto file : data->includes) {
        std::cout << file << std::endl;
    }

    std::cout << "Be Parted:" << data->beParted << std::endl;

    std::cout << "File Parents:" << std::endl;
    for (auto file : data->parents) {
        std::cout << file << std::endl;
    }

    std::cout << "File Parts:" << std::endl;
    for (auto file : data->parts) {
        std::cout << file << std::endl;
    }

    std::cout << "Ref:" << data->ref.size() << std::endl;
    std::cout << "File Ref:" << std::endl;
    for (auto file : data->ref) {
        std::cout << file << std::endl;
    }
}

void ConfigGenerator::ProcessParts() {
    std::vector<ClassData *> parentObjects;
    for (auto classData : mxClassData) {
        if (classData.second->parts.size() > 0) {
            parentObjects.push_back(classData.second);
        }
    }

    for (auto parentObject : parentObjects) {
        for (auto className : parentObject->parts) {
            auto it = mxClassData.find(className);
            ClassData *partialClass = it->second;
            if (partialClass) {
                partialClass->beParted = true;
                for (auto property : partialClass->xStructData.xPropertyList) {
                    parentObject->xStructData.xPropertyList[property.first] = property.second;
                }

                for (auto record : partialClass->xStructData.xRecordList) {
                    parentObject->xStructData.xRecordList[record.first] = record.second;
                }

                for (auto element : partialClass->xIniData.xElementList) {
                    parentObject->xIniData.xElementList[element.first] = element.second;
                }

                // partial class doesnt need to generate Ini xml file
                partialClass->xIniData.xElementList.clear();
            }
        }
    }
}

void ConfigGenerator::ProcessIncludeFiles() {
    std::vector<ClassData *> parentObjects;
    for (auto classData : mxClassData) {
        if (classData.second->includes.size() > 0) {
            parentObjects.push_back(classData.second);
        }
    }

    for (auto parentObject : parentObjects) {
        for (auto className : parentObject->includes) {
            auto it = mxClassData.find(className);
            ClassData *subClass = it->second;

            if (subClass) {
                subClass->beIncluded = true;
                subClass->parents.push_back(parentObject->xStructData.className);

                for (auto property : subClass->xStructData.xPropertyList) {
                    // parentObject->xStructData.xPropertyList.insert(std::make_pair(property.first, property.second));
                }

                for (auto record : subClass->xStructData.xRecordList) {
                    // parentObject->xStructData.xRecordList.insert(std::make_pair(record.first, record.second));
                }
            }
        }
    }
}

void ConfigGenerator::ProcessRefFiles() {
    std::vector<ClassData *> parentObjects;
    for (auto classData : mxClassData) {
        if (classData.second->ref.size() > 0) {
            parentObjects.push_back(classData.second);
        }
    }

    for (auto parentObject : parentObjects) {
        for (auto className : parentObject->ref) {
            auto it = mxClassData.find(className);
            ClassData *subClass = it->second;

            if (subClass) {
                for (auto property : subClass->xStructData.xPropertyList) {
                    parentObject->xStructData.xPropertyList.insert(std::make_pair(property.first, property.second));
                }

                for (auto record : subClass->xStructData.xRecordList) {
                    parentObject->xStructData.xRecordList.insert(std::make_pair(record.first, record.second));
                }
            }
        }
    }
}

} // namespace sqkctl
