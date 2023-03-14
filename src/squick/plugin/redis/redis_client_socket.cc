

#include "redis_client.h"
#include "redis_client_socket.h"

#if SQUICK_PLATFORM == SQUICK_PLATFORM_WIN

#elif SQUICK_PLATFORM == SQUICK_PLATFORM_APPLE
#include <arpa/inet.h>
#endif

//1048576 = 1024 * 1024
#define SQUICK_BUFFER_MAX_READ	1048576

RedisClientSocket::RedisClientSocket()
{
	bev = nullptr;
	base = nullptr;
	listener = nullptr;
	m_pRedisReader = nullptr;

	mNetStatus = SQUICK_NET_EVENT::SQUICK_NET_EVENT_NONE;
	m_pRedisReader = redisReaderCreate();
	fd_ = -1;

#ifdef _MSC_VER
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif

	base = event_base_new();
	assert(base != nullptr);
}

RedisClientSocket::~RedisClientSocket()
{
	if (m_pRedisReader)
	{
		redisReaderFree(m_pRedisReader);
		m_pRedisReader = nullptr;
	}

	if (bev)
	{
		bufferevent_free(bev);
		bev = nullptr;
	}

	if (base)
	{
		event_base_free(base);
		base = nullptr;
	}
}

int64_t RedisClientSocket::Connect(const std::string &ip, const int port)
{

	struct sockaddr_in addr;
	std::string realIP = GetIP(ip);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (evutil_inet_pton(AF_INET, realIP.c_str(), &addr.sin_addr) <= 0)
	{
		printf("inet_pton");
		return -1;
	}

	bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
	assert(bev != nullptr);

	int bRet = bufferevent_socket_connect(bev, (struct sockaddr*)&addr, sizeof(addr));
	if (0 != bRet)
	{
		//int nError = GetLastError();
		printf("bufferevent_socket_connect error");
		return -1;
	}

	fd_ = bufferevent_getfd(bev);

	bufferevent_setcb(bev, conn_readcb, conn_writecb, conn_eventcb, this);
	bufferevent_enable(bev, EV_READ | EV_WRITE);

	event_set_log_callback(&RedisClientSocket::log_cb);

	return fd_;
}

bool RedisClientSocket::ReConnect(const std::string& ip, const int port)
{
	std::cout << "RedisClientSocket::ReConnect: " << ip << ":" << port << std::endl;
	if (bev)
	{
		bufferevent_free(bev);
		fd_ = -1;
		bev = nullptr;
	}

	return Connect(ip, port);
}

int RedisClientSocket::Update()
{
	if (base)
	{
		event_base_loop(base, EVLOOP_ONCE | EVLOOP_NONBLOCK);
	}

	return 0;
}

bool RedisClientSocket::IsConnect()
{
	return mNetStatus == SQUICK_NET_EVENT::SQUICK_NET_EVENT_CONNECTED;
}

redisReader * RedisClientSocket::GetRedisReader()
{
	return m_pRedisReader;
}

int RedisClientSocket::Close()
{
    return 0;
}

int RedisClientSocket::Write(const char *buf, size_t count)
{
	if (buf == NULL || count <= 0)
	{
		return 0;
	}

	if (bev == nullptr || mNetStatus != SQUICK_NET_EVENT::SQUICK_NET_EVENT_CONNECTED)
	{
		return -1;
	}

	return bufferevent_write(bev, buf, count);
}

void RedisClientSocket::listener_cb(evconnlistener * listener, evutil_socket_t fd, sockaddr * sa, int socklen, void * user_data)
{
	
}

void RedisClientSocket::conn_readcb(bufferevent * bev, void * user_data)
{
	RedisClientSocket* pClientSocket = (RedisClientSocket*)user_data;

	struct evbuffer* input = bufferevent_get_input(bev);
	if (!input)
	{
		return;
	}

	size_t len = evbuffer_get_length(input);
	unsigned char *pData = evbuffer_pullup(input, len);
	redisReaderFeed(pClientSocket->m_pRedisReader, (const char *)pData, len);
	evbuffer_drain(input, len);
}

void RedisClientSocket::conn_writecb(bufferevent * bev, void * user_data)
{
}

void RedisClientSocket::conn_eventcb(bufferevent * bev, short events, void * user_data)
{
	RedisClientSocket* pClientSocket = (RedisClientSocket*)user_data;
	if (events & BEV_EVENT_CONNECTED)
	{
		pClientSocket->mNetStatus = SQUICK_NET_EVENT::SQUICK_NET_EVENT_CONNECTED;
	}
	else if (events & BEV_EVENT_TIMEOUT)
	{
		pClientSocket->mNetStatus = SQUICK_NET_EVENT::SQUICK_NET_EVENT_TIMEOUT;
	}
	else
	{
		pClientSocket->mNetStatus = SQUICK_NET_EVENT::SQUICK_NET_EVENT_ERROR;
	}
}

void RedisClientSocket::log_cb(int severity, const char * msg)
{
}

string RedisClientSocket::GetIP(const std::string& url)
{
	std::vector<std::string> ips;
	struct hostent* host = gethostbyname(url.c_str());
	if (!host)
	{
		return "";
	}

	//Aliases
	for (int i = 0; host->h_aliases[i]; i++)
	{
		printf("Aliases %d: %s\n", i + 1, host->h_aliases[i]);
	}

	//ip address type
	// SQUICK_WILL_DO 
	//printf("Address type: %s\n", (host->h_addrtype == AF_INET) ? "AF_INET" : "AF_INET6");

	//ip address
	for (int i = 0; host->h_addr_list[i]; i++)
	{
		char* ip = inet_ntoa(*(struct in_addr*)host->h_addr_list[i]);
		//printf("IP addr %d: %s\n", i, ip);
		ips.push_back(ip);
	}

	if (ips.empty())
	{
		return "";
	}

	return ips[0];
}
