// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <winsock2.h>

#include "src/socket.h"

namespace sock {

WSAHandler::WSAHandler() = default;
WSAHandler::~WSAHandler() {
  if (valid_) {
    WSACleanup();
  }
}

bool WSAHandler::Init() {
  int res = WSAStartup(MAKEWORD(2,2), &wsa_data_);
  valid_ = res == NO_ERROR;
  return valid_;
}

}  // namespace sock

