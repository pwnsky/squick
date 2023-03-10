
#ifndef NFREDISPLUGIN_NFREDISCOMMAND_H
#define NFREDISPLUGIN_NFREDISCOMMAND_H

#include <iostream>
#include <vector>
#include <sstream>

#include "redis_protocol_define.h"

class RedisCommand
{
public:
    RedisCommand( const std::string& cmd )
    {
        mxParam.push_back( cmd );
    }

    ~RedisCommand()
    {

    }

    template <typename T>
    RedisCommand& operator<<( const T& t )
    {
        std::stringstream str;
        str << t;
        mxParam.push_back(str.str());
        return *this;
    }

    std::string Serialize() const
    {
        std::stringstream xDataString;
        xDataString << '*' << mxParam.size() << NFREDIS_CRLF;
        std::vector<std::string>::const_iterator it = mxParam.begin();
        for ( ; it != mxParam.end(); ++it )
        {
            xDataString << '$' << it->size() << NFREDIS_CRLF;
            xDataString << *it << NFREDIS_CRLF;
        }

        return xDataString.str();
    }

private:
    std::vector<std::string> mxParam;
};


#endif //NFREDISPLUGIN_NFREDISCOMMAND_H
