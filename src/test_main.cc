#include <stdio.h>

#include <third_party/stb/stb_sprintf.h>

int main() {

  char buf[256];

  stbsp_sprintf(buf, "Test string: %s\n", "It works!");
  printf("Result: %s\n", buf);
}
