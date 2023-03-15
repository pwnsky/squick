#pragma once
#include <squick/core/base.h>

class IMysqlModule
	: public IModule
{
public:
	// "mysqlx://root@127.0.0.1"
	virtual bool Connect(std::string url) = 0;
};
