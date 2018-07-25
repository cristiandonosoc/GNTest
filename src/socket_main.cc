#include <winsock2.h>

#include <cstdio>


class SocketInitialization {
 public:
  SocketInitialization() = default;
  ~SocketInitialization() {
    if (valid) {
      WSACleanup();
    }
  }

  bool Initialize() {
    int res = WSAStartup(MAKEWORD(2,2), &wsa_data);
    valid = res == NO_ERROR;
    return valid;
  }

  WSAData wsa_data;
  bool valid;
};

void PrintWSAError(const char *header) {
  int err_no = WSAGetLastError();
  char* buffer;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, err_no, 0, (LPSTR)&buffer, 0, NULL);
  fprintf(stderr, "Error -> %s: %s\n", header, buffer);
}


int main() {
  SocketInitialization sock_init;
  sock_init.Initialize();
  if (!sock_init.valid) {
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

  printf("Succesfully created socket\n");

}
