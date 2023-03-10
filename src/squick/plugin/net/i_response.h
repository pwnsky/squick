#pragma once

#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include "third_party/ajson/ajson.hpp"

class IResponse
{
public:
	enum ResponseType
	{
		RES_TYPE_SUCCESS,
		RES_TYPE_FAILED,
		RES_TYPE_AUTH_FAILED,
	};

public:

	ResponseType code;
	std::string message;

};

AJSON(IResponse, code, message)