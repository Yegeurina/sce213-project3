/**********************************************************************
 * Copyright (c) 2020
 *  Jinwoo Jeong <jjw8967@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdint.h>

#include "malloc.h"
#include "types.h"
#include "list_head.h"

#define ALIGNMENT 32
#define HDRSIZE sizeof(header_t)
#define __ROUND_UP(x,shift) ((((x) >> shift) + (((x) & ((1<<shift)-1)) != 0)) << shift)

static LIST_HEAD(free_list); // Don't modify this line
static algo_t g_algo;        // Don't modify this line
static void *bp;             // Don't modify thie line

/***********************************************************************
 * extend_heap()
 *
 * DESCRIPTION
 *   allocate size of bytes of memory and returns a pointer to the
 *   allocated memory.
 *
 * RETURN VALUE
 *   Return a pointer to the allocated memory.
 */
void *my_malloc(size_t size)
{
  header_t *header = NULL;
  header_t *temp = NULL;
  bool flag = true;

  size = __ROUND_UP(size, 5);

  // printf("size : %ld\n",size);
  //printf("%p  ",sbrk(0));

  /* Implement this function */
  if (g_algo == 0) // First_fit
  {
    list_for_each_entry(header,&free_list, list)
    {
      if(header->free && header->size>=size)
      {
        if(header->size-size<=HDRSIZE)
        {
          header->free = false;
        }
        else
        {
          temp = header+(size+HDRSIZE)/HDRSIZE;
          temp->size = header->size-size-HDRSIZE;
          temp->free = true;

          header->size = size;
          header->free = false;

          list_add(&temp->list,&header->list);

          return header;
        }
        flag = false;
        break;
      }
      temp = header;
    } 
    if(flag && (temp==NULL || !temp->free))
    {
      header = sbrk(size+HDRSIZE);
      header->size = size;
      header->free = false;
      list_add_tail(&header->list, &free_list);
    }
    else if(flag && temp->free)
    {
      //printf("Here!\n");
      header = sbrk(size+HDRSIZE);
      header->size = temp->size;
      header->free = true;
      
      temp->size = size;
      temp->free = false;
      list_add_tail(&header->list,&free_list);
      
      return temp;
    }
  }
  else if(g_algo == 1) //Best_fit
  {
    header_t *ptr = NULL;
    size_t temp_size;
    list_for_each_entry(header,&free_list,list)
    {
      if(header->free)
      {
        if(ptr==NULL || temp_size>(header->size-size))
        {
          temp_size = (header->size-size);
          ptr =  header;
        }
      }
    }

    list_for_each_entry(header,&free_list, list)
    {
      if(ptr==header)
      {
        if(header->size-size<=HDRSIZE)
        {
          header->free = false;
        }
        else
        {
          temp = header+(size+HDRSIZE)/HDRSIZE;
          temp->size = header->size-size-HDRSIZE;
          temp->free = true;

          header->size = size;
          header->free = false;

          list_add(&temp->list,&header->list);

          return header;  
        }
        flag = false;
        break;
      }
      temp = header;
    }
    if(flag && (temp==NULL || !temp->free))
    {
      header = sbrk(size+HDRSIZE);
      header->size = size;
      header->free = false;
      list_add_tail(&header->list, &free_list);
    }
    else if(flag && temp->free)
    {
      //printf("Here!\n");
      header = sbrk(size+HDRSIZE);
      header->size = temp->size;
      header->free = true;
      
      temp->size = size;
      temp->free = false;
      list_add_tail(&header->list,&free_list);

      return temp;
    }
  }

  return header;
}

/***********************************************************************
 * my_realloc()
 *
 * DESCRIPTION
 *   tries to change the size of the allocation pointed to by ptr to
 *   size, and returns ptr. If there is not enough memory block,
 *   my_realloc() creates a new allocation, copies as much of the old
 *   data pointed to by ptr as will fit to the new allocation, frees
 *   the old allocation.
 *
 * RETURN VALUE
 *   Return a pointer to the reallocated memory
 */
void *my_realloc(void *ptr, size_t size)
{
  /* Implement this function */

  return NULL;
}

/***********************************************************************
 * my_realloc()
 *
 * DESCRIPTION
 *   deallocates the memory allocation pointed to by ptr.
 */
void my_free(void *ptr)
{
  // printf("ptr = %p \n",ptr);

  header_t *cursor, *header, *temp=NULL;
  list_for_each_entry(header,&free_list, list)
  {
    if(header == ptr)
    {
      header->free = true;
      break;
    }
  }

  list_for_each_entry_reverse(cursor, &free_list,list)
  {
    if(cursor->free && temp!=NULL && temp->free)
    {
      cursor->size += temp->size+ALIGNMENT;
      list_del(&temp->list);
    }
    temp = cursor;
  }

  return;
}

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING BELOW THIS LINE ******      */
/*          ****** BUT YOU MAY CALL SOME IF YOU WANT TO.. ******      */
/*          ****** EXCEPT TO mem_init() AND mem_deinit(). ******      */
void mem_init(const algo_t algo)
{
  g_algo = algo;
  bp = sbrk(0);
}

void mem_deinit()
{
  header_t *header;
  size_t size = 0;
  list_for_each_entry(header, &free_list, list) {
    size += HDRSIZE + header->size;
  }
  sbrk(-size);

  if (bp != sbrk(0)) {
    fprintf(stderr, "[Error] There is memory leak\n");
  }
}

void print_memory_layout()
{
  header_t *header;
  int cnt = 0;

  printf("===========================\n");
  list_for_each_entry(header, &free_list, list) {
    cnt++;
    //printf("%p  ",header);
    printf("%c %ld\n" , (header->free) ? 'F' : 'M', header->size);
  }

  printf("Number of block: %d\n", cnt);
  printf("===========================\n");
  return;
}
