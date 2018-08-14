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

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define MM_ALIGN_LEFT(x) (uint8_t *)((uintptr_t)(x) & ~(uintptr_t)(MM_ALIGNMENT-1))
#define MM_ALIGN_RIGHT(x) (uint8_t *)((uintptr_t)(x + MM_ALIGNMENT - 1) & ~(uintptr_t)(MM_ALIGNMENT-1))

typedef struct _tiny_block_t
{
    struct _tiny_block_t * next;
    struct _tiny_block_t * prev;
    uint8_t flags;  ///< flags
} tiny_block_t;

static const int MIN_OVERHEAD = MM_ALIGNMENT + sizeof(tiny_block_t);

static inline int is_free_block( tiny_block_t * block )
{
    return block->flags == 0;
}

static inline int get_block_size( tiny_block_t * block )
{
    return (uint8_t *)block->next - (uint8_t *)block;
}

static inline int get_block_data_size( tiny_block_t * block )
{
    return get_block_size( block ) - sizeof(tiny_block_t);
}

static inline tiny_block_t* get_first_block( void* pool )
{
    return (tiny_block_t *)(pool);
}

static inline void mark_block_as_used( tiny_block_t * block )
{
    block->flags = 1;
}

static inline void mark_block_as_free( tiny_block_t * block )
{
    block->flags = 0;
}

static inline void * block_to_data( tiny_block_t * block )
{
    return (void *)((uint8_t *)block + sizeof(tiny_block_t));
}

static inline tiny_block_t * data_to_block( void * data )
{
    return (tiny_block_t *)((uint8_t *)data - sizeof(tiny_block_t));
}

static tiny_block_t * find_free_block( void* pool, int payload_size )
{
    tiny_block_t * block = get_first_block( pool );
    for (;;)
    {
        if ( block->next == NULL ) // last record has no any pointers
        {
            return NULL;
        }
        if ( is_free_block(block) && ( get_block_size( block ) >= payload_size + MIN_OVERHEAD ) )
        {
            break;
        }
        block = block->next;
    }
    return block;
}

static void split_block_internal( tiny_block_t * block, uint8_t* new_block_location )
{
    tiny_block_t * new_block = (tiny_block_t *)new_block_location;
    new_block->next = block->next;
    new_block->prev = block;
    block->next->prev = new_block;
    block->next = new_block;
    new_block->flags = block->flags;
}

static tiny_block_t * split_block_right( tiny_block_t * block, int size )
{
    tiny_block_t * new_block;
    if (get_block_size( block ) <= size + MIN_OVERHEAD )
    {
        /* do not split the block. We do not have space to place new block */
        new_block = block;
    }
    else
    {
        split_block_internal(block, MM_ALIGN_LEFT((uint8_t *)block->next - size) - sizeof(tiny_block_t));
        new_block = block->next;
    }
    return new_block;
}

static tiny_block_t * split_block_left( tiny_block_t * block, int size )
{
    if (get_block_size( block ) <= size + MIN_OVERHEAD )
    {
        /* do not split the block. We do not have space to place new block */
    }
    else
    {
        split_block_internal(block, MM_ALIGN_RIGHT((uint8_t *)block + size + sizeof(tiny_block_t)*2) -
                                     sizeof(tiny_block_t));
    }
    return block;
}

static inline void combine_with_next_block( tiny_block_t * block )
{
    // combine with next block
    block->next->next->prev = block;
    block->next = block->next->next;
}

static void combine_block( tiny_block_t * block )
{
    uint8_t flags = block->flags;
    if ( (block->prev != NULL) && (block->prev->flags == flags) )
    {
        // combine with previous block
        block->prev->next = block->next;
        block->next->prev = block->prev;
        block = block->prev;
    }
    if ((block->next->next != NULL) && (block->next->flags == flags))
    {
        // combine with next block
        combine_with_next_block( block );
    }
}


void mm_init( void * pool, int size )
{
    tiny_block_t * block = get_first_block( pool );
    block->next = (tiny_block_t *)((uint8_t *)pool + size - sizeof(tiny_block_t));
    block->next->next = NULL;
    block->next->prev = block;
    mark_block_as_free( block );
    mark_block_as_used( block->next );
}

void* mm_alloc( void* pool, int size )
{
    tiny_block_t * block = find_free_block( pool, size );
    if ( !block )
    {
        return NULL;
    }
    block = split_block_right( block, size );
    mark_block_as_used( block );
    return block_to_data( block );
}

void mm_free( void* pool, void* data )
{
    tiny_block_t * block = data_to_block( data );
    mark_block_as_free( block );
    combine_block( block );
}

void* mm_resize( void* pool, void* data, int new_size )
{
    tiny_block_t* block = data_to_block( data );
    int delta = new_size - get_block_data_size(block);
    if (delta < 0)
    {
        tiny_block_t * next_block = split_block_right( block, -delta - sizeof(tiny_block_t) );
        if (next_block != block)
        {
            mm_free( pool, next_block );
        } // else nothing to do here
    }
    else
    {
        tiny_block_t * next_block = block->next;
        if ( (next_block->next != NULL) &&
             is_free_block( next_block ) &&
             (get_block_data_size(next_block) >= delta ) )
        {
            int size = get_block_data_size(next_block);
            if (size > delta + sizeof(tiny_block_t))
            {
                // try to split block to leave some place for other buffers
                split_block_left( next_block, delta );
            }
            combine_with_next_block( block );
        }
        else
        {
            /* no way to resize block. Allocate new, copy data and delete old allocation */
            void * new_data = mm_alloc( pool, new_size );
            if ( new_data != NULL )
            {
                memcpy( new_data, data, get_block_data_size(block) );
                mm_free( pool, data );
            }
            return new_data;
        }
    }
    return data;
}

void* mm_resize_head( void* pool, void * data, int new_size )
{
    tiny_block_t * block = data_to_block( data );
    int delta = new_size - get_block_data_size(block);
    if (delta < 0)
    {
        tiny_block_t * next_block = split_block_right( block, new_size );
        if (next_block != block)
        {
            mm_free( pool, block );
        } // else nothing to do here
        return block_to_data( block );
    }
    else
    {
        tiny_block_t * prev_block = block->prev;
        if ( (prev_block != NULL) &&
             is_free_block( prev_block) &&
             (get_block_data_size(prev_block) >= delta ) )
        {
            tiny_block_t * next = block->next;
            uint8_t flags = block->flags;
            block = (tiny_block_t *)((uint8_t *)block - delta);
            prev_block->next = block;
            block->next = next;
            block->prev = prev_block;
            block->flags = flags;
            return block_to_data( block );
        }
        else
        {
            /* no way to resize block. Allocate new, copy data and delete old allocation */
            void * new_data = mm_alloc( pool, new_size );
            if ( new_data != NULL )
            {
                memcpy( (uint8_t *)new_data + delta, data, get_block_data_size(block) );
                mm_free( pool, data );
            }
            return new_data;
        }
    }
    return NULL;
}

int mm_free_size(void* pool)
{
    int total = 0;
    tiny_block_t * block = get_first_block(pool);
    for(;;)
    {
        if ( block->next == NULL )
        {
            break;
        }
        if (is_free_block(block))
        {
            total += get_block_data_size(block);
        }
        block = block->next;
    }
    return total;
}

void mm_print(void* pool)
{
    int i = 0;
    tiny_block_t * block = get_first_block(pool);
    for(;;)
    {
        if ( block->next == NULL )
        {
            break;
        }
        fprintf(stderr, "[%d] F:%02X, S:%d\n", i, block->flags, get_block_size(block));
        block = block->next;
        i++;
    }
/*    fprintf(stderr, "BACKWARD\n");
    for(;;)
    {
        block = block->prev;
        i--;
        fprintf(stderr, "[%d] F:%02X, S:%d\n", i, block->flags, get_block_size(block));
        if ( block->prev == NULL )
        {
            break;
        }
    } */
    fprintf(stderr, "=================\n");
}

