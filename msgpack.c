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

#include <stdint.h>
#include <stdio.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int DumpMsgPack(FILE* from, FILE* to);

static int DumpMap(FILE* from, FILE* to, size_t size) {
  if (putc('{', to) == EOF) {
    perror("Failed to write opening bracket");
    return 0;
  }
  while (size-- > 0) {
    int result = DumpMsgPack(from, to);
    if (!result) return 0;
    if (putc(':', to) == EOF) {
      perror("Failed to write semicolon");
      return 0;
    }
    result = DumpMsgPack(from, to);
    if (!result) return 0;
    if (size && putc(',', to) == EOF) {
      perror("Failed to write comma");
      return 0;
    }
  }
  if (putc('}', to) == EOF) {
    perror("Failed to write closing bracket");
    return 0;
  }
  return 1;
}

static int DumpArray(FILE* from, FILE* to, size_t size) {
  if (putc('[', to) == EOF) {
    perror("Failed to write opening bracket");
    return 0;
  }
  while (size-- > 0) {
    int result = DumpMsgPack(from, to);
    if (!result) return 0;
    if (size && putc(',', to) == EOF) {
      perror("Failed to write comma");
      return 0;
    }
  }
  if (putc(']', to) == EOF) {
    perror("Failed to write closing bracket");
    return 0;
  }
  return 1;
}

static int DumpString(FILE* from, FILE* to, size_t size) {
  if (putc('"', to) == EOF) {
    perror("Failed to write opening quote");
    return 0;
  }
  for (char buffer[256]; size;) {
    size_t count = MIN(sizeof(buffer), size);
    if (!fread(buffer, count, 1, from)) {
      perror("Failed to read string");
      return 0;
    }
    if (!fwrite(buffer, count, 1, to)) {
      perror("Failed to write string");
      return 0;
    }
    size -= count;
  }
  if (putc('"', to) == EOF) {
    perror("Failed to write closing quote");
    return 0;
  }
  return 1;
}

#define FIXED(name, string)                           \
  static int name(FILE* from, FILE* to) {             \
    (void)from;                                       \
    if (!fwrite(string, sizeof(string) - 1, 1, to)) { \
      perror("Failed to write fixed " string);        \
      return 0;                                       \
    }                                                 \
    return 1;                                         \
  }

FIXED(OnNil, "null")
FIXED(OnFalse, "false")
FIXED(OnTrue, "true")

#define NOIMPL(name)                              \
  static int name(FILE* from, FILE* to) {         \
    (void)from;                                   \
    (void)to;                                     \
    fprintf(stderr, #name " is not implemented"); \
    return 0;                                     \
  }

NOIMPL(OnBin8)
NOIMPL(OnBin16)
NOIMPL(OnBin32)
NOIMPL(OnExt8)
NOIMPL(OnExt16)
NOIMPL(OnExt32)
NOIMPL(OnFloat32)
NOIMPL(OnFloat64)
NOIMPL(OnUint8)
NOIMPL(OnUint16)
NOIMPL(OnUint32)
NOIMPL(OnUint64)
NOIMPL(OnInt8)
NOIMPL(OnInt16)
NOIMPL(OnInt32)
NOIMPL(OnInt64)
NOIMPL(OnFixExt1)
NOIMPL(OnFixExt2)
NOIMPL(OnFixExt4)
NOIMPL(OnFixExt8)
NOIMPL(OnFixExt16)

#define SIZED(name, impl, size)                   \
  static int name(FILE* from, FILE* to) {         \
    uint8_t buffer[size];                         \
    if (!fread(buffer, size, 1, from)) {          \
      perror("Failed to read items count");       \
      return 0;                                   \
    }                                             \
    size_t count = 0;                             \
    for (size_t index = 0; index < size; ++index) \
      count = count << 8 | buffer[index];         \
    return impl(from, to, count);                 \
  }

SIZED(OnStr8, DumpString, 1)
SIZED(OnStr16, DumpString, 2)
SIZED(OnStr32, DumpString, 4)
SIZED(OnArray16, DumpArray, 2)
SIZED(OnArray32, DumpArray, 4)
SIZED(OnMap16, DumpMap, 2)
SIZED(OnMap32, DumpMap, 4)

static int (*const kHandlers[])(FILE*, FILE*) = {
    OnNil,     NULL,      OnFalse,   OnTrue,     OnBin8,    OnBin16, OnBin32,
    OnExt8,    OnExt16,   OnExt32,   OnFloat32,  OnFloat64, OnUint8, OnUint16,
    OnUint32,  OnUint64,  OnInt8,    OnInt16,    OnInt32,   OnInt64, OnFixExt1,
    OnFixExt2, OnFixExt4, OnFixExt8, OnFixExt16, OnStr8,    OnStr16, OnStr32,
    OnArray16, OnArray32, OnMap16,   OnMap32};

int DumpMsgPack(FILE* from, FILE* to) {
  int prefix = getc(from);
  if (prefix == EOF) {
    perror("Failed to read prefix");
    return 0;
  }
  if (prefix <= 0x7f) {
    int result = fprintf(to, "%u", prefix) > 0;
    if (!result) perror("Failed to write positive fixint");
    return result;
  }
  if (prefix <= 0x8f) return DumpMap(from, to, prefix & 0x0f);
  if (prefix <= 0x9f) return DumpArray(from, to, prefix & 0x0f);
  if (prefix <= 0xbf) return DumpString(from, to, prefix & 0x1f);
  if (prefix <= 0xdf) return (*kHandlers[prefix - 0xc0])(from, to);
  int result = fprintf(to, "%d", ~0 & prefix) > 0;
  if (!result) perror("Failed to write negative fixint");
  return result;
}
