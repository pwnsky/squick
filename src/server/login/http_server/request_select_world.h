
#pragma once

#include <squick/plugin/net/i_request.h>

class RequestSelectWorld : public IRequest
{
public:
	int id;

};

AJSON(RequestSelectWorld, id)