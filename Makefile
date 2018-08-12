#
#    Copyright (C) 2018 Alexey Dynda
#
#    This file is part of Tiny memory manager library.
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

default: all

DESTDIR ?=
BLD ?= ./bld

.SUFFIXES: .c .cpp .ino

$(BLD)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -std=gnu11 $(CCFLAGS) -c $< -o $@

$(BLD)/%.o: %.ino
	mkdir -p $(dir $@)
	$(CXX) -std=c++11 $(CCFLAGS) -x c++ -c $< -o $@

$(BLD)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) -std=c++11 $(CCFLAGS) -c $< -o $@

# ************* Common defines ********************

INCLUDES += \
        -I./src

CCFLAGS += -g -Os $(INCLUDES) -Wall -Werror -ffunction-sections -fdata-sections \
	-fno-exceptions \
	$(EXTRA_CCFLAGS)

LDFLAGS += -L$(BLD)

.PHONY: clean all check tiny_mm

SRCS = \
	src/tiny_mm.c \

OBJS = $(addprefix $(BLD)/, $(addsuffix .o, $(basename $(SRCS))))

####################### Compiling library #########################

$(BLD)/libtiny_mm.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

tiny_mm: $(BLD)/libtiny_mm.a

all: tiny_mm

check: tiny_mm $(BLD)/test/check.o
	$(CC) $(CPPFLAGS) $(BLD)/test/check.o $(LDFLAGS) -ltiny_mm -o $(BLD)/check
	$(BLD)/check

clean:
	rm -rf $(BLD)
