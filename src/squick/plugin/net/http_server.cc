#include "http_server.h"
#include <thread>
#include <squick/core/base.h>

bool HttpServer::Update() {
    if (mxBase) {
        event_base_loop(mxBase, EVLOOP_ONCE | EVLOOP_NONBLOCK);
    }

    return true;
}

int HttpServer::StartServer(const unsigned short port) {
    /*
event_init();
struct evhttp *httpserv = evhttp_start(addr,port);
    evhttp_set_gencb(httpserv, reqHandler,NULL);
    event_dispatch();
    */

    // struct event_base *base;
    struct evhttp *http;
    struct evhttp_bound_socket *handle;

#if PLATFORM == PLATFORM_WIN
    WSADATA WSAData;
    WSAStartup(0x101, &WSAData);
#else
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return (1);
#endif

    mxBase = event_base_new();
    if (!mxBase) {
        std::cout << "create event_base fail" << std::endl;
        ;
        return 1;
    }

    http = evhttp_new(mxBase);
    if (!http) {
        std::cout << "create evhttp fail" << std::endl;
        ;
        return 1;
    }
#ifdef SQUICK_SSL
    // 创建SSL上下文环境 ，可以理解为 SSL句柄
    SSL_CTX *ctx = SSL_CTX_new(SSLv23_server_method());
    SSL_CTX_set_options(ctx, SSL_OP_SINGLE_DH_USE | SSL_OP_SINGLE_ECDH_USE | SSL_OP_NO_SSLv2);
    /* Cheesily pick an elliptic curve to use with elliptic curve ciphersuites.
     * We just hardcode a single curve which is reasonably decent.
     * See http://www.mail-archive.com/openssl-dev@openssl.org/msg30957.html */

    EC_KEY *ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (!ecdh)
        die_most_horribly_from_openssl_error("EC_KEY_new_by_curve_name");
    if (1 != SSL_CTX_set_tmp_ecdh(ctx, ecdh))
        die_most_horribly_from_openssl_error("SSL_CTX_set_tmp_ecdh");

    
    const char *certificate_chain = SQUICK_SSL_CERTIFICATE;
    const char *private_key = SQUICK_SSL_PRIVATE_KEY;

    // 设置服务器证书 和 服务器私钥 到 OPENSSL ctx上下文句柄中
    server_setup_certs(ctx, certificate_chain, private_key);

    /*
        使我们创建好的evhttp句柄 支持 SSL加密
        实际上，加密的动作和解密的动作都已经帮
        我们自动完成，我们拿到的数据就已经解密之后的
    */
    // std::cout << "bind port :" << port << std::endl;;
#endif

    handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", port);
    if (!handle) {
        std::cout << "bind port :" << port << " fail" << std::endl;
        ;
        perror("bind prot");
        return 1;
    }

    evhttp_set_gencb(http, listener_cb, (void *)this);

    return 0;
}

void HttpServer::listener_cb(struct evhttp_request *req, void *arg) {
    dout << "HttpServer::listener_cb" << endl;
    if (req == NULL) {
        std::cout << "req ==NULL"
                  << " " << __FUNCTION__ << " " << __LINE__;
        return;
    }

    HttpServer *pNet = (HttpServer *)arg;
    if (pNet == NULL) {
        std::cout << "pNet ==NULL"
                  << " " << __FUNCTION__ << " " << __LINE__;
        evhttp_send_error(req, HTTP_BADREQUEST, 0);
        return;
    }

    std::shared_ptr<HttpRequest> pRequest = pNet->AllocHttpRequest();
    if (pRequest == nullptr) {
        std::cout << "pRequest ==NULL"
                  << " " << __FUNCTION__ << " " << __LINE__;
        evhttp_send_error(req, HTTP_BADREQUEST, 0);
        return;
    }

    pRequest->req = req;

    // headers
    struct evkeyvalq *header = evhttp_request_get_input_headers(req);
    if (header == NULL) {
        pNet->mxHttpRequestPool.push_back(pRequest);

        std::cout << "header ==NULL"
                  << " " << __FUNCTION__ << " " << __LINE__;
        evhttp_send_error(req, HTTP_BADREQUEST, 0);
        return;
    }

    struct evkeyval *kv = header->tqh_first;
    while (kv) {
        pRequest->headers.insert(std::map<std::string, std::string>::value_type(kv->key, kv->value));

        kv = kv->next.tqe_next;
    }

    // uri
    const char *uri = evhttp_request_get_uri(req);
    if (uri == NULL) {
        pNet->mxHttpRequestPool.push_back(pRequest);

        std::cout << "uri ==NULL"
                  << " " << __FUNCTION__ << " " << __LINE__;
        evhttp_send_error(req, HTTP_BADREQUEST, 0);
        return;
    }

    pRequest->url = uri;
    pRequest->remoteHost = req->remote_host;
    pRequest->type = (HttpType)evhttp_request_get_command(req);

    // get decodeUri
    struct evhttp_uri *decoded = evhttp_uri_parse(uri);
    if (!decoded) {
        pNet->mxHttpRequestPool.push_back(pRequest);

        evhttp_send_error(req, HTTP_BADREQUEST, 0);
        std::cout << "bad request "
                  << " " << __FUNCTION__ << " " << __LINE__;
        return;
    }

    // path
    const char *urlPath = evhttp_uri_get_path(decoded);
    if (urlPath != NULL) {
        pRequest->path = urlPath;
    } else {
        std::cout << "urlPath ==NULL "
                  << " " << __FUNCTION__ << " " << __LINE__;
    }

    evhttp_uri_free(decoded);

    dout << "Got a GET request:" << uri << std::endl;
    if (evhttp_request_get_command(req) == evhttp_cmd_type::EVHTTP_REQ_GET) {
        // OnGetProcess(request, );

        struct evkeyvalq params;
        evhttp_parse_query(uri, &params);
        struct evkeyval *kv = params.tqh_first;
        while (kv) {
            pRequest->params.insert(std::map<std::string, std::string>::value_type(kv->key, kv->value));

            kv = kv->next.tqe_next;
        }
    }

    struct evbuffer *in_evb = evhttp_request_get_input_buffer(req);
    if (in_evb == NULL) {
        pNet->mxHttpRequestPool.push_back(pRequest);

        std::cout << "urlPath ==NULL "
                  << " " << __FUNCTION__ << " " << __LINE__;
        return;
    }

    size_t len = evbuffer_get_length(in_evb);
    if (len > 0) {
        unsigned char *pData = evbuffer_pullup(in_evb, len);
        pRequest->body.clear();

        if (pData != NULL) {
            pRequest->body.append((const char *)pData, len);
        }
    }

    if (pNet->mFilter) {
        // return 401
        try {
            WebStatus xWebStatus = pNet->mFilter(pRequest);
            if (xWebStatus != WebStatus::WEB_OK) {

                pNet->mxHttpRequestPool.push_back(pRequest);

                // 401
                pNet->ResponseMsg(pRequest, "Filter error", xWebStatus);
                return;
            }
        } catch (std::exception &e) {
            pNet->ResponseMsg(pRequest, e.what(), WebStatus::WEB_ERROR);
        } catch (...) {
            pNet->ResponseMsg(pRequest, "UNKNOW ERROR", WebStatus::WEB_ERROR);
        }
    }

    // call cb
    try {
        if (pNet->mReceiveCB) {
            pNet->mReceiveCB(pRequest);
        } else {
            pNet->ResponseMsg(pRequest, "NO PROCESSER", WebStatus::WEB_ERROR);
        }
    } catch (std::exception &e) {
        pNet->ResponseMsg(pRequest, e.what(), WebStatus::WEB_ERROR);
    } catch (...) {
        pNet->ResponseMsg(pRequest, "UNKNOW ERROR", WebStatus::WEB_ERROR);
    }
}

std::shared_ptr<HttpRequest> HttpServer::AllocHttpRequest() {
    if (mxHttpRequestPool.size() <= 0) {
        for (int i = 0; i < 100; ++i) {
            std::shared_ptr<HttpRequest> request = std::shared_ptr<HttpRequest>(new HttpRequest(++mIndex));
            mxHttpRequestPool.push_back(request);
            mxHttpRequestMap.AddElement(request->id, request);
        }
    }

    std::shared_ptr<HttpRequest> pRequest = mxHttpRequestPool.front();
    mxHttpRequestPool.pop_front();
    pRequest->Reset();

    return pRequest;
}

bool HttpServer::ResponseMsg(std::shared_ptr<HttpRequest> req, const std::string &msg, WebStatus code, const std::string &strReason) {
    if (req == nullptr) {
        return false;
    }
    evhttp_request *pHttpReq = (evhttp_request *)req->req;
    // create buffer
    struct evbuffer *eventBuffer = evbuffer_new();

    // send data
    evbuffer_add_printf(eventBuffer, "%s", msg.c_str());

    evhttp_add_header(evhttp_request_get_output_headers(pHttpReq), "Content-Type", "application/json");

    evhttp_send_reply(pHttpReq, code, strReason.c_str(), eventBuffer);

    // free
    evbuffer_free(eventBuffer);

    mxHttpRequestPool.push_back(req);

    return true;
}

std::shared_ptr<HttpRequest> HttpServer::GetHttpRequest(const int64_t index) { return mxHttpRequestMap.GetElement(index); }