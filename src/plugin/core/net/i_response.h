#pragma once

#include "third_party/ajson/ajson.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <string>

class IResponse {
  public:
    enum ResponseType {
        SUCCESS,
        FAILED,
        AUTH_FAILED,
        SERVER_ERROR,
        QEUEST_ERROR,
    };

  public:
    ResponseType code;
    std::string msg;
};

AJSON(IResponse, code, msg)