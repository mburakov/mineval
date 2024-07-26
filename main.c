/*
 * Copyright (C) 2019 Mikhail Burakov. This file is part of mineval.
 *
 * mineval is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mineval is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mineval.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int DumpMsgPack(FILE* from, FILE* to);

static FILE* ConnectNvim() {
  int sock = -1;
  do {
    char* env = getenv("NVIM");
    if (!env) {
      fprintf(stderr, "Not running in nvim\n");
      break;
    }
    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
      perror("Failed to create socket");
      break;
    }
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, env, sizeof(addr.sun_path) - 1);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr))) {
      perror("Failed to connect nvim");
      break;
    }
    FILE* result = fdopen(sock, "r+b");
    if (!result) {
      perror("Failed to open stream");
      break;
    }
    return result;
  } while (0);
  if (sock != -1) close(sock);
  return NULL;
}

static int FormatLength(FILE* stream, size_t length) {
  char buffer[5];
  size_t size;
  if (length <= 31) {
    buffer[0] = '\xa0' | (char)length;
    size = 1;
  } else if (length <= 255) {
    buffer[0] = '\xd9';
    buffer[1] = (char)length;
    size = 2;
  } else if (length <= 65535) {
    buffer[0] = '\xda';
    buffer[1] = (char)(length >> 8);
    buffer[2] = (char)(length);
    size = 3;
  } else {
    buffer[0] = '\xdb';
    buffer[1] = (char)(length >> 24);
    buffer[2] = (char)(length >> 16);
    buffer[3] = (char)(length >> 8);
    buffer[4] = (char)(length);
    size = 5;
  }
  if (!fwrite(buffer, size, 1, stream)) {
    perror("Failed to format length");
    return 0;
  }
  return 1;
}

int main(int argc, char** argv) {
  FILE* stream = NULL;
  int result = EXIT_FAILURE;
  do {
    if (argc < 2) {
      if (DumpMsgPack(stdin, stdout)) result = EXIT_SUCCESS;
      break;
    }
    if (!(stream = ConnectNvim())) break;
    static const char kPrefix[] = "\x94\x00\x00\xa9nvim_eval\x91";
    if (!fwrite(kPrefix, sizeof(kPrefix) - 1, 1, stream)) {
      perror("Failed to write prefix");
      break;
    }
    size_t length = strlen(argv[1]);
    if (!FormatLength(stream, length)) break;
    if (!fwrite(argv[1], length, 1, stream)) {
      perror("Failed to write string");
      break;
    }
    if (!DumpMsgPack(stream, stdout)) break;
    result = EXIT_SUCCESS;
  } while (0);
  if (stream) fclose(stream);
  return result;
}
