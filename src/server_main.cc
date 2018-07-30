// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>

#include "log.h"
#include "socket.h"
#include "string.h"

using namespace warhol;

int main() {
  warhol::WSAHandler wsa_handler;
  if (!wsa_handler.Init()) {
    StdoutAndFlush("Could not initialize sockets");
    return 1;
  }

  Socket socket;
  auto status = socket.Init(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (!status.ok()) {
    LogStatus(status);
    return 1;
  }

  /* status = socket.SetNonBlocking(); */
  /* if (!status.ok()) { */
  /*   LogStatus(status); */
  /*   return 1; */
  /* } */

  status = socket.Bind(INADDR_ANY, 2345);
  if (!status.ok()) {
    LogStatus(status);
    return 1;
  }

  StdoutAndFlush("Binded the socket.");

  status = socket.Listen();
  if (!status.ok()) {
    LogStatus(status);
    return 1;
  }

  StdoutAndFlush("Listening on socket.");

  Socket client_socket;
  status = socket.Accept(&client_socket);
  if (!status.ok()) {
    LogStatus(status);
    return 1;
  }

  StdoutAndFlush("Accepted connection.");


  while (true) {
    StdoutAndFlush("Waiting to read from socket.");

    uint8_t buf[1024];
    int read = 0;
    status = client_socket.Recv(buf, sizeof(buf), &read);

    if (status.type() == Status::Type::kDisconnect) {
      StdoutAndFlush("Client disconected. Exiting.");
      return 0;
    } else if (!status.ok()) {
      LogStatus(status);
      return 1;
    }

    buf[read] = 0;  // 0 ended string.
    StdoutAndFlush(StringPrintf("READ %d bytes: \"%s\"", read, buf));
  }
}
