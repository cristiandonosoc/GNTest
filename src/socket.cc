// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <winsock2.h>

#include "src/socket.h"

namespace sock {

// WSAData ---------------------------------------------------------------------

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

// Socket ----------------------------------------------------------------------


namespace {

Err CreateWSAError(const char *header) {
  int err_no = WSAGetLastError();
  char* buffer;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, err_no, 0, (LPSTR)&buffer, 0, NULL);
  fprintf(stderr, "Error -> %s: %s\n", header, buffer);
  return Err();
}


}  // namespace


Socket::Socket(int family, int sock_type, int protocol) {
  handle_ = socket(family, sock_type, protocol);
}



}  // namespace sock

