#include "config_module.h"
#include "third_party/rapidxml/rapidxml_utils.hpp"

bool ConfigModule::Start() { return true; }

bool ConfigModule::Destory() { return true; }

bool ConfigModule::Update() { return true; }

bool ConfigModule::AfterStart() { return true; }

bool ConfigModule::ClearConfig() {
    mmData.ClearAll();

    return true;
}

const int ConfigModule::GetFieldInt(const std::string &strStructName, const std::string &strStructItemName, const std::string &strAttribute) {
    std::shared_ptr<CStructInfo> pSDKInfo = mmData.GetElement(strStructName);
    if (pSDKInfo) {
        std::shared_ptr<CAttributeList> pParam = pSDKInfo->GetElement(strStructItemName);
        if (pParam) {
            return pParam->GetInt(strAttribute);
        }
    }

    return 0;
}

const std::string &ConfigModule::GetFieldString(const std::string &strStructName, const std::string &strStructItemName, const std::string &strAttribute) {
    std::shared_ptr<CStructInfo> pSDKInfo = mmData.GetElement(strStructName);
    if (pSDKInfo) {
        std::shared_ptr<CAttributeList> pParam = pSDKInfo->GetElement(strStructItemName);
        if (pParam) {
            return pParam->GetString(strAttribute);
        }
    }

    return NULL_STR;
}

const int ConfigModule::GetFieldInt(const std::string &strStructName, const std::string &strSDKAttribute) {
    std::shared_ptr<CStructInfo> pSDKInfo = mmData.GetElement(strStructName);
    if (pSDKInfo) {
        return pSDKInfo->mmStructAttribute.GetInt(strSDKAttribute);
    }

    return 0;
}
const std::string &ConfigModule::GetFieldString(const std::string &strStructName, const std::string &strSDKAttribute) {
    std::shared_ptr<CStructInfo> pSDKInfo = mmData.GetElement(strStructName);
    if (pSDKInfo) {
        return pSDKInfo->mmStructAttribute.GetString(strSDKAttribute);
    }

    return NULL_STR;
}

bool ConfigModule::LoadConfig(const std::string &strFile) {
    try {
        rapidxml::file<> xFile(strFile.c_str());
        rapidxml::xml_document<> xDoc;
        xDoc.parse<0>(xFile.data());

        rapidxml::xml_node<> *pRoot = xDoc.first_node();
        if (NULL == pRoot) {
            return false;
        }

        for (rapidxml::xml_node<> *pRootNode = pRoot->first_node(); pRootNode; pRootNode = pRootNode->next_sibling()) {
            std::shared_ptr<CStructInfo> pAppSDKConfigInfo = mmData.GetElement(pRootNode->name());
            if (!pAppSDKConfigInfo) {
                pAppSDKConfigInfo = std::shared_ptr<CStructInfo>(new CStructInfo());
                mmData.AddElement(pRootNode->name(), pAppSDKConfigInfo);
            }

            for (rapidxml::xml_attribute<> *pAttribute = pRootNode->first_attribute(); pAttribute; pAttribute = pAttribute->next_attribute()) {
                const char *pstrConfigName = pAttribute->name();
                const char *pstrConfigValue = pAttribute->value();

                pAppSDKConfigInfo->mmStructAttribute.AddElement(pstrConfigName, std::shared_ptr<std::string>(new std::string(pstrConfigValue)));
            }

            for (rapidxml::xml_node<> *pSDKInterfaceNode = pRootNode->first_node(); pSDKInterfaceNode; pSDKInterfaceNode = pSDKInterfaceNode->next_sibling()) {
                if (pSDKInterfaceNode->first_attribute("ID") != NULL) {
                    std::string strStructItemName = pSDKInterfaceNode->first_attribute("ID")->value();
                    std::shared_ptr<CAttributeList> pConfigData = pAppSDKConfigInfo->GetElement(strStructItemName);
                    if (!pConfigData) {
                        pConfigData = std::shared_ptr<CAttributeList>(new CAttributeList());
                        pAppSDKConfigInfo->AddElement(strStructItemName, pConfigData);
                    }

                    for (rapidxml::xml_attribute<> *pAttribute = pSDKInterfaceNode->first_attribute(); pAttribute; pAttribute = pAttribute->next_attribute()) {
                        const char *pstrConfigName = pAttribute->name();
                        const char *pstrConfigValue = pAttribute->value();

                        pConfigData->AddElement(pstrConfigName, std::shared_ptr<std::string>(new std::string(pstrConfigValue)));
                    }
                } else {
                    for (rapidxml::xml_attribute<> *pAttribute = pSDKInterfaceNode->first_attribute(); pAttribute; pAttribute = pAttribute->next_attribute()) {
                        const char *pstrConfigName = pAttribute->name();
                        const char *pstrConfigValue = pAttribute->value();

                        pAppSDKConfigInfo->mmStructAttribute.AddElement(pstrConfigName, std::shared_ptr<std::string>(new std::string(pstrConfigValue)));
                    }
                }
            }
        }
    } catch (...) {
        return false;
    }

    return true;
}

std::vector<std::string> ConfigModule::GetSubKeyList(const std::string &strStructName) {
    std::vector<std::string> xList;
    std::shared_ptr<CStructInfo> pStructTypeData = mmData.GetElement(strStructName);
    if (pStructTypeData) {
        std::string strStructName;
        for (std::shared_ptr<CAttributeList> pData = pStructTypeData->First(strStructName); pData != NULL; pData = pStructTypeData->Next(strStructName)) {
            xList.push_back(strStructName);
        }
    }

    return xList;
}

std::vector<std::string> ConfigModule::GetFieldList(const std::string &strStructName) {
    std::vector<std::string> xList;
    std::shared_ptr<CStructInfo> pStructTypeData = mmData.GetElement(strStructName);
    if (pStructTypeData) {
        std::string strStructName;
        for (std::shared_ptr<std::string> pData = pStructTypeData->mmStructAttribute.First(strStructName); pData != NULL;
             pData = pStructTypeData->mmStructAttribute.Next(strStructName)) {
            xList.push_back(strStructName);
        }
    }

    return xList;
}
