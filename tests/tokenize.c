/*
 * Copyright 2018 Tarkan Al-Kazily
 */

#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "inttypes.h"
#include "ctype.h"

// Tokenizes a string into argv style arguments. Returns the number of arguments found.
static uint16_t tokenize(char *str, uint16_t len, char **argv, uint16_t max_args) {
  char *ptr = str;
  uint16_t argc = 0;
  char *token = NULL;

  // Convert trailing \r to a null byte
  str[strlen(str)] = '\0';
  // Ignore leading whitespace
  while (isspace(*ptr)) {
    ptr++;
  }
  token = strtok(ptr, " ");
  while (token && argc < max_args) {
    argv[argc] = token;
    argc++;
    token = strtok(NULL, " ");
  }

  return argc;
}

static void test(char *input) {
  char *result[16];
  char hello_mod[1024];
  strcpy(hello_mod, input);
  uint16_t res = tokenize(hello_mod, strlen(hello_mod), result, 16);
  printf("Case: %s\n", input);
  printf("res: %d\n", res);
  for (int i = 0; i < res && i < 16; i++) {
    printf("result[%d]: %s\n", i, result[i]);
  }
  printf("=============\n");
}

int main(int argc, char **argv) {
  char *hello = "echo hello world\r";
  char *spacehello = "   echo    hello    world\r";
  char *goodbye = "c h lo wor dasd asdfa asdfas asdfasd\r";
  char *tests = "\r";
  char *okay = "okay there\r";
  test(hello);
  test(spacehello);
  test(goodbye);
  test(tests);
  test(okay);
  return 0;
}
