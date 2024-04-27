#include "http_server.h"
#include <event2/bufferevent_ssl.h>
#include <openssl/ssl.h>
#include <squick/core/base.h>
#include <thread>
bool HttpServer::Update() {
    if (mxBase) {
        event_base_loop(mxBase, EVLOOP_ONCE | EVLOOP_NONBLOCK);
    }

    return true;
}

int HttpServer::StartServer(const unsigned short port, bool is_ssl = false) {
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
        return 1;
    }
    if (is_ssl) {
        // to do
    }

    handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", port);
    if (!handle) {
        std::cout << "bind port :" << port << " fail" << std::endl;
        ;
        perror("bind prot");
        return 1;
    }
    evhttp_set_allowed_methods(http, EVHTTP_REQ_GET | EVHTTP_REQ_POST | EVHTTP_REQ_OPTIONS);

    evhttp_set_gencb(http, listener_cb, (void *)this);
    return 0;
}

void HttpServer::listener_cb(struct evhttp_request *req, void *arg) {
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
        try {
            WebStatus xWebStatus = pNet->mFilter(pRequest);
            if (xWebStatus == WebStatus::WEB_IGNORE) {

            } else if (xWebStatus == WebStatus::WEB_RETURN) {
                return;
            } else {
                pNet->mxHttpRequestPool.push_back(pRequest);
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

bool HttpServer::SetHeader(std::shared_ptr<HttpRequest> req, const std::string &key, const std::string &value) {
    if (req == nullptr) {
        return false;
    }
    evhttp_request *pHttpReq = (evhttp_request *)req->req;
    evhttp_add_header(evhttp_request_get_output_headers(pHttpReq), key.c_str(), value.c_str());
    return true;
}

std::shared_ptr<HttpRequest> HttpServer::GetHttpRequest(const int64_t index) { return mxHttpRequestMap.GetElement(index); }