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

#pragma once

#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** Specifies memory alignment */
#ifdef __AVR__
#define MM_ALIGNMENT  1
#else
#define MM_ALIGNMENT  4
#endif

/**
 * @brief initializes memory pool
 *
 * Initializes memory pool
 * @param pool pointer to buffer to use by memory manager
 * @param size size of buffer
 */
void mm_init( void * pool, int size );

/**
 * @brief Allocates memory block in pool
 *
 * Allocates memory block in pool. Each allocation needs
 * at least additional 5 bytes for service purposes.
 *
 * @param pool pointer to buffer to use by memory manager
 * @param size size of block to allocate
 * @return pointer to allocated block
 */
void* mm_alloc( void* pool, int size );

/**
 * @brief Free memory block in pool
 *
 * Free memory block, allocated earlier in pool
 * @param pool pointer to buffer to use by memory manager
 * @param data pointer to block, allocated by mm_alloc()
 */
void mm_free( void* pool, void * data );

/**
 * @brief Resizes memory block in pool
 *
 * Resizes memory block in pool. Depending of pool fragmentation
 * the function can move block to new location. All user data
 * will be copied to new place. If block increase is requested
 * mm_resize() adds new data to the end of the user data.
 * If resize is not possible due memory state, the function returns
 * NULL. Block passed by a user, still remains.
 *
 * @param pool pointer to buffer to use by memory manager
 * @param data pointer to block, allocated by mm_alloc()
 * @param new_size new size of the block
 * @return pointer to old or newly allocated block
 */
void* mm_resize( void* pool, void * data, int new_size );

/**
 * @brief Resizes memory block in pool
 *
 * Resizes memory block in pool. Unlike mm_resize(), this
 * function adds new data before user data. If resize operation
 * cannot be performed, the function returns NULL.
 * Block passed by a user, still remains.
 *
 * @param pool pointer to buffer to use by memory manager
 * @param size size of block to allocate
 * @return pointer to allocated block
 */
void* mm_resize_head( void* pool, void * data, int new_size );

/**
 * @brief Returns total free size in memory pool
 *
 * Returns total free size in memory pool
 * @param pool pointer to buffer to use by memory manager
 * @return total free size in memory pool in bytes
 */
int mm_free_size(void* pool);

/**
 * @brief Prints debug information
 *
 * Prints debug information, containing current memory state.
 * @param pool pointer to buffer to use by memory manager
 */
void mm_print(void* pool);

#ifdef __cplusplus
}
#endif
