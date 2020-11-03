# Tiny Memory Manager

[tocstart]: # (toc start)

  * [Introduction](#introduction)
  * [Key Features](#key-features)
  * [Supported platforms](#supported-platforms)
  * [Setting up](#setting-up)
  * [License](#license)

[tocend]: # (toc end)


## Introduction

Tiny memory manager can be used for dynamic memory allocations. This library can be used to allocate memory blocks from predefined pool.
Please submit any comments and wishes to issues section.

## Key Features

 * Simple API and easy to use
 * Allows resizing of allocated blocks
 * Small overhead (but please, be careful: each allocation eats at least 5 bytes)

```cpp
#include "tiny_mm.h"
#include <stdint.h>

uint8_t pool[512];

int main(int argc, char *argv[])
{
    mm_init( pool, sizeof(pool) );
    void *p = mm_alloc( pool, 20 );
    mm_free( pool, p );
}

```

## Supported platforms

Any platform should be fine with this library.

## Setting up

*Setting up for Arduino*
 * Download source from https://github.com/lexus2k/tiny_mm
 * Put the sources to Arduino/libraries/ folder

*Compiling with gcc*
 * Download source from https://github.com/lexus2k/tiny_mm
 * Build the library
   * cd tiny_mm && make
   * Link library to your project

## License

The library is free. If this project helps you, you can give me a cup of coffee.
[![Donate via Paypal](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.me/lexus2k)


== License ==

Copyright (C) 2018, 2020 Alexey Dynda

This file is part of Tiny memory manager Library.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
