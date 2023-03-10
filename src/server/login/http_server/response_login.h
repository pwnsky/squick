

#pragma once

#include <squick/plugin/net/i_response.h>

class ResponseLogin : public IResponse
{
public:
	std::string jwt;

};

AJSON(ResponseLogin, jwt, code, message)