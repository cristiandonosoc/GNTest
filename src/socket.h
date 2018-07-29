// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <winsock2.h>

#include "status.h"

namespace sock {

// WSAHandler ------------------------------------------------------------------

// Handles initialization and cleanup of WSA sockets.
// Needs a call Init, but the destructor will cleanup the socket.
class WSAHandler {
 public:
  WSAHandler();
  ~WSAHandler();

  bool Init();

  const WSAData& wsa_data() { return wsa_data_; }

 private:
  WSAData wsa_data_;
  bool valid_;
};

// Socket ----------------------------------------------------------------------

class Socket {
 public:
  Socket(int family, int sock_type, int proto);

  Status Bind(int family, uint32_t ip, uint16_t port);
  Status Listen();

  bool valid() const { return handle_ != INVALID_SOCKET; }


 private:
  sockaddr_in addr_;
  SOCKET handle_ = INVALID_SOCKET;
};

}  // namespace sock


