

#include "udp_module.h"
#include "net.h"

#define BUF_SIZE 14500
// 500

static void udp_cb(const int sock, short int which, void *arg) {
    UDPModule *udpModule = (UDPModule *)arg;

    struct sockaddr_in client_addr;
    socklen_t size = sizeof(client_addr);
    char buf[BUF_SIZE];
    std::string data(buf);
    std::cout << std::this_thread::get_id() << " received:" << data.length() << std::endl;

    /* Recv the data, store the address of the sender in server_sin */
    if (recvfrom(sock, (char *)&buf, sizeof(buf) - 1, 0, (struct sockaddr *)&client_addr, &size) == -1) {
        perror("recvfrom()");
        // event_loopbreak();
    }

    /* Send the data back to the client */
    if (sendto(sock, data.c_str(), data.length(), 0, (struct sockaddr *)&client_addr, size) == -1) {
        perror("sendto()");
        // event_loopbreak();
    }
}

int bind_socket(struct event *ev, int port, void *p) {
    int sock_fd;
    int flag = 1;
    struct sockaddr_in sin;
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("socket()");
        return -1;
    }

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(int)) < 0) {
        perror("setsockopt()");
        return 1;
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    if (::bind(sock_fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("bind()");
        return -1;
    } else {
        printf("bind() success - [%u]\n", port);
    }

    event_set(ev, sock_fd, EV_READ | EV_PERSIST, (void (*)(evutil_socket_t, short, void *)) & udp_cb, p);
    if (event_add(ev, NULL) == -1) {
        printf("event_add() failed\n");
    }

    return 0;
}

UDPModule::UDPModule(IPluginManager *p) {
    pm_ = p;

    mnBufferSize = 0;
}

UDPModule::~UDPModule() {}

bool UDPModule::Start() { return true; }

bool UDPModule::AfterStart() { return true; }

int UDPModule::Listen(const unsigned int nMaxClient, const unsigned short nPort, const int nCpuCount, const uint32_t expand_buffer_size) {
    /* Start. event */
    mxBase = event_init();
    if (mxBase == NULL) {
        printf("event_init() failed\n");
        return -1;
    }

    /* Bind socket */
    if (bind_socket(&udp_event, nPort, this) != 0) {
        printf("bind_socket() failed\n");
        return -1;
    }

    return 0;
}

void UDPModule::RemoveReceiveCallBack(const uint32_t msg_id) {}

bool UDPModule::AddReceiveCallBack(const uint32_t msg_id, const NET_RECEIVE_FUNCTOR_PTR &cb) { return true; }

bool UDPModule::AddReceiveCallBack(const NET_RECEIVE_FUNCTOR_PTR &cb) { return true; }

bool UDPModule::AddEventCallBack(const NET_EVENT_FUNCTOR_PTR &cb) { return true; }

bool UDPModule::Update() {
    if (mxBase) {
        event_base_loop(mxBase, EVLOOP_ONCE | EVLOOP_NONBLOCK);
    }

    return true;
}

bool UDPModule::SendMsgWithOutHead(const uint32_t msg_id, const std::string &msg, const socket_t sock) { return true; }

bool UDPModule::SendMsgToAllClientWithOutHead(const uint32_t msg_id, const std::string &msg) { return true; }

bool UDPModule::SendMsgPB(const uint32_t msg_id, const google::protobuf::Message &xData, const socket_t sock) { return true; }

bool UDPModule::SendMsgPB(const uint32_t msg_id, const google::protobuf::Message &xData, const socket_t sock, const Guid id) { return true; }

bool UDPModule::SendMsg(const uint32_t msg_id, const std::string &xData, const socket_t sock) { return true; }

bool UDPModule::SendMsg(const uint32_t msg_id, const std::string &xData, const socket_t sock, const Guid id) { return true; }

bool UDPModule::SendMsgPBToAllClient(const uint32_t msg_id, const google::protobuf::Message &xData) { return true; }

bool UDPModule::SendMsgPB(const uint32_t msg_id, const google::protobuf::Message &xData, const socket_t sock, const std::vector<Guid> *pClientIDList) {
    return true;
}

bool UDPModule::SendMsgPB(const uint32_t msg_id, const std::string &strData, const socket_t sock, const std::vector<Guid> *pClientIDList) { return true; }

INet *UDPModule::GetNet() { return nullptr; }

void UDPModule::OnReceiveNetPack(const socket_t sock, const uint32_t msg_id, const char *msg, const uint32_t len) {}

void UDPModule::OnSocketNetEvent(const socket_t sock, const SQUICK_NET_EVENT eEvent, INet *pNet) {}
