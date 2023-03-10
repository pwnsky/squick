
#include <algorithm>
#include <ctype.h>
//#include "plugin.h"
#include "element_module.h"
#include "class_module.h"
#include <squick/plugin/kernel/i_thread_pool_module.h>

ElementModule::ElementModule(ElementModule* p)
{
    mbLoaded = false;
	originalElementModule = p;
}

ElementModule::ElementModule(IPluginManager* p)
{
	originalElementModule = this;
    pPluginManager = p;
    mbLoaded = false;

	if (!this->mbBackup)
	{
		for (int i = 0; i < pPluginManager->GetAppCPUCount(); ++i)
		{
			ThreadElementModule threadElement;
			threadElement.used = false;
			threadElement.elementModule = new ElementModule(this);
			threadElement.elementModule->mbBackup = true;
			threadElement.elementModule->pPluginManager = pPluginManager;

			mThreadElements.push_back(threadElement);
		}
	}
}

ElementModule::~ElementModule()
{
    if (!this->mbBackup)
    {
		for (int i = 0; i < mThreadElements.size(); ++i)
		{
			delete mThreadElements[i].elementModule;
		}

		mThreadElements.clear();
    }
}

bool ElementModule::Awake()
{
	m_pClassModule = pPluginManager->FindModule<IClassModule>();
	m_pLogModule = pPluginManager->FindModule<ILogModule>();

	if (this->mbBackup)
	{
		for (int i = 0; i < originalElementModule->mThreadElements.size(); ++i)
		{
			if (originalElementModule->mThreadElements[i].elementModule == this)
			{
				m_pClassModule = m_pClassModule->GetThreadClassModule(i);
				break;
			}
		}
	}

	for (int i = 0; i < mThreadElements.size(); ++i)
	{
		mThreadElements[i].elementModule->Awake();
	}

	Load();

	return true;
}

bool ElementModule::Start()
{
	for (int i = 0; i < mThreadElements.size(); ++i)
	{
		mThreadElements[i].elementModule->Start();
	}

    return true;
}

bool ElementModule::AfterStart()
{
	CheckRef();

	for (int i = 0; i < mThreadElements.size(); ++i)
	{
		mThreadElements[i].elementModule->AfterStart();
	}

	return true;
}

bool ElementModule::Destory()
{
    Clear();

    return true;
}

IElementModule* ElementModule::GetThreadElementModule()
{
	std::thread::id threadID = std::this_thread::get_id();

	for (int i = 0; i < mThreadElements.size(); ++i)
	{
		if (mThreadElements[i].used)
		{
			if (mThreadElements[i].threadID == threadID)
			{
				return mThreadElements[i].elementModule;
			}
		}
		else
		{
			mThreadElements[i].used = true;
			mThreadElements[i].threadID = threadID;
			return mThreadElements[i].elementModule;
		}
	}

	return nullptr;
}

// 加载配置文件
bool ElementModule::Load()
{
    if (mbLoaded)
    {
        return false;
    }

    SQUICK_SHARE_PTR<IClass> pLogicClass = m_pClassModule->First();
    while (pLogicClass)
    {
        const std::string& strInstancePath = pLogicClass->GetInstancePath();
        if (strInstancePath.empty())
        {
            pLogicClass = m_pClassModule->Next();
            continue;
        }
        //////////////////////////////////////////////////////////////////////////
		std::string strFile = pPluginManager->GetConfigPath() + "/" + strInstancePath;
		std::string content;
		pPluginManager->GetFileContent(strFile, content);
#ifdef DEBUG
        std::cout << "Loading config file: " <<  strFile << std::endl;
#endif
		rapidxml::xml_document<> xDoc;
		
        
        //////////////////////////////////////////////////////////////////////////
        //support for unlimited layer class inherits
        rapidxml::xml_node<>* root = nullptr;
        try {
            xDoc.parse<0>((char*)content.c_str());
            root = xDoc.first_node();
            if(nullptr == root) {
                std::cout << "Config Load Error, Nullptr" << SQUICK_DEBUG_INFO << std::endl;
                continue;
            }
        }catch (const std::runtime_error& e)
        {
            std::cerr << "Runtime error was: " << e.what() << std::endl;
            continue;
        }
        catch (const rapidxml::parse_error& e)
        {
            std::cerr << "Parse error was: " << e.what() << std::endl;
             continue;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error was: " << e.what() << std::endl;
             continue;
        }
        catch (...)
        {
            std::cerr << "An unknown error occurred." << std::endl;
            continue;
        }
        for (rapidxml::xml_node<>* attrNode = root->first_node(); attrNode; attrNode = attrNode->next_sibling())
        {
            Load(attrNode, pLogicClass);
        }

        mbLoaded = true;
        pLogicClass = m_pClassModule->Next();
    }
    
	for (int i = 0; i < mThreadElements.size(); ++i)
	{
		mThreadElements[i].elementModule->Load();
	}
    return true;
}

bool ElementModule::CheckRef()
{
    SQUICK_SHARE_PTR<IClass> pLogicClass = m_pClassModule->First();
    while (pLogicClass)
    {
		SQUICK_SHARE_PTR<IPropertyManager> pClassPropertyManager = pLogicClass->GetPropertyManager();
		if (pClassPropertyManager)
		{
			SQUICK_SHARE_PTR<IProperty> pProperty = pClassPropertyManager->First();
			while (pProperty)
			{
				//if one property is ref,check every config
				if (pProperty->GetRef())
				{
					const std::vector<std::string>& strIdList = pLogicClass->GetIDList();
					for (int i = 0; i < strIdList.size(); ++i)
					{
						const std::string& strId = strIdList[i];

						const std::string& strRefValue= this->GetPropertyString(strId, pProperty->GetKey());
						if (!strRefValue.empty() && !this->GetElement(strRefValue))
						{
							std::string msg = "check ref failed id:" + strRefValue + ", in " + pLogicClass->GetClassName() + "=>" + strId;
							NFASSERT(nRet, msg.c_str(), __FILE__, __FUNCTION__);

							m_pLogModule->LogError(msg, __FUNCTION__, __LINE__);
							exit(0);
						}
					}
				}
				pProperty = pClassPropertyManager->Next();
			}
		}
        //////////////////////////////////////////////////////////////////////////
        pLogicClass = m_pClassModule->Next();
    }

    return false;
}

// 加载属性
bool ElementModule::Load(rapidxml::xml_node<>* attrNode, SQUICK_SHARE_PTR<IClass> pLogicClass)
{
    //attrNode is the node of a object
    std::string configID = attrNode->first_attribute("Id")->value();
    if (configID.empty())
    {
        NFASSERT(0, configID, __FILE__, __FUNCTION__);
        return false;
    }

    if (ExistElement(configID))
    {
        NFASSERT(0, configID, __FILE__, __FUNCTION__);
        return false;
    }

    SQUICK_SHARE_PTR<ElementConfigInfo> pElementInfo(SQUICK_NEW ElementConfigInfo());
    AddElement(configID, pElementInfo);

    //can find all configid by class name
    pLogicClass->AddId(configID);

    //ElementConfigInfo* pElementInfo = CreateElement( configID, pElementInfo );
    SQUICK_SHARE_PTR<IPropertyManager> pElementPropertyManager = pElementInfo->GetPropertyManager();
    SQUICK_SHARE_PTR<IRecordManager> pElementRecordManager = pElementInfo->GetRecordManager();

    //1.add property
    //2.set the default value  of them
    SQUICK_SHARE_PTR<IPropertyManager> pClassPropertyManager = pLogicClass->GetPropertyManager();
    SQUICK_SHARE_PTR<IRecordManager> pClassRecordManager = pLogicClass->GetRecordManager();
    if (pClassPropertyManager && pClassRecordManager)
    {
        SQUICK_SHARE_PTR<IProperty> pProperty = pClassPropertyManager->First();
        while (pProperty)
        {

            pElementPropertyManager->AddProperty(Guid(), pProperty);

            pProperty = pClassPropertyManager->Next();
        }

        SQUICK_SHARE_PTR<IRecord> pRecord = pClassRecordManager->First();
        while (pRecord)
        {
            SQUICK_SHARE_PTR<IRecord> xRecord = pElementRecordManager->AddRecord(Guid(), pRecord->GetName(), pRecord->GetStartData(), pRecord->GetTag(), pRecord->GetRows());

            xRecord->SetPublic(pRecord->GetPublic());
            xRecord->SetPrivate(pRecord->GetPrivate());
            xRecord->SetSave(pRecord->GetSave());
            xRecord->SetCache(pRecord->GetCache());
			xRecord->SetRef(pRecord->GetRef());
			xRecord->SetForce(pRecord->GetForce());
			xRecord->SetUpload(pRecord->GetUpload());

            pRecord = pClassRecordManager->Next();
        }

    }

    //3.set the config value to them

    //const char* pstrConfigID = attrNode->first_attribute( "ID" );
    for (rapidxml::xml_attribute<>* pAttribute = attrNode->first_attribute(); pAttribute; pAttribute = pAttribute->next_attribute())
    {
        const char* pstrConfigName = pAttribute->name();
        const char* pstrConfigValue = pAttribute->value();
        //printf( "%s : %s\n", pstrConfigName, pstrConfigValue );

        SQUICK_SHARE_PTR<IProperty> temProperty = pElementPropertyManager->GetElement(pstrConfigName);
        if (!temProperty)
        {
            continue;
        }

        SquickData var;
        const DATA_TYPE eType = temProperty->GetType();
        switch (eType)
        {
            case TDATA_INT:
            {
                if (!LegalNumber(pstrConfigValue))
                {
                    NFASSERT(0, temProperty->GetKey(), __FILE__, __FUNCTION__);
                }
                var.SetInt(lexical_cast<INT64>(pstrConfigValue));
            }
            break;
            case TDATA_FLOAT:
            {
                if (strlen(pstrConfigValue) <= 0 || !LegalFloat(pstrConfigValue))
                {
                    NFASSERT(0, temProperty->GetKey(), __FILE__, __FUNCTION__);
                }
                var.SetFloat((double)atof(pstrConfigValue));
            }
            break;
            case TDATA_STRING:
                {
                    var.SetString(pstrConfigValue);
                }
                break;
            case TDATA_OBJECT:
            {
                if (strlen(pstrConfigValue) <= 0)
                {
                    NFASSERT(0, temProperty->GetKey(), __FILE__, __FUNCTION__);
                }
                var.SetObject(Guid());
            }
            break;
			case TDATA_VECTOR2:
			{
				if (strlen(pstrConfigValue) <= 0)
				{
					NFASSERT(0, temProperty->GetKey(), __FILE__, __FUNCTION__);
				}

				Vector2 tmp;
				if (!tmp.FromString(pstrConfigValue))
				{
					NFASSERT(0, temProperty->GetKey(), __FILE__, __FUNCTION__);
				}
				var.SetVector2(tmp);
			}
			break;
			case TDATA_VECTOR3:
			{
				if (strlen(pstrConfigValue) <= 0)
				{
					NFASSERT(0, temProperty->GetKey(), __FILE__, __FUNCTION__);
				}
				Vector3 tmp;
				if (!tmp.FromString(pstrConfigValue))
				{
					NFASSERT(0, temProperty->GetKey(), __FILE__, __FUNCTION__);
				}
				var.SetVector3(tmp);
			}
			break;
            default:
                NFASSERT(0, temProperty->GetKey(), __FILE__, __FUNCTION__);
                break;
        }

        temProperty->SetValue(var);
        if (eType == TDATA_STRING)
        {
            temProperty->DeSerialization();
        }
    }

    SquickData xDataClassName;
    xDataClassName.SetString(pLogicClass->GetClassName());
    pElementPropertyManager->SetProperty("ClassName", xDataClassName);


    SquickData xDataID;
    xDataID.SetString(configID);
    pElementPropertyManager->SetProperty("ID", xDataID);
    pElementPropertyManager->SetProperty("ConfigID", xDataID);

    return true;
}

bool ElementModule::Save()
{
    return true;
}

INT64 ElementModule::GetPropertyInt(const std::string& configName, const std::string& propertyName)
{
    SQUICK_SHARE_PTR<IProperty> pProperty = GetProperty(configName, propertyName);
    if (pProperty)
    {
        return pProperty->GetInt();
    }

    return 0;
}

int ElementModule::GetPropertyInt32(const std::string& configName, const std::string& propertyName)
{
	SQUICK_SHARE_PTR<IProperty> pProperty = GetProperty(configName, propertyName);
	if (pProperty)
	{
		return pProperty->GetInt32();
	}

	return 0;
}

double ElementModule::GetPropertyFloat(const std::string& configName, const std::string& propertyName)
{
    SQUICK_SHARE_PTR<IProperty> pProperty = GetProperty(configName, propertyName);
    if (pProperty)
    {
        return pProperty->GetFloat();
    }

    return 0.0;
}

const std::string& ElementModule::GetPropertyString(const std::string& configName, const std::string& propertyName)
{
    SQUICK_SHARE_PTR<IProperty> pProperty = GetProperty(configName, propertyName);
    if (pProperty)
    {
        return pProperty->GetString();
    }

    return  NULL_STR;
}

const Vector2 ElementModule::GetPropertyVector2(const std::string & configName, const std::string & propertyName)
{
	SQUICK_SHARE_PTR<IProperty> pProperty = GetProperty(configName, propertyName);
	if (pProperty)
	{
		return pProperty->GetVector2();
	}

	return Vector2();
}

const Vector3 ElementModule::GetPropertyVector3(const std::string & configName, const std::string & propertyName)
{
	SQUICK_SHARE_PTR<IProperty> pProperty = GetProperty(configName, propertyName);
	if (pProperty)
	{
		return pProperty->GetVector3();
	}

	return Vector3();
}

const std::vector<std::string> ElementModule::GetListByProperty(const std::string & className, const std::string & propertyName, INT64 nValue)
{
	std::vector<std::string> xList;

	SQUICK_SHARE_PTR<IClass> xClass = m_pClassModule->GetElement(className);
	if (nullptr != xClass)
	{
		const std::vector<std::string>& xElementList = xClass->GetIDList();
		for (int i = 0; i < xElementList.size(); ++i)
		{
			const std::string& configID = xElementList[i];
			INT64 nElementValue = GetPropertyInt(configID, propertyName);
			if (nValue == nElementValue)
			{
				xList.push_back(configID);
			}
		}
	}

	return xList;
}

const std::vector<std::string> ElementModule::GetListByProperty(const std::string & className, const std::string & propertyName, const std::string & nValue)
{
	std::vector<std::string> xList;

	SQUICK_SHARE_PTR<IClass> xClass = m_pClassModule->GetElement(className);
	if (nullptr != xClass)
	{
		const std::vector<std::string>& xElementList = xClass->GetIDList();
		for (int i = 0; i < xElementList.size(); ++i)
		{
			const std::string& configID = xElementList[i];
			const std::string& strElementValue = GetPropertyString(configID, propertyName);
			if (nValue == strElementValue)
			{
				xList.push_back(configID);
			}
		}
	}

	return xList;
}

SQUICK_SHARE_PTR<IProperty> ElementModule::GetProperty(const std::string& configName, const std::string& propertyName)
{
    SQUICK_SHARE_PTR<ElementConfigInfo> pElementInfo = GetElement(configName);
    if (pElementInfo)
    {
        return pElementInfo->GetPropertyManager()->GetElement(propertyName);
    }

    return NULL;
}

SQUICK_SHARE_PTR<IPropertyManager> ElementModule::GetPropertyManager(const std::string& configName)
{
    SQUICK_SHARE_PTR<ElementConfigInfo> pElementInfo = GetElement(configName);
    if (pElementInfo)
    {
        return pElementInfo->GetPropertyManager();
    }

    return NULL;
}

SQUICK_SHARE_PTR<IRecordManager> ElementModule::GetRecordManager(const std::string& configName)
{
    SQUICK_SHARE_PTR<ElementConfigInfo> pElementInfo = GetElement(configName);
    if (pElementInfo)
    {
        return pElementInfo->GetRecordManager();
    }
    return NULL;
}

bool ElementModule::LoadSceneInfo(const std::string& fileName, const std::string& className)
{
	std::string content;
	pPluginManager->GetFileContent(fileName, content);

	rapidxml::xml_document<> xDoc;
	xDoc.parse<0>((char*)content.c_str());
	
    SQUICK_SHARE_PTR<IClass> pLogicClass = m_pClassModule->GetElement(className.c_str());
    if (pLogicClass)
    {
        //support for unlimited layer class inherits
        rapidxml::xml_node<>* root = xDoc.first_node();
        for (rapidxml::xml_node<>* attrNode = root->first_node(); attrNode; attrNode = attrNode->next_sibling())
        {
            Load(attrNode, pLogicClass);
        }
    }
    else
    {
        std::cout << "error load scene info failed, name is:" << className << " file name is :" << fileName << std::endl;
    }

    return true;
}

bool ElementModule::ExistElement(const std::string& configName)
{
    SQUICK_SHARE_PTR<ElementConfigInfo> pElementInfo = GetElement(configName);
    if (pElementInfo)
    {
        return true;
    }

    return false;
}

bool ElementModule::ExistElement(const std::string& className, const std::string& configName)
{
    SQUICK_SHARE_PTR<ElementConfigInfo> pElementInfo = GetElement(configName);
    if (!pElementInfo)
    {
        return false;
    }

    const std::string& strClass = pElementInfo->GetPropertyManager()->GetPropertyString("ClassName");
    if (strClass != className)
    {
        return ExistElement(configName);
    }

    return true;
}

bool ElementModule::LegalNumber(const char* str)
{
    int len = int(strlen(str));
    if (len <= 0)
    {
        return false;
    }

    int nStart = 0;
    if ('-' == str[0])
    {
        nStart = 1;
    }

    for (int i = nStart; i < len; ++i)
    {
        if (!isdigit(str[i]))
        {
            return false;
        }
    }

    return true;
}

bool ElementModule::LegalFloat(const char * str)
{

	int len = int(strlen(str));
	if (len <= 0)
	{
		return false;
	}

	int nStart = 0;
	int nEnd = len;
	if ('-' == str[0])
	{
		nStart = 1;
	}
	if ('f' == std::tolower(str[nEnd -1]))
	{
		nEnd--;
	}

	if (nEnd <= nStart)
	{
		return false;
	}

	int pointNum = 0;
	for (int i = nStart; i < nEnd; ++i)
	{
		if ('.' == str[i])
		{
			pointNum++;
		}

		if (!isdigit(str[i]) && '.' != str[i])
		{
			return false;
		}
	}

	if (pointNum > 1)
	{
		return false;
	}

	return true;
}

bool ElementModule::BeforeDestory()
{
    return true;

}

bool ElementModule::Update()
{
    return true;

}

bool ElementModule::Clear()
{
    ClearAll();

    mbLoaded = false;
    return true;
}