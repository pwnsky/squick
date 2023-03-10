#pragma once

#include <squick/plugin/net/i_request.h>
#include <string>

class RequestLogin : public IRequest
{
public:
	std::string user;
	std::string password;

};

AJSON(RequestLogin, user, password)