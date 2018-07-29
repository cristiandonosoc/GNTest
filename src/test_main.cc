#include <stdio.h>

#include <third_party/stb/stb_sprintf.h>

#include "status.h"


int main() {

  char buf[256];

  stbsp_sprintf(buf, "Test string: %s\n", "It works!");
  printf("Result: %s\n", buf);

  sock::Status s;
  printf("MESSAGE: %s\n", s.err_msg().c_str());

  sock::Status status("Testing the error %s\n", "test");
  printf("Printing error: %s\n", status.err_msg().c_str());
}
