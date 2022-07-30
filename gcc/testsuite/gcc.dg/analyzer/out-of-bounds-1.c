#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef void * (*memcpy_t) (void *dst, const void *src, size_t n);

static memcpy_t __attribute__((noinline))
get_memcpy (void)
{
  return memcpy;
}

void test1 (void)
{
  int id_sequence[3];

  /* Populate the id array. */
  id_sequence[0] = 123;
  id_sequence[1] = 234;
  id_sequence[2] = 345;
  id_sequence[3] = 456; /* { dg-line test1 }

  /* { dg-warning "" "warning" { target *-*-* } test1 } */
  /* { dg-message "" "note" { target *-*-* } test1 } */
}

void test2 (void)
{
  int n = 4;
  int arr[4];

  for (int i = n; i >= 0; i--)
    arr[i] = i; /* { dg-line test2 } */

  /* { dg-warning "" "warning" { target *-*-* } test2 } */
  /* { dg-message "" "note" { target *-*-* } test2 } */
}

void test3 (void)
{
  int n = 4;
  int arr[4];

  for (int i = n; i >= 0; i--)
    arr[i] = i; /* { dg-line test3 } */

  /* { dg-warning "" "warning" { target *-*-* } test3 } */
  /* { dg-message "" "note" { target *-*-* } test3 } */
}

void test4 (void)
{
  int *arr = malloc (4 * sizeof (int));
  if (!arr)
    return;
  
  int *last_el = arr + 3;
  *last_el = 3;

  free (arr);
}

void test5 (void)
{
  int *arr = malloc (4 * sizeof (int));
  if (!arr)
    return;
  
  int *last_el = arr + 4;
  *last_el = 4; /* { dg-line test5 } */

  free (arr);
  /* { dg-warning "" "warning" { target *-*-* } test5 } */
  /* { dg-message "" "note" { target *-*-* } test5 } */
}

// extern int is_valid (void);

// int returnChunkSize (void *ptr) {

// /* if chunk info is valid, return the size of usable memory,
//    else, return -1 to indicate an error.  */
//   return is_valid () ? sizeof (*ptr) : -1;
// }

// int test66 (void) {
//   int destBuf[4];
//   int srcBuf[4];
//   memcpy (destBuf, srcBuf, returnChunkSize (destBuf) - 1);
// }
