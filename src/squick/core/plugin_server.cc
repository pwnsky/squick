

#include "plugin_server.h"
#include "squick/core/exception.h"

PluginServer::PluginServer(const std::string& strArgv)
{
    this->strArgvList = strArgv;
    
#if SQUICK_PLATFORM != SQUICK_PLATFORM_WIN
	SQUICK_CRASH_TRY_ROOT
#endif  
}

void PluginServer::Update()
{
	pPluginManager->Update();
}

void PluginServer::SetBasicWareLoader(std::function<void(IPluginManager * p)> fun)
{
	externalBasicWarePluginLoader = fun;
}

void PluginServer::SetMidWareLoader(std::function<void(IPluginManager * p)> fun)
{
	externalMidWarePluginLoader = fun;
}

void PluginServer::Start()
{

    pPluginManager = SQUICK_SHARE_PTR<IPluginManager>(SQUICK_NEW PluginManager());

    ProcessParameter();

    pPluginManager->SetGetFileContentFunctor(GetFileContent);
    pPluginManager->SetConfigPath("../");


	if (externalBasicWarePluginLoader)
	{
		externalBasicWarePluginLoader(pPluginManager.get());
	}


    if (externalMidWarePluginLoader)
    {
        externalMidWarePluginLoader(pPluginManager.get());
    }

	pPluginManager->LoadPluginConfig();
    pPluginManager->LoadPlugin();

    pPluginManager->Awake();
    pPluginManager->Start();
    pPluginManager->AfterStart();
    pPluginManager->CheckConfig();
    pPluginManager->ReadyUpdate();
}

void PluginServer::Final()
{
    pPluginManager->BeforeDestory();
    pPluginManager->Destory();
    pPluginManager->Finalize();

    pPluginManager = nullptr;
}

void PluginServer::ProcessParameter()
{
    //run it as a daemon process
    if (strArgvList.find("-d") != string::npos)
    {
        StartDaemon();
    }

#if SQUICK_PLATFORM != SQUICK_PLATFORM_WIN
    //run it as a daemon process
    if (strArgvList.find("-d") != string::npos)
    {
        StartDaemon();
    }
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
#endif


    std::vector<std::string> argList;
	std::string token;
	std::istringstream tokenStream(this->strArgvList);
	while (std::getline(tokenStream, token, ' '))
	{
		argList.push_back(token);
	}

    pPluginManager->SetConfigName(FindParameterValue(argList, "plugin="));
	pPluginManager->SetAppName(FindParameterValue(argList, "server="));

	std::string strAppID = FindParameterValue(argList, "id=");
    int appID = 0;
    if (SQUICK_StrTo(strAppID, appID))
    {
        pPluginManager->SetAppID(appID);
    }

	std::string strDockerFlag = FindParameterValue(argList, "docker=");
	int nDockerFlag = 0;
	if (SQUICK_StrTo(strDockerFlag, nDockerFlag))
	{
		pPluginManager->SetRunningDocker(nDockerFlag);
	}

    // NoSqlServer.xml:IP=\"127.0.0.1\"==IP=\"192.168.1.1\"
    if (strArgvList.find(".xml:") != string::npos)
    {
        for (int i = 0; i < argList.size(); i++)
        {
            std::string strPipeline = argList[i];
            size_t posFile = strPipeline.find(".xml:");
            size_t posContent = strPipeline.find("==");
            if (posFile != string::npos && posContent != string::npos)
            {
                std::string fileName = strPipeline.substr(0, posFile + 4);
                std::string content = strPipeline.substr(posFile + 5, posContent - (posFile + 5));
                std::string replaceContent = strPipeline.substr(posContent + 2, strPipeline.length() - (posContent + 2));

                pPluginManager->AddFileReplaceContent(fileName, content, replaceContent);
            }
        }
    }

    std::string strTitleName = pPluginManager->GetAppName() + std::to_string(pPluginManager->GetAppID());// +" PID" + NFGetPID();
    if (!strTitleName.empty())
    {
        size_t pos = strTitleName.find("Server");
		if (pos != string::npos)
		{
			strTitleName.replace(pos, 6, "");
			strTitleName = "Sqcuik" + strTitleName;
		}
    }
    else
    {
        strTitleName = "SqcuikIDE";
    }


#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN
    SetConsoleTitle(strTitleName.c_str());
#elif SQUICK_PLATFORM == SQUICK_PLATFORM_LINUX
    prctl(PR_SET_NAME, strTitleName.c_str());
    //setproctitle(strTitleName.c_str());
#endif

}

void PluginServer::StartDaemon()
{

#if SQUICK_PLATFORM != SQUICK_PLATFORM_WIN
    daemon(1, 0);
    // ignore signals
    signal(SIGINT, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
#endif
}

bool PluginServer::GetFileContent(IPluginManager* p, const std::string& strFilePath, std::string& content)
{
    FILE* fp = fopen(strFilePath.c_str(), "rb");
    if (!fp)
    {
        return false;
    }

    fseek(fp, 0, SEEK_END);
    const long filelength = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    content.resize(filelength);
    fread((void*)content.data(), filelength, 1, fp);
    fclose(fp);

    std::string fileName = strFilePath.substr(strFilePath.find_last_of("/\\") + 1);
    std::vector<ReplaceContent> contents = p->GetFileReplaceContents(fileName);
    if (!contents.empty())
    {
        for (auto it : contents)
        {
            std::size_t pos = content.find(it.content);
            if (pos != string::npos)
            {
                content.replace(pos, it.content.length(), it.newValue.c_str());
            }
        }
    }

    return true;
}

std::string PluginServer::FindParameterValue(const std::vector<std::string>& argList, const std::string& header)
{
	for (int i = 0; i < argList.size(); i++)
	{
		std::string name = argList[i];
		if (name.find(header) != string::npos)
		{
			name.erase(0, header.length());
			return name;
		}

	}

	return "";
}
