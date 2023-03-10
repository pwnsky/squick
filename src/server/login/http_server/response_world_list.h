
#pragma once

#include <list>
#include <squick/plugin/net/i_response.h>
#include <squick/struct/struct.h>

class ResponseWorldList : public IResponse
{
public:
	class World
	{
	public:
		int id;
		std::string name;
		SquickStruct::ServerState state;
		int count;
	};

	std::list<World> world;
};

AJSON(ResponseWorldList::World, id, name, state, count)
AJSON(ResponseWorldList, world, code, message)