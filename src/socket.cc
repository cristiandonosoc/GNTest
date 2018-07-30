// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <winsock2.h>
#include <Ws2tcpip.h>

#include "src/socket.h"

namespace warhol {

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

Status GetWSAError(const char *header) {
  int err_no = WSAGetLastError();
  char* buffer;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, err_no, 0, (LPSTR)&buffer, 0, NULL);
  return Status("Error -> %s: %s\n", header, buffer);
}

}  // namespace


Socket::Socket() = default;


Status Socket::SetNonBlocking() {
  // Set non-blocking
  DWORD nb = 1;
  int nb_res = ioctlsocket(sock_handle, FIONBIO, &nb);
  if (nb_res == SOCKET_ERROR)
    return GetWSAError("Could not set non-blocking");
  return Status();
}

Status Socket::Init(int16_t family, int sock_type, int protocol) {
  sock_handle = socket(family, sock_type, protocol);
  if (sock_handle == INVALID_SOCKET)
    return GetWSAError("Could not create socket");
  sock_family = family;
  return Status();
}

// Server --------------------------------------

Status Socket::Bind(int32_t ip, uint16_t port) {
  if (sock_handle == INVALID_SOCKET)
    return Status("Socket is not valid");

  // Bind the action
  sockaddr_in addr;
  addr.sin_family = sock_family;
  addr.sin_addr.s_addr = ip;
  addr.sin_port = htons(port);

  int bind_res = bind(sock_handle, (const sockaddr*)&addr, sizeof(addr));
  if (bind_res == SOCKET_ERROR)
    return GetWSAError("Could not bind socket");

  return Status();
}

Status Socket::Listen() {
  if (listen(sock_handle, SOMAXCONN) == SOCKET_ERROR)
    return GetWSAError("Error on listen");
  return Status();
}

Status Socket::Accept(Socket* socket) {
  SOCKET new_handle = accept(sock_handle, &socket->sock_addr, 0);
  if (new_handle == SOCKET_ERROR)
    return GetWSAError("Error on accept");

  socket->sock_family = this->sock_family;
  socket->sock_handle = new_handle;
  return Status();
}

// Client --------------------------------------


Status Socket::Connect(const std::string& ip, uint16_t port) {
  sockaddr_in addr;
  addr.sin_family = sock_family;
  inet_pton(sock_family, ip.c_str(), &addr.sin_addr.s_addr);
  addr.sin_port = htons(port);

  if (connect(sock_handle, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    return GetWSAError("Error on connect");
  return Status();
}

// I/O -----------------------------------------

Status Socket::Recv(uint8_t* buf, size_t buf_size, int* out_read) {
  int read = recv(sock_handle, (char*)buf, buf_size, 0);
  if (read == SOCKET_ERROR)
    return GetWSAError("Error on Recv");

  *out_read = read;
  return Status();
}

Status Socket::Send(const uint8_t* buf, size_t size, int* out_sent) {
  int sent = send(sock_handle, (char*)buf, size, 0);
  if (sent == SOCKET_ERROR)
    return GetWSAError("Error on Send");
  *out_sent = sent;
  return Status();
}

}  // namespace warhol

