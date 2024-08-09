#pragma once

#include "i_generator.h"
namespace sqkctl {
class CSGenerator : public IGenerator {
  public:
    CSGenerator(const std::string &excelPath, const std::string &outPath) { SetPath(excelPath, outPath); }

    virtual bool Generate(const std::map<std::string, ClassData *> &classData) override {
        FILE *csWriter = fopen((outPath + "/excel/Excel.cs").c_str(), "w");

        std::string strFileHead = "// -------------------------------------------------------------------------\n";
        strFileHead = strFileHead + "//    @FileName         :    Excel.cs\n" + "//    @Author           :    I0gan\n" +
                      "//    @Module           :    Excel\n" + "// -------------------------------------------------------------------------\n\n" +
                      "using System;\n"
                      //+ "using System.Collections.Concurrent;\n"
                      + "using System.Collections.Generic;\n" + "using System.Linq;\n" + "using System.Text;\n" +
                      "using System.Threading;\n"
                      //+ "using System.Threading.Tasks;\n\n"
                      + "namespace Excel\n{\n";

        fwrite(strFileHead.c_str(), strFileHead.length(), 1, csWriter);
        /////////////////////////////////////////////////////

        ClassData *pBaseObject = classData.at("IObject");
        for (std::map<std::string, ClassData *>::const_iterator it = classData.begin(); it != classData.end(); ++it) {
            const std::string &className = it->first;
            ClassData *pClassDta = it->second;

            if (pClassDta->beIncluded || pClassDta->beParted || className == "Include") {
                continue;
            }

            // cs
            std::string strPropertyInfo;

            strPropertyInfo += "\tpublic class " + className + "\n\t{\n";
            strPropertyInfo += "\t\t//Class name\n\t";
            strPropertyInfo += "\tpublic static readonly String ThisName = \"" + className + "\";\n";

            if (className != "IObject") {
                // add base class properties
                strPropertyInfo += "\t\t// IObject\n";

                for (std::map<std::string, ClassProperty *>::iterator itProperty = pBaseObject->xStructData.xPropertyList.begin();
                     itProperty != pBaseObject->xStructData.xPropertyList.end(); ++itProperty) {
                    const std::string &propertyName = itProperty->first;
                    ClassProperty *pClassProperty = itProperty->second;

                    strPropertyInfo += "\t\tpublic static readonly String " + propertyName + " = \"" + propertyName + "\";";
                    strPropertyInfo += "// " + pClassProperty->descList["Type"] + "\n";
                }
            }

            strPropertyInfo += "\t\t// Property\n";
            for (std::map<std::string, ClassProperty *>::iterator itProperty = pClassDta->xStructData.xPropertyList.begin();
                 itProperty != pClassDta->xStructData.xPropertyList.end(); ++itProperty) {
                const std::string &propertyName = itProperty->first;
                ClassProperty *pClassProperty = itProperty->second;

                strPropertyInfo += "\t\tpublic static readonly String " + propertyName + " = \"" + propertyName + "\";";
                strPropertyInfo += "// " + pClassProperty->descList["Type"] + "\n";
            }

            // include property, come from

            for (std::string item : pClassDta->includes) {
                auto includeClass = classData.at(item);

                strPropertyInfo += "\t\t// Include Property, come from " + includeClass->xStructData.className + " \n";

                for (std::map<std::string, ClassProperty *>::iterator itProperty = includeClass->xStructData.xPropertyList.begin();
                     itProperty != includeClass->xStructData.xPropertyList.end(); ++itProperty) {
                    const std::string &propertyName = itProperty->first;
                    ClassProperty *pClassProperty = itProperty->second;

                    strPropertyInfo += "\t\tpublic static readonly String " + propertyName + " = \"" + propertyName + "\";";
                    strPropertyInfo += "// " + pClassProperty->descList["Type"] + "\n";
                }
            }

            fwrite(strPropertyInfo.c_str(), strPropertyInfo.length(), 1, csWriter);

            // record
            std::string strRecordInfo = "";
            strRecordInfo += "\t\t// Record\n";

            for (std::map<std::string, ClassRecord *>::iterator itRecord = pClassDta->xStructData.xRecordList.begin();
                 itRecord != pClassDta->xStructData.xRecordList.end(); ++itRecord) {
                const std::string &recordName = itRecord->first;
                ClassRecord *pClassRecord = itRecord->second;

                std::cout << "save for cs ---> " << className << "::" << recordName << std::endl;

                strRecordInfo += "\t\tpublic class " + recordName + "\n\t\t{\n";
                strRecordInfo += "\t\t\t//Class name\n\t";
                strRecordInfo += "\t\tpublic static readonly String ThisName = \"" + recordName + "\";\n";

                // col
                for (int i = 0; i < pClassRecord->colList.size(); ++i) {
                    for (std::map<std::string, ClassRecord::RecordColDesc *>::iterator itCol = pClassRecord->colList.begin();
                         itCol != pClassRecord->colList.end(); ++itCol) {
                        const std::string &colTag = itCol->first;
                        ClassRecord::RecordColDesc *pRecordColDesc = itCol->second;

                        if (pRecordColDesc->index == i) {
                            strRecordInfo +=
                                "\t\t\tpublic const int " + colTag + " = " + std::to_string(pRecordColDesc->index) + ";//" + pRecordColDesc->type + "\n";
                        }
                    }
                }

                strRecordInfo += "\n\t\t}\n";
            }
            // include record

            for (std::string item : pClassDta->includes) {
                auto includeClass = classData.at(item);

                strPropertyInfo += "\t\t// Include Record, come from " + includeClass->xStructData.className + " \n";

                for (std::map<std::string, ClassRecord *>::iterator itRecord = includeClass->xStructData.xRecordList.begin();
                     itRecord != includeClass->xStructData.xRecordList.end(); ++itRecord) {
                    const std::string &recordName = itRecord->first;
                    ClassRecord *pClassRecord = itRecord->second;

                    strRecordInfo += "\t\tpublic class " + recordName + "\n\t\t{\n";
                    strRecordInfo += "\t\t\t//Class name\n\t";
                    strRecordInfo += "\t\tpublic static readonly String ThisName = \"" + recordName + "\";\n";

                    // col
                    for (int i = 0; i < pClassRecord->colList.size(); ++i) {
                        for (std::map<std::string, ClassRecord::RecordColDesc *>::iterator itCol = pClassRecord->colList.begin();
                             itCol != pClassRecord->colList.end(); ++itCol) {
                            const std::string &colTag = itCol->first;
                            ClassRecord::RecordColDesc *pRecordColDesc = itCol->second;

                            if (pRecordColDesc->index == i) {
                                strRecordInfo +=
                                    "\t\t\tpublic const int " + colTag + " = " + std::to_string(pRecordColDesc->index) + ";//" + pRecordColDesc->type + "\n";
                            }
                        }
                    }

                    strRecordInfo += "\n\t\t}\n";
                }
            }

            fwrite(strRecordInfo.c_str(), strRecordInfo.length(), 1, csWriter);

            std::string strHppEnumInfo = "";

            std::string strClassEnd;
            strClassEnd += "\n\t}\n";

            fwrite(strClassEnd.c_str(), strClassEnd.length(), 1, csWriter);
        }

        std::string strFileEnd = "\n}";
        fwrite(strFileEnd.c_str(), strFileEnd.length(), 1, csWriter);
        fclose(csWriter);

        return false;
    }
};
} // namespace sqkctl
