/* { dg-additional-options "-Wno-analyzer-out-of-bounds -Wno-analyzer-use-of-uninitialized-value" } */
#include "analyzer-decls.h"

typedef __SIZE_TYPE__ size_t;

#define NULL ((void *)0)

extern void *malloc (size_t __size)
  __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__malloc__))
  __attribute__ ((__alloc_size__ (1)));
extern void *realloc (void *__ptr, size_t __size)
  __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__warn_unused_result__))
  __attribute__ ((__alloc_size__ (2)));
extern void free (void *__ptr)
  __attribute__ ((__nothrow__ , __leaf__));
extern void *memset (void *__ptr, int __value, size_t __size);

/* realloc where the region shrinks on success_with_move.  */

void test_1 ()
{
  char *p = malloc (16);
  if (!p)
    return;
  memset (p, 1, 16);

  char *q = realloc (p, 8);
  if (!q)
    {
      free (p);
      return;
    }
  else if (p != q)
    __analyzer_eval (q[8] == 1); /* { dg-warning "UNKNOWN" } */
  
  free (q);
}
