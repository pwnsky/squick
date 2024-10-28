
#include "class_module.h"
#include "plugin.h"
#include "third_party/rapidxml/rapidxml.hpp"
#include "third_party/rapidxml/rapidxml_print.hpp"
#include <algorithm>
#include <iostream>
#include <time.h>
ClassModule::ClassModule() { mConfigFileName = "res/XlsxXML/Root.xml"; }

ClassModule::ClassModule(IPluginManager *p) {
    pm_ = p;
    mConfigFileName = "res/XlsxXML/Root.xml";

#ifdef DEBUG
    std::cout << "Using [" << pm_->GetWorkPath() << "/" + mConfigFileName << "]" << std::endl;
#endif

    if (!this->mbBackup) {
        for (int i = 0; i < CONFIG_DATA_THREADS_CAN_BE_USED_CNT; ++i) {
            ThreadClassModule threadElement;
            threadElement.used = false;
            threadElement.classModule = new ClassModule();
            threadElement.classModule->mbBackup = true;
            threadElement.classModule->pm_ = pm_;

            threadElement.classModule->Awake();
            threadElement.classModule->Start();
            threadElement.classModule->AfterStart();

            mThreadClasses.push_back(threadElement);
        }
    }
}

ClassModule::~ClassModule() { ClearAll(); }

bool ClassModule::Awake() {
    for (int i = 0; i < mThreadClasses.size(); ++i) {
        mThreadClasses[i].classModule->Awake();
    }

    Load();

    return true;
}

bool ClassModule::Start() {
    for (int i = 0; i < mThreadClasses.size(); ++i) {
        mThreadClasses[i].classModule->Start();
    }
    return true;
}

bool ClassModule::Destroy() {
    for (int i = 0; i < mThreadClasses.size(); ++i) {
        mThreadClasses[i].classModule->Destroy();
    }

    ClearAll();

    return true;
}

// Get a free class module for find
IClassModule *ClassModule::GetThreadClassModule() {
    std::thread::id threadID = std::this_thread::get_id();

    for (int i = 0; i < mThreadClasses.size(); ++i) {
        if (mThreadClasses[i].used) {
            if (mThreadClasses[i].threadID == threadID) {
                return mThreadClasses[i].classModule;
            }
        } else {
            // init for this thread
            mThreadClasses[i].used = true;
            mThreadClasses[i].threadID = threadID;
            return mThreadClasses[i].classModule;
        }
    }
    return nullptr;
}

DATA_TYPE ClassModule::ComputerType(const char *pstrTypeName, SquickData &var) {
    if (0 == strcmp(pstrTypeName, "int")) {
        var.SetInt(NULL_INT);
        return var.GetType();
    } else if (0 == strcmp(pstrTypeName, "string")) {
        var.SetString("");
        return var.GetType();
    } else if (0 == strcmp(pstrTypeName, "float")) {
        var.SetFloat(NULL_FLOAT);
        return var.GetType();
    } else if (0 == strcmp(pstrTypeName, "object")) {
        var.SetObject(NULL_OBJECT);
        return var.GetType();
    } else if (0 == strcmp(pstrTypeName, "vector2")) {
        var.SetVector2(NULL_VECTOR2);
        return var.GetType();
    } else if (0 == strcmp(pstrTypeName, "vector3")) {
        var.SetVector3(NULL_VECTOR3);
        return var.GetType();
    }

    return TDATA_UNKNOWN;
}

bool ClassModule::AddProperties(rapidxml::xml_node<> *pPropertyRootNode, std::shared_ptr<IClass> pClass) {
    for (rapidxml::xml_node<> *pPropertyNode = pPropertyRootNode->first_node(); pPropertyNode; pPropertyNode = pPropertyNode->next_sibling()) {
        if (pPropertyNode) {
            const char *propertyName = pPropertyNode->first_attribute("Id")->value();
            if (pClass->GetPropertyManager()->GetElement(propertyName)) {
                // error
                SQUICK_ASSERT(0, propertyName, __FILE__, __FUNCTION__);
                continue;
            }

            const char *pstrType = pPropertyNode->first_attribute("Type")->value();

            SquickData varProperty;
            if (TDATA_UNKNOWN == ComputerType(pstrType, varProperty)) {
                // std::cout << "error:" << pClass->GetTypeName() << "  " << pClass->GetInstancePath() << ": " << propertyName << " type error!!!" << std::endl;

                SQUICK_ASSERT(0, propertyName, __FILE__, __FUNCTION__);
            }

            // printf( " Property:%s[%s]\n", propertyName, pstrType );

            std::shared_ptr<IProperty> xProperty = pClass->GetPropertyManager()->AddProperty(Guid(), propertyName, varProperty.GetType());
        }
    }

    return true;
}

bool ClassModule::AddRecords(rapidxml::xml_node<> *pRecordRootNode, std::shared_ptr<IClass> pClass) {
    for (rapidxml::xml_node<> *pRecordNode = pRecordRootNode->first_node(); pRecordNode; pRecordNode = pRecordNode->next_sibling()) {
        if (pRecordNode) {
            const char *pstrRecordName = pRecordNode->first_attribute("Id")->value();

            if (pClass->GetRecordManager()->GetElement(pstrRecordName)) {
                // error
                // file << pClass->mstrType << ":" << pstrRecordName << std::endl;
                // assert(0);
                SQUICK_ASSERT(0, pstrRecordName, __FILE__, __FUNCTION__);
                continue;
            }

            const char *pstrRow = pRecordNode->first_attribute("Row")->value();
            const char *pstrCol = pRecordNode->first_attribute("Col")->value();
            

            std::string strView;
            if (pRecordNode->first_attribute("View") != NULL) {
                strView = pRecordNode->first_attribute("View")->value();
            }

            std::shared_ptr<DataList> recordVar(new DataList());
            std::shared_ptr<DataList> recordTag(new DataList());

            for (rapidxml::xml_node<> *recordColNode = pRecordNode->first_node(); recordColNode; recordColNode = recordColNode->next_sibling()) {
                // const char* pstrColName = recordColNode->first_attribute( "Id" )->value();
                SquickData TData;
                const char *pstrColType = recordColNode->first_attribute("Type")->value();
                if (TDATA_UNKNOWN == ComputerType(pstrColType, TData)) {
                    // assert(0);
                    SQUICK_ASSERT(0, pstrRecordName, __FILE__, __FUNCTION__);
                }

                recordVar->Append(TData);
                //////////////////////////////////////////////////////////////////////////
                if (recordColNode->first_attribute("Tag") != NULL) {
                    const char *pstrTag = recordColNode->first_attribute("Tag")->value();
                    recordTag->Add(pstrTag);
                } else {
                    recordTag->Add("");
                }
            }

            std::shared_ptr<IRecord> xRecord = pClass->GetRecordManager()->AddRecord(Guid(), pstrRecordName, recordVar, recordTag, atoi(pstrRow));
            
        }
    }

    return true;
}

bool ClassModule::AddComponents(rapidxml::xml_node<> *pComponentRootNode, std::shared_ptr<IClass> pClass) {
    /*
for (rapidxml::xml_node<>* pComponentNode = pComponentRootNode->first_node(); pComponentNode; pComponentNode = pComponentNode->next_sibling())
{
    if (pComponentNode)
    {
        const char* componentName = pComponentNode->first_attribute("Name")->value();
        const char* strLanguage = pComponentNode->first_attribute("Language")->value();
        const char* strEnable = pComponentNode->first_attribute("Enable")->value();
        bool bEnable = lexical_cast<bool>(strEnable);
        if (bEnable)
        {
            if (pClass->GetComponentManager()->GetElement(componentName))
            {
                //error
                SQUICK_ASSERT(0, componentName, __FILE__, __FUNCTION__);
                continue;
            }
            std::shared_ptr<IComponent> xComponent(new IComponent(Guid(), componentName));
            pClass->GetComponentManager()->AddComponent(componentName, xComponent);
        }
    }
}
    */
    return true;
}

bool ClassModule::AddClassInclude(const char *pstrClassFilePath, std::shared_ptr<IClass> pClass) {
    if (pClass->Find(pstrClassFilePath)) {
        return false;
    }

    //////////////////////////////////////////////////////////////////////////
    std::string strFile = pstrClassFilePath;
    std::string content;
    pm_->GetFileContent(strFile, content);

    rapidxml::xml_document<> xDoc;
    xDoc.parse<0>((char *)content.c_str());
    //////////////////////////////////////////////////////////////////////////

    // support for unlimited layer class inherits
    rapidxml::xml_node<> *root = xDoc.first_node();

    rapidxml::xml_node<> *pRropertyRootNode = root->first_node("Propertys");
    if (pRropertyRootNode) {
        AddProperties(pRropertyRootNode, pClass);
    }

    //////////////////////////////////////////////////////////////////////////
    // and record
    rapidxml::xml_node<> *pRecordRootNode = root->first_node("Records");
    if (pRecordRootNode) {
        AddRecords(pRecordRootNode, pClass);
    }

    rapidxml::xml_node<> *pComponentRootNode = root->first_node("Components");
    if (pComponentRootNode) {
        AddComponents(pComponentRootNode, pClass);
    }

    // pClass->mvIncludeFile.push_back( pstrClassFilePath );
    // and include file
    rapidxml::xml_node<> *pIncludeRootNode = root->first_node("Includes");
    if (pIncludeRootNode) {
        for (rapidxml::xml_node<> *includeNode = pIncludeRootNode->first_node(); includeNode; includeNode = includeNode->next_sibling()) {
            const char *pstrIncludeFile = includeNode->first_attribute("Id")->value();
            // std::vector<std::string>::iterator it = std::find( pClass->mvIncludeFile.begin(), pClass->mvIncludeFile..end(), pstrIncludeFile );

            if (AddClassInclude(pstrIncludeFile, pClass)) {
                pClass->Add(pstrIncludeFile);
            }
        }
    }

    return true;
}

bool ClassModule::AddClass(const char *pstrClassFilePath, std::shared_ptr<IClass> pClass) {
    std::shared_ptr<IClass> pParent = pClass->GetParent();
    while (pParent) {
        // inherited some properties form class of parent
        std::string fileName = "";
        pParent->First(fileName);
        while (!fileName.empty()) {
            if (pClass->Find(fileName)) {
                fileName.clear();
                continue;
            }

            if (AddClassInclude(fileName.c_str(), pClass)) {
                pClass->Add(fileName);
            }

            fileName.clear();
            pParent->Next(fileName);
        }

        pParent = pParent->GetParent();
    }

    //////////////////////////////////////////////////////////////////////////
    if (AddClassInclude(pstrClassFilePath, pClass)) {
        pClass->Add(pstrClassFilePath);
    }

    // file.close();

    return true;
}

bool ClassModule::AddClass(const std::string &className, const std::string &strParentName) {
    std::shared_ptr<IClass> pParentClass = GetElement(strParentName);
    std::shared_ptr<IClass> pChildClass = GetElement(className);
    if (!pChildClass) {
        pChildClass = std::shared_ptr<IClass>(new Class(className));
        AddElement(className, pChildClass);
        // pChildClass = CreateElement( className );

        pChildClass->SetTypeName("");
        pChildClass->SetInstancePath("");

        if (pParentClass) {
            pChildClass->SetParent(pParentClass);
        }
    }

    return true;
}

bool ClassModule::Load(rapidxml::xml_node<> *attrNode, std::shared_ptr<IClass> pParentClass) {
    const char *pstrLogicClassName = attrNode->first_attribute("Id")->value();
    const char *pstrPath = attrNode->first_attribute("Path")->value();
    const char *pstrInstancePath = attrNode->first_attribute("InstancePath")->value();

    std::string realPath = pm_->GetWorkPath() + "/res/XlsxXML/";
    realPath += pstrPath;

    std::string realInstancePath = pm_->GetWorkPath() + "/res/XlsxXML/";
    realInstancePath += pstrInstancePath;

    // printf( "-----------------------------------------------------\n");
    // printf( "%s:\n", pstrLogicClassName );

    std::shared_ptr<IClass> pClass(new Class(pstrLogicClassName));
    AddElement(pstrLogicClassName, pClass);
    pClass->SetParent(pParentClass);
    pClass->SetInstancePath(realInstancePath);

    AddClass(realPath.c_str(), pClass);

    for (rapidxml::xml_node<> *pDataNode = attrNode->first_node(); pDataNode; pDataNode = pDataNode->next_sibling()) {
        // her children
        Load(pDataNode, pClass);
    }
    // printf( "-----------------------------------------------------\n");
    return true;
}

bool ClassModule::Load() {
    //////////////////////////////////////////////////////////////////////////
    std::string strFile = pm_->GetWorkPath() + "/" + mConfigFileName;
    std::string content;
    pm_->GetFileContent(strFile, content);

    rapidxml::xml_document<> xDoc;
    xDoc.parse<0>((char *)content.c_str());
    //////////////////////////////////////////////////////////////////////////
    // support for unlimited layer class inherits
    rapidxml::xml_node<> *root = xDoc.first_node();
    for (rapidxml::xml_node<> *attrNode = root->first_node(); attrNode; attrNode = attrNode->next_sibling()) {
        Load(attrNode, NULL);
    }

    for (int i = 0; i < mThreadClasses.size(); ++i) {
        mThreadClasses[i].classModule->Load();
    }

    return true;
}

bool ClassModule::Save() { return true; }

std::shared_ptr<IPropertyManager> ClassModule::GetClassPropertyManager(const std::string &className) {
    std::shared_ptr<IClass> pClass = GetElement(className);
    if (pClass) {
        return pClass->GetPropertyManager();
    }

    return NULL;
}

std::shared_ptr<IRecordManager> ClassModule::GetClassRecordManager(const std::string &className) {
    std::shared_ptr<IClass> pClass = GetElement(className);
    if (pClass) {
        return pClass->GetRecordManager();
    }

    return NULL;
}

bool ClassModule::Clear() { return true; }

bool ClassModule::AddClassCallBack(const std::string &className, const CLASS_EVENT_FUNCTOR_PTR &cb) {
    std::shared_ptr<IClass> pClass = GetElement(className);
    if (nullptr == pClass) {
        return false;
    }

    return pClass->AddClassCallBack(cb);
}

bool ClassModule::DoEvent(const Guid &objectID, const std::string &className, const CLASS_OBJECT_EVENT classEvent, const DataList &valueList) {
    std::shared_ptr<IClass> pClass = GetElement(className);
    if (nullptr == pClass) {
        return false;
    }

    return pClass->DoEvent(objectID, classEvent, valueList);
}

bool ClassModule::AfterStart() { return true; }

IClassModule *ClassModule::GetThreadClassModule(const int index) { return mThreadClasses[index].classModule; }
