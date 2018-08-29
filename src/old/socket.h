// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

#include <winsock2.h>

#include "status.h"

namespace warhol {

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
  Socket();

  Status Init(int16_t family, int sock_type, int proto);
  Status SetNonBlocking();

  // Server
  Status Bind(int32_t ip, uint16_t port);
  Status Listen();
  Status Accept(Socket*);   // out is optional.

  // Client
  Status Connect(const std::string& ip, uint16_t port);

  // I/O
  Status Recv(uint8_t* buf, size_t buf_size, int* out_read);
  Status Send(const uint8_t* buf, size_t size, int* out_sent);

 public:
  int16_t sock_family = 0;
  SOCKET sock_handle = INVALID_SOCKET;
  sockaddr sock_addr;
};

}  // namespace warhol


