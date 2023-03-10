#include "http_client_module.h"
#include "http_client.h"

#define DEFAULT_USER_AGENT "Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/59.0.3071.115 Mobile Safari/537.36"

HttpClientModule::HttpClientModule(IPluginManager* p)
{
    m_bIsUpdate = true;
    pPluginManager = p;
    m_pHttpClient = new HttpClient();
    m_xDefaultHttpHeaders["Connection"] = "close";
    //evhttp_add_header(output_headers, "Connection", "keep-alive");
	m_xDefaultHttpHeaders["Content-Type"] = "text/plain;text/html;application/x-www-form-urlencoded;charset=utf-8";
	//m_xDefaultHttpHeaders["Content-Type"] = "text/plain;text/html;application/x-www-form-urlencoded;charset=utf-8";
    m_xDefaultHttpHeaders["User-Agent"] = DEFAULT_USER_AGENT;
    m_xDefaultHttpHeaders["Cache-Control"] = "no-cache";
}

HttpClientModule::~HttpClientModule()
{
    delete m_pHttpClient;
    m_pHttpClient = NULL;
}

bool HttpClientModule::Start()
{
    m_pHttpClient->Start();

    return true;
}

bool HttpClientModule::AfterStart()
{
	m_pKernelModule = pPluginManager->FindModule<IKernelModule>();
	return true;
}

bool HttpClientModule::Update()
{
    m_pHttpClient->Update();//TODO
    return true;
}

bool HttpClientModule::Destory()
{
    m_pHttpClient->Final();

    return true;
}

int HttpClientModule::Post(const std::string & strUri, const std::string & strData, std::string & strResData)
{
	return Post(strUri, m_xDefaultHttpHeaders, strData, strResData);
}

int HttpClientModule::Post(const std::string & strUri, const std::map<std::string, std::string>& xHeaders, const std::string & strData, std::string & strResData)
{
	HTTP_RESP_FUNCTOR_PTR pd(new HTTP_RESP_FUNCTOR(std::bind(&HttpClientModule::CallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

	Guid id = m_pKernelModule->CreateGUID();
    std::string memo;
    m_pHttpClient->DoPost(strUri, strData,memo, pd, xHeaders, id);

	mxRespDataMap.AddElement(id, SQUICK_SHARE_PTR<RespData>(SQUICK_NEW RespData()));

	SQUICK_SHARE_PTR<RespData> xRespData = mxRespDataMap.GetElement(id);
	while (!xRespData->resp)
	{
        NFSLEEP(1);
	}

	strResData = xRespData->strRespData;

	return xRespData->state_code;
}

int HttpClientModule::Get(const std::string & strUri, std::string & strResData)
{
	return Get(strUri, m_xDefaultHttpHeaders, strResData);
}

int HttpClientModule::Get(const std::string & strUri, const std::map<std::string, std::string>& xHeaders, std::string & strResData)
{
	HTTP_RESP_FUNCTOR_PTR pd(new HTTP_RESP_FUNCTOR(std::bind(&HttpClientModule::CallBack, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

	Guid id = m_pKernelModule->CreateGUID();
	m_pHttpClient->DoGet(strUri, pd, m_xDefaultHttpHeaders, id);

	mxRespDataMap.AddElement(id, SQUICK_SHARE_PTR<RespData>(SQUICK_NEW RespData()));

	SQUICK_SHARE_PTR<RespData> xRespData = mxRespDataMap.GetElement(id);
	while (!xRespData->resp)
	{
        NFSLEEP(1);
	}

	strResData = xRespData->strRespData;
	return xRespData->state_code;
}

bool HttpClientModule::DoGet(const std::string& strUri,
                                     const std::map<std::string, std::string>& xHeaders,
                                     HTTP_RESP_FUNCTOR_PTR pCB)
{
    return m_pHttpClient->DoGet(strUri, pCB, xHeaders.size() == 0 ? m_xDefaultHttpHeaders : xHeaders);
}

bool HttpClientModule::DoPost(const std::string& strUri,
                                      const std::map<std::string, std::string>& xHeaders,
                                      const std::string& strPostData,
                                      HTTP_RESP_FUNCTOR_PTR pCB, const std::string& strMemo)
{
    Guid aid = m_pKernelModule->CreateGUID();

    return m_pHttpClient->DoPost(strUri, strPostData, strMemo, pCB,
        xHeaders.size() == 0 ? m_xDefaultHttpHeaders : xHeaders, aid);

}

void HttpClientModule::CallBack(const Guid id, const int state_code, const std::string & strRespData)
{
	SQUICK_SHARE_PTR<RespData> xRespData = mxRespDataMap.GetElement(id);
	if (xRespData)
	{
		xRespData->resp = true;
		xRespData->state_code = state_code;
		xRespData->strRespData = strRespData;
	}
}
