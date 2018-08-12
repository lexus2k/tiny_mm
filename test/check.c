/*
    Copyright (C) 2018 Alexey Dynda

    This file is part of Tiny memory manager library.

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
*/

#include "tiny_mm.h"
#include <stdio.h>
#include <assert.h>

// Allocate pool for memory manager
uint8_t pool[800];

static void check_alloc_free(void)
{
    int size = mm_free_size(pool);
    void *p1 = mm_alloc(pool, 50);
    void *p2 = mm_alloc(pool, 100);
    fprintf(stderr, "######### check mm_alloc and mm_free\n");
    assert( mm_free_size(pool) <= size - 150 );
    mm_print( pool );
    mm_free(pool, p1);
    mm_free(pool, p2);
    mm_print( pool );
    // The size after free should be equal if free correctly combines all blocks
    assert( size == mm_free_size(pool) );
}

static void check_resize_head(void)
{
    void *p1 = mm_alloc( pool, 120 );
    int remained = mm_free_size( pool );
    fprintf(stderr, "######### check mm_resize_head\n");
    p1 = mm_resize_head( pool, p1, 150 );
    mm_print( pool );
    // in current implementation mm_resize_head should not produce new blocks
    assert( remained - 30 == mm_free_size( pool ) );
    mm_free( pool, p1 );
}

static void check_resize_head_no_space_in_front(void)
{
    // check that if no space, resize will allocate new block
    void *p1 = mm_alloc(pool, 60);
    void *p2 = mm_alloc(pool, 120);
    int remained = mm_free_size( pool );
    fprintf(stderr, "######### check mm_resize_head no space in front\n");
    p1 = mm_resize_head( pool, p1, 90 );
    mm_print( pool );
    assert( mm_free_size( pool ) <= remained - 30 );
    mm_free( pool, p1 );
    mm_free( pool, p2 );
}

int main(int argc, char *argv[])
{
    int size;
    // init pool and print the content
    mm_init(pool, sizeof(pool));
    size = mm_free_size( pool );
    mm_print( pool );

    check_alloc_free();
    check_resize_head();
    check_resize_head_no_space_in_front();

    assert( size == mm_free_size(pool) );
    fprintf(stderr, "######### [DONE]\n");
    mm_print( pool );
    return 0;
}

