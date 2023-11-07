#include <stdio.h>
#include <string.h>
#include <internal/io.h>

int puts(const char * s) {
  int val = write(1, s, strlen(s));
  write(1, "\n", 1);
  return val;
}
