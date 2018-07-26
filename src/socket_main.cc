// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#include <winsock2.h>

#include <cstdio>

#include "src/socket.h"


void PrintWSAError(const char *header) {
  int err_no = WSAGetLastError();
  char* buffer;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, err_no, 0, (LPSTR)&buffer, 0, NULL);
  fprintf(stderr, "Error -> %s: %s\n", header, buffer);
}


int main() {
  sock::WSAHandler wsa_handler;
  if (!wsa_handler.Init()) {
    fprintf(stderr, "Could not initialize sockets\n");
    return 1;
  }

  SOCKET socket_handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket_handle == INVALID_SOCKET) {
    PrintWSAError("Could not create socket");
    return 1;
  }

  // Bind the action
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(2345);

  int bind_res = bind(socket_handle, (const sockaddr*)&addr, sizeof(addr));
  if (bind_res == SOCKET_ERROR) {
    PrintWSAError("Could not bind socket");
    return 1;
  }

  // Set non-blocking
  DWORD nb = 1;
  int nb_res = ioctlsocket(socket_handle, FIONBIO, &nb);
  if (nb_res == SOCKET_ERROR) {
    PrintWSAError("Could not set non-blocking");
    return 1;
  }

  if (listen(socket_handle, SOMAXCONN) == SOCKET_ERROR) {
    PrintWSAError("Error on listen");
    return 1;
  }

/*   SOCKET client_socket = INVALID_SOCKET; */
/*   client_socket = accept(socket_handle, NULL, NULL); */
/*   if (client_socket == INVALID_SOCKET) { */
/*     PrintWSAError("Failed to accept connection"); */
/*     return 1; */
/*   } */

/*   bool running = true; */
/*   while (running) { */

/*   } */



  printf("Succesfully created socket\n");

}
