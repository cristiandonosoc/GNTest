// This code has a BSD license. See LICENSE.

#include <stdio.h>

#include "src/socket.h"

#include <windows.h>

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

  status = socket.Connect("127.0.0.1", 2345);
  if (!status.ok()) {
    LogStatus(status);
    return 1;
  }

  printf("Succesfully connected.\n");

  const char msg[] = "SUPER MESSAGE TO SEND!";
  int sent = 0;
  printf("Sending message.\n");
  status = socket.Send((uint8_t*)msg, sizeof(msg), &sent);
  if (!status.ok()) {
    LogStatus(status);
    return 1;
  }

  while (true)
    Sleep(1000);
}
