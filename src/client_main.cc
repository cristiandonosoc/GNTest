// This code has a BSD license. See LICENSE.

#include <stdio.h>

#include <iostream>


#include "log.h"
#include "socket.h"

#include <windows.h>

const char msg[] = "0123456789";

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

  status = socket.Connect("127.0.0.1", 2345);
  if (!status.ok()) {
    LogStatus(status);
    return 1;
  }

  StdoutAndFlush("Succesfully connected.");

  while (true) {

    StdoutAndFlush("Press any key to send message.");
    char inbuf[16];
    std::cin.getline(inbuf, sizeof(inbuf));

    StdoutAndFlush("Sending message.");
    int sent = 0;
    status = socket.Send((uint8_t*)msg, sizeof(msg), &sent);
    if (!status.ok()) {
      LogStatus(status);
      return 1;
    }
    StdoutAndFlush("Send message");

  }
}
