// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <stdio.h>

#include "src/socket.h"

using namespace warhol;

int main() {
  warhol::WSAHandler wsa_handler;
  if (!wsa_handler.Init()) {
    fprintf(stderr, "Could not initialize sockets\n");
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

  printf("Binded the socket.\n");

  status = socket.Listen();
  if (!status.ok()) {
    LogStatus(status);
    return 1;
  }


  printf("Listening on socket.\n");
  fflush(stdout);


  Socket client_socket;
  status = socket.Accept(&client_socket);
  if (!status.ok()) {
    LogStatus(status);
    return 1;
  }

  printf("Accepted connection.\n");


  while (true) {
    uint8_t buf[1024];
    int read = 0;
    printf("Reading from socket.\n");
    status = client_socket.Recv(buf, sizeof(buf), &read);
    if (status.type() == Status::Type::kDisconnect) {
      printf("Client disconnected. Exiting.\n");
      return 0;
    } else if (!status.ok()) {
      LogStatus(status);
      return 1;
    }

    buf[read] = 0;  // 0 ended string.
    printf("READ: %s\n", buf);
  }
}
