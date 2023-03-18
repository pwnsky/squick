#pragma once

#include "third_party/ajson/ajson.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <string>

class IResponse {
  public:
    enum ResponseType {
        RES_TYPE_SUCCESS,
        RES_TYPE_FAILED,
        RES_TYPE_AUTH_FAILED,
    };

  public:
    ResponseType code;
    std::string message;
};

AJSON(IResponse, code, message)