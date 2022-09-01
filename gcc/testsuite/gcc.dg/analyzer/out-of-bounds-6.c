/* { dg-additional-options "-Wno-unused-but-set-variable" } */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <stdint.h>

/* Tests with symbolic values.  */

void test1 (size_t size)
{
  char *buf = __builtin_malloc (size);
  if (!buf) return;

  buf[size] = '\0'; /* { dg-warning "" } */
  free (buf);
}

void test2 (size_t size)
{
  char *buf = __builtin_malloc (size);
  if (!buf) return;

  buf[size + 1] = '\0'; /* { dg-warning "" } */
  free (buf);
}

/* Test commutativeness.  */

void test3 (size_t size, size_t op)
{
  char *buf = __builtin_malloc (size);
  if (!buf) return;

  buf[size + op] = '\0'; /* { dg-warning "" } */
  buf[op + size] = '\0'; /* { dg-warning "" } */
  free (buf);
}

void test4 (size_t size, size_t op)
{
  char *buf = __builtin_malloc (op * size);
  if (!buf) return;

  buf[size * op] = '\0'; /* { dg-warning "" } */
  buf[op * size] = '\0'; /* { dg-warning "" } */
  free (buf);
}

/* Test where the offset itself is not out-of-bounds
   but multiple bytes are read.
   
   TODO: these three don't work because my current algo bails out if I do a
         binop expr of sym_bit_offset and num_bits_sval.  */

void test5 (size_t size)
{
  int32_t *buf = __builtin_alloca (4 * size + 3); /* { dg-warning "allocated buffer size is not a multiple of the pointee's size" } */
  buf[size] = 42; /* { dg-warning "" } */
}

void test6 (size_t size, size_t offset)
{
  char src[size];
  char dst[size];
  memcpy (dst, src, size + offset); /* { dg-warning "" } */
}

void test7 (size_t size, size_t offset)
{
  int32_t src[size];
  int32_t dst[size];
  memcpy (dst, src, 4 * size + 1); /* { dg-warning "" } */
}

/* Test for no false-positives.  */

void test9 (size_t size)
{
  int32_t *buf = __builtin_alloca (4 * size + 4);
  buf[size] = 42;
}

void test10 (size_t size, size_t offset)
{
  int buf[size];
  buf[offset] = 42;
}

void test11 (size_t size, int offset)
{
  int buf[size];
  /* We don't know whether offset is positive or not.  */
  buf[size + offset] = 42;
}

void test12 (size_t size, size_t offset, size_t offset2)
{
  int buf[size];
  /* We don't know whether offset > offset2.  */
  buf[size + offset - offset2] = 42;
}

void test13 (size_t a, size_t b)
{
  int buf[a * b];
  /* We can't reason about a*b < a+b either.  */
  buf[a + b] = 42;
}

/* Misc.  */

char *test99 (const char *x, const char *y)
{
  size_t len_x = __builtin_strlen (x);
  size_t len_y = __builtin_strlen (y);
  /* BUG (root cause): forgot to add 1 for terminator.  */
  size_t sz = len_x + len_y;
  char *result = __builtin_malloc (sz);
  if (!result)
    return NULL;
  __builtin_memcpy (result, x, len_x);
  __builtin_memcpy (result + len_x, y, len_y);
  /* BUG (symptom): off-by-one out-of-bounds write to heap.  */
  result[len_x + len_y] = '\0'; /* { dg-warning "" } */
  return result;
}
