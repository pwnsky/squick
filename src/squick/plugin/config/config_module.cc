

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
    SQUICK_SHARE_PTR<CStructInfo> pSDKInfo = mmData.GetElement(strStructName);
    if (pSDKInfo) {
        SQUICK_SHARE_PTR<CAttributeList> pParam = pSDKInfo->GetElement(strStructItemName);
        if (pParam) {
            return pParam->GetInt(strAttribute);
        }
    }

    return 0;
}

const std::string &ConfigModule::GetFieldString(const std::string &strStructName, const std::string &strStructItemName, const std::string &strAttribute) {
    SQUICK_SHARE_PTR<CStructInfo> pSDKInfo = mmData.GetElement(strStructName);
    if (pSDKInfo) {
        SQUICK_SHARE_PTR<CAttributeList> pParam = pSDKInfo->GetElement(strStructItemName);
        if (pParam) {
            return pParam->GetString(strAttribute);
        }
    }

    return NULL_STR;
}

const int ConfigModule::GetFieldInt(const std::string &strStructName, const std::string &strSDKAttribute) {
    SQUICK_SHARE_PTR<CStructInfo> pSDKInfo = mmData.GetElement(strStructName);
    if (pSDKInfo) {
        return pSDKInfo->mmStructAttribute.GetInt(strSDKAttribute);
    }

    return 0;
}
const std::string &ConfigModule::GetFieldString(const std::string &strStructName, const std::string &strSDKAttribute) {
    SQUICK_SHARE_PTR<CStructInfo> pSDKInfo = mmData.GetElement(strStructName);
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
            SQUICK_SHARE_PTR<CStructInfo> pAppSDKConfigInfo = mmData.GetElement(pRootNode->name());
            if (!pAppSDKConfigInfo) {
                pAppSDKConfigInfo = SQUICK_SHARE_PTR<CStructInfo>(SQUICK_NEW CStructInfo());
                mmData.AddElement(pRootNode->name(), pAppSDKConfigInfo);
            }

            for (rapidxml::xml_attribute<> *pAttribute = pRootNode->first_attribute(); pAttribute; pAttribute = pAttribute->next_attribute()) {
                const char *pstrConfigName = pAttribute->name();
                const char *pstrConfigValue = pAttribute->value();

                pAppSDKConfigInfo->mmStructAttribute.AddElement(pstrConfigName, SQUICK_SHARE_PTR<std::string>(SQUICK_NEW std::string(pstrConfigValue)));
            }

            for (rapidxml::xml_node<> *pSDKInterfaceNode = pRootNode->first_node(); pSDKInterfaceNode; pSDKInterfaceNode = pSDKInterfaceNode->next_sibling()) {
                if (pSDKInterfaceNode->first_attribute("ID") != NULL) {
                    std::string strStructItemName = pSDKInterfaceNode->first_attribute("ID")->value();
                    SQUICK_SHARE_PTR<CAttributeList> pConfigData = pAppSDKConfigInfo->GetElement(strStructItemName);
                    if (!pConfigData) {
                        pConfigData = SQUICK_SHARE_PTR<CAttributeList>(SQUICK_NEW CAttributeList());
                        pAppSDKConfigInfo->AddElement(strStructItemName, pConfigData);
                    }

                    for (rapidxml::xml_attribute<> *pAttribute = pSDKInterfaceNode->first_attribute(); pAttribute; pAttribute = pAttribute->next_attribute()) {
                        const char *pstrConfigName = pAttribute->name();
                        const char *pstrConfigValue = pAttribute->value();

                        pConfigData->AddElement(pstrConfigName, SQUICK_SHARE_PTR<std::string>(SQUICK_NEW std::string(pstrConfigValue)));
                    }
                } else {
                    for (rapidxml::xml_attribute<> *pAttribute = pSDKInterfaceNode->first_attribute(); pAttribute; pAttribute = pAttribute->next_attribute()) {
                        const char *pstrConfigName = pAttribute->name();
                        const char *pstrConfigValue = pAttribute->value();

                        pAppSDKConfigInfo->mmStructAttribute.AddElement(pstrConfigName, SQUICK_SHARE_PTR<std::string>(SQUICK_NEW std::string(pstrConfigValue)));
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
    SQUICK_SHARE_PTR<CStructInfo> pStructTypeData = mmData.GetElement(strStructName);
    if (pStructTypeData) {
        std::string strStructName;
        for (SQUICK_SHARE_PTR<CAttributeList> pData = pStructTypeData->First(strStructName); pData != NULL; pData = pStructTypeData->Next(strStructName)) {
            xList.push_back(strStructName);
        }
    }

    return xList;
}

std::vector<std::string> ConfigModule::GetFieldList(const std::string &strStructName) {
    std::vector<std::string> xList;
    SQUICK_SHARE_PTR<CStructInfo> pStructTypeData = mmData.GetElement(strStructName);
    if (pStructTypeData) {
        std::string strStructName;
        for (SQUICK_SHARE_PTR<std::string> pData = pStructTypeData->mmStructAttribute.First(strStructName); pData != NULL;
             pData = pStructTypeData->mmStructAttribute.Next(strStructName)) {
            xList.push_back(strStructName);
        }
    }

    return xList;
}
