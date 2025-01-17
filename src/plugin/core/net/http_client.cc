
#include "http_client.h"
#include "net_module.h"

#include <openssl/err.h>
#include <openssl/ssl.h>

#if PLATFORM == PLATFORM_WIN
#define snprintf _snprintf
#endif

bool HttpClient::Update() {
    if (m_pBase) {
        event_base_loop(m_pBase, EVLOOP_ONCE | EVLOOP_NONBLOCK);
    }

    return true;
}

bool HttpClient::Start() {
    for (int i = 0; i < 1024; ++i) {
        mlHttpObject.push_back(new HttpObject(this, nullptr, nullptr, Guid()));
    }

#if PLATFORM == PLATFORM_WIN
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        return false;
    }
#endif

#if SQUICK_ENABLE_SSL

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    // Startialize OpenSSL
    SSL_library_init();
    ERR_load_crypto_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
#endif

    m_pSslCtx = SSL_CTX_new(SSLv23_client_method());
    if (!m_pSslCtx) {
        // LOG(ERROR) << "SSL_CTX_new err. " << __FUNCTION__ << " " << __LINE__;
        return false;
    }

#endif

    m_pBase = event_base_new();
    if (m_pBase == nullptr) {
        // LOG(ERROR) << "event_base_new err " << __FUNCTION__ << " " << __LINE__;
        return false;
    }
    return true;
}

bool HttpClient::Final() {
    if (m_pBase) {
        event_base_free(m_pBase);
        m_pBase = nullptr;
    }

#if SQUICK_ENABLE_SSL
    if (m_pSslCtx) {
        SSL_CTX_free(m_pSslCtx);
        m_pSslCtx = nullptr;
    }

#if (OPENSSL_VERSION_NUMBER < 0x10100000L) || defined(LIBRESSL_VERSION_NUMBER)
    EVP_cleanup();
    ERR_free_strings();

#if OPENSSL_VERSION_NUMBER < 0x10000000L
    ERR_remove_state(0);
#else
    ERR_remove_thread_state(NULL);
#endif

    CRYPTO_cleanup_all_ex_data();

    sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
#endif /* (OPENSSL_VERSION_NUMBER < 0x10100000L) || defined(LIBRESSL_VERSION_NUMBER) */
#endif
    return true;
}

bool HttpClient::MakeRequest(const std::string &strUri, HTTP_RESP_FUNCTOR_PTR pCB, const std::string &strPostData,
                             const std::map<std::string, std::string> &xHeaders, const HttpType eHttpType, const std::string &strMemoData, const Guid id) {
    struct evhttp_uri *http_uri = evhttp_uri_parse(strUri.c_str());
    if (http_uri == NULL) {
        // LOG(ERROR) << "evhttp_uri_parse err. " << __FUNCTION__ << " " << __LINE__;
        return false;
    }

    const char *scheme = evhttp_uri_get_scheme(http_uri);
    if (scheme == NULL) {
        // LOG(ERROR) << "scheme == NULL err. " << __FUNCTION__ << " " << __LINE__;
        return false;
    }

    std::string lowwerScheme(scheme);
    std::transform(lowwerScheme.begin(), lowwerScheme.end(), lowwerScheme.begin(), (int (*)(int))std::tolower);
    if (lowwerScheme.compare("https") != 0 && lowwerScheme.compare("http") != 0) {
        if (http_uri) {
            evhttp_uri_free(http_uri);
        }

        // LOG(ERROR) << "scheme == NULL err. " << __FUNCTION__ << " " << __LINE__;
        return false;
    }

    bool isHttps = false;
    if (lowwerScheme.compare("https") == 0) {
        isHttps = true;
    }

    const char *host = evhttp_uri_get_host(http_uri);
    if (host == NULL) {
        if (http_uri) {
            evhttp_uri_free(http_uri);
        }

        // LOG(ERROR) << "url must have a host err. " << __FUNCTION__ << " " << __LINE__;
        return false;
    }

    int port = evhttp_uri_get_port(http_uri);
    if (port == -1) {
        port = isHttps ? 443 : 80;
    }

    const char *path = evhttp_uri_get_path(http_uri);
    if (path == NULL) {
        // LOG(ERROR) << "path == NUL err. " << __FUNCTION__ << " " << __LINE__;
        return false;
    }

    if (strlen(path) == 0) {
        path = "/";
    }

    char uri[512] = {0};

    const char *query = evhttp_uri_get_query(http_uri);
    if (query == NULL) {
        snprintf(uri, sizeof(uri) - 1, "%s", path);
    } else {
        snprintf(uri, sizeof(uri) - 1, "%s?%s", path, query);
    }

    uri[sizeof(uri) - 1] = '\0';
    //-------we do not verify peer--------//
    // like the curl SSL_VERIFYPEER is set false

    // if (1 != SSL_CTX_load_verify_locations(ssl_ctx, crt, NULL)) {
    //	err_openssl("SSL_CTX_load_verify_locations");
    //	goto error;
    // }
    // SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, NULL);
    // SSL_CTX_set_cert_verify_callback(ssl_ctx, cert_verify_callback,(void *)host);

    struct bufferevent *bev = NULL;

#if SQUICK_ENABLE_SSL
    SSL *pSSL = SSL_new(m_pSslCtx);
    if (pSSL == NULL) {
        if (http_uri) {
            evhttp_uri_free(http_uri);
        }

        // LOG(ERROR) << "SSL_new err. " << __FUNCTION__ << " " << __LINE__;
        return false;
    }
#endif

    if (!isHttps) {
        bev = bufferevent_socket_new(m_pBase, -1, BEV_OPT_CLOSE_ON_FREE);
    } else {
#if SQUICK_ENABLE_SSL
        bev = bufferevent_openssl_socket_new(m_pBase, -1, pSSL, BUFFEREVENT_SSL_CONNECTING, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
#endif
    }

    if (bev == NULL) {
        if (http_uri) {
            evhttp_uri_free(http_uri);
        }

        // LOG(ERROR) << " bev == NUL err. " << __FUNCTION__ << " " << __LINE__;
        return false;
    }

#if SQUICK_ENABLE_SSL
    if (isHttps) {
        bufferevent_openssl_set_allow_dirty_shutdown(bev, 1);
    }
#endif

    struct evhttp_connection *evcon = evhttp_connection_base_bufferevent_new(m_pBase, NULL, bev, host, port);
    if (evcon == NULL) {
        if (http_uri) {
            evhttp_uri_free(http_uri);
        }

        // LOG(ERROR) << " evcon == NUL err. " << __FUNCTION__ << " " << __LINE__;
        return false;
    }

    if (m_nRetry > 0) {
        evhttp_connection_set_retries(evcon, m_nRetry);
    }
    if (m_nTimeOut >= 0) {
        evhttp_connection_set_timeout(evcon, m_nTimeOut);
    }

    HttpObject *pHttpObj = nullptr;
    if (mlHttpObject.size() > 0) {
        pHttpObj = mlHttpObject.front();
        mlHttpObject.pop_front();
        pHttpObj->m_pHttpClient = this;
        pHttpObj->m_pBev = bev;
        pHttpObj->m_pCB = pCB;
        pHttpObj->mID = id;
        pHttpObj->strMemo = strMemoData;
    } else {
        pHttpObj = new HttpObject(this, bev, pCB, id, strMemoData);
    }

    if (pHttpObj == nullptr) {
        // LOG(ERROR) << "pHttpObj == nullptr err. " << __FUNCTION__ << " " << __LINE__;
        return false;
    }

    // Fire off the request
    struct evhttp_request *req = evhttp_request_new(OnHttpReqDone, pHttpObj);
    if (req == NULL) {
        if (http_uri) {
            evhttp_uri_free(http_uri);
        }

        // LOG(ERROR) << "req == NULL err. " << __FUNCTION__ << " " << __LINE__;
        return false;
    }

    struct evkeyvalq *output_headers = evhttp_request_get_output_headers(req);
    if (output_headers == NULL) {
        // LOG(ERROR) << "output_headers == NULL err. " << __FUNCTION__ << " " << __LINE__;
        return false;
    }

    evhttp_add_header(output_headers, "Host", host);
    evhttp_add_header(output_headers, "Connection", "close");
    // evhttp_add_header(output_headers, "Connection", "keep-alive");

    std::map<std::string, std::string>::const_iterator it = xHeaders.cbegin();
    while (it != xHeaders.cend()) {
        evhttp_add_header(output_headers, it->first.c_str(), it->second.c_str());
        it++;
    }

    size_t len = strPostData.length();
    if (len > 0) {
        struct evbuffer *output_buffer = evhttp_request_get_output_buffer(req);
        if (output_buffer == NULL) {
            // LOG(ERROR) << "output_buffer == NUL err. " << __FUNCTION__ << " " << __LINE__;
            return false;
        }

        evbuffer_add(output_buffer, strPostData.c_str(), len);
        char buf[256] = {0};
        evutil_snprintf(buf, sizeof(buf) - 1, "%lu", (unsigned long)len);
        evhttp_add_header(output_headers, "Content-Length", buf);
    }

    int r_ = evhttp_make_request(evcon, req, (evhttp_cmd_type)eHttpType, uri);
    if (r_ != 0) {

        if (http_uri) {
            evhttp_uri_free(http_uri);
        }

        // LOG(ERROR) << " evhttp_make_request() failed" << " " << __FUNCTION__ << " " << __LINE__;

        return false;
    }

    if (http_uri) {
        evhttp_uri_free(http_uri);
    }

    return true;
}

bool HttpClient::DoGet(const std::string &strUri, HTTP_RESP_FUNCTOR_PTR pCB, const std::map<std::string, std::string> &xHeaders, const Guid id) {
    std::string memo;
    return MakeRequest(strUri, pCB, "", xHeaders, HttpType::SQUICK_HTTP_REQ_GET, memo);
}

reqid_t HttpClient::GenerateRequestID() {
    last_req_id_++;
    if (last_req_id_ > 0xffffffffffff) {
        last_req_id_ = 0;
    }
    return last_req_id_;
}

Awaitable<HttpClientResponseData> HttpClient::CoGet(const std::string &url, const std::map<std::string, std::string> &xHeaders) {
    HTTP_RESP_FUNCTOR_PTR pd(new HTTP_RESP_FUNCTOR(
        std::bind(&HttpClient::CoroutineResponseHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
    auto req_id = GenerateRequestID();
    Awaitable<HttpClientResponseData> awaitbale;
    awaitbale.data_.req_id = req_id;
    awaitbale.handler_ = std::bind(&HttpClient::CoroutineBinder, this, placeholders::_1);
    MakeRequest(url, pd, "", xHeaders, HttpType::SQUICK_HTTP_REQ_GET, "", Guid(0, req_id));
    return awaitbale;
}
Awaitable<HttpClientResponseData> HttpClient::CoPost(const std::string &url, const std::string &strPostData, const std::string &strMemoData,
                                                     const std::map<std::string, std::string> &xHeaders) {
    HTTP_RESP_FUNCTOR_PTR pd(new HTTP_RESP_FUNCTOR(
        std::bind(&HttpClient::CoroutineResponseHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
    auto req_id = GenerateRequestID();
    Awaitable<HttpClientResponseData> awaitbale;
    awaitbale.data_.req_id = req_id;
    awaitbale.handler_ = std::bind(&HttpClient::CoroutineBinder, this, placeholders::_1);
    MakeRequest(url, pd, strPostData, xHeaders, HttpType::SQUICK_HTTP_REQ_POST, strMemoData, Guid(0, req_id));
    return awaitbale;
}

void HttpClient::CoroutineBinder(Awaitable<HttpClientResponseData> *awaitble) {
    if (awaitble == nullptr) {
        // LOG_ERROR("awaitble is nullptr", "");
        return;
    }
    reqid_t req_id = awaitble->data_.req_id;
    auto iter = co_awaitbles_.find(req_id);
    if (iter == co_awaitbles_.end()) {
        co_awaitbles_[req_id] = awaitble;
    } else {
        // LOG_ERROR("CoroutineBinder: same req id in a map, req id: %v, address %v ", req_id, awaitble->coro_handle_.address());
    }
}

void HttpClient::CoroutineResponseHandler(const Guid id, const int state_code, const std::string &strRespData, const std::string &strMemoData) {
    reqid_t req_id = id.nData64;
    auto iter = co_awaitbles_.find(req_id);
    if (iter == co_awaitbles_.end()) {
        // LOG_ERROR("CoroutineBinder: Not find req id in a map, req id: %v" << req_id);
        return;
    }
    auto awaitble = iter->second;
    awaitble->data_.error = 0;
    awaitble->data_.content = strRespData;
    awaitble->data_.state_code = state_code;

    // resume coroutine
    if (!awaitble->coro_handle_.done()) {
        awaitble->coro_handle_.resume();
    }

    // remove request infos
    co_awaitbles_.erase(iter);
}

bool HttpClient::DoPost(const std::string &strUri, const std::string &strPostData, const std::string &strMemoData, HTTP_RESP_FUNCTOR_PTR pCB,
                        const std::map<std::string, std::string> &xHeaders, const Guid id) {
    return MakeRequest(strUri, pCB, strPostData, xHeaders, HttpType::SQUICK_HTTP_REQ_POST, strMemoData);
}

void HttpClient::OnHttpReqDone(struct evhttp_request *req, void *ctx) {
    HttpObject *pHttpObj = (HttpObject *)(ctx);
    if (pHttpObj == NULL) {
        // LOG(ERROR) << "pHttpObj ==NULL" << " " << __FUNCTION__ << " " << __LINE__;
        return;
    }
    std::string strResp;

    if (req == NULL) {
        /* If req is NULL, it means an error occurred, but
         * sadly we are mostly left guessing what the error
         * might have been.  We'll do our best... */
        struct bufferevent *bev = (struct bufferevent *)pHttpObj->m_pBev;
        unsigned long oslerr = 0;
        int printed_err = 0;
        int errcode = EVUTIL_SOCKET_ERROR();

        std::string strErrMsg = "";
        fprintf(stderr, "some request failed - no idea which one though!\n");
        /* Print out the OpenSSL error queue that libevent
         * squirreled away for us, if any. */

        char buffer[1024] = {0};
        int nread = 0;

#if SQUICK_ENABLE_SSL
        while ((oslerr = bufferevent_get_openssl_error(bev))) {
            ERR_error_string_n(oslerr, buffer, sizeof(buffer));
            fprintf(stderr, "%s\n", buffer);
            strErrMsg += std::string(buffer);
            printed_err = 1;
        }
#endif
        /* If the OpenSSL error queue was empty, maybe it was a
         * socket error; let's try printing that. */
        if (!printed_err) {
            char tmpBuf[1024] = {0};
            snprintf(tmpBuf, 1024, "socket error = %s (%d)\n", evutil_socket_error_to_string(errcode), errcode);
            strErrMsg += std::string(tmpBuf);
        }

        strResp = strErrMsg;

        HttpClient *pHttpClient = (HttpClient *)(pHttpObj->m_pHttpClient);
        pHttpClient->mlHttpObject.push_back(pHttpObj);

        // LOG(ERROR) << strErrMsg << __FUNCTION__ << " " << __LINE__;
        return;
    }

    int nRespCode = evhttp_request_get_response_code(req);
    char buffer[4096] = {0};
    int nread = 0;

    while ((nread = evbuffer_remove(evhttp_request_get_input_buffer(req), buffer, sizeof(buffer))) > 0) {
        // TODO it's not good idea,to append or memcpy
        strResp += std::string(buffer, nread);
    }

    if (req->evcon) {
        evhttp_connection_free(req->evcon);
    }

    if (req->output_headers) {
        evhttp_clear_headers(req->output_headers);
    }

    if (pHttpObj) {
        if (pHttpObj->m_pBev) {
#if SQUICK_ENABLE_SSL
            SSL *pSSL = bufferevent_openssl_get_ssl(pHttpObj->m_pBev);
            SSL_free(pSSL);
#endif
        }
    }

#if PLATFORM != PLATFORM_WIN
    SQUICK_CRASH_TRY
#endif

    if (pHttpObj->m_pCB) {
        if (pHttpObj->m_pCB.get()) {
            HTTP_RESP_FUNCTOR fun(*pHttpObj->m_pCB.get());
            fun(pHttpObj->mID, nRespCode, strResp, pHttpObj->strMemo);
        }
    }

#if PLATFORM != PLATFORM_WIN
    SQUICK_CRASH_END
#endif

    HttpClient *pHttpClient = (HttpClient *)(pHttpObj->m_pHttpClient);
    pHttpClient->mlHttpObject.push_back(pHttpObj);
}
