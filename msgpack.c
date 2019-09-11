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

int DumpMsgPack(FILE* from, FILE* to);

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
NOIMPL(OnStr8)
NOIMPL(OnStr16)
NOIMPL(OnStr32)
NOIMPL(OnArray16)
NOIMPL(OnArray32)
NOIMPL(OnMap16)
NOIMPL(OnMap32)

static int (*const kHandlers[])(FILE*, FILE*) = {
    OnNil,     NULL,      OnFalse,   OnTrue,     OnBin8,    OnBin16, OnBin32,
    OnExt8,    OnExt16,   OnExt32,   OnFloat32,  OnFloat64, OnUint8, OnUint16,
    OnUint32,  OnUint64,  OnInt8,    OnInt16,    OnInt32,   OnInt64, OnFixExt1,
    OnFixExt2, OnFixExt4, OnFixExt8, OnFixExt16, OnStr8,    OnStr16, OnStr32,
    OnArray16, OnArray32, OnMap16,   OnMap32};

int DumpMsgPack(FILE* from, FILE* to) {
  int prefix = fgetc(from);
  if (prefix == -1) return 0;
  if (0xc0 <= prefix && prefix <= 0xdf)
    return (*kHandlers[prefix - 0xc0])(from, to);
  // TODO: Implement immediate fixtypes
  return 0;
}
