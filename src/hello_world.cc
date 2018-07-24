#include <cstdio>
#include <functional>

#include "src/socket.h"

void CallFunction(std::function<void()> f) {
  f();
}

int main() {

  auto test = []() {
    printf("Hello, World!\n");
  };
  CallFunction(std::move(test));

  CreateSocket();
}
