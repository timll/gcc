#include <stdlib.h>

/* CWE-131 example 5 */
void test_1(void) 
{
  int *id_sequence = (int*) malloc(3); /* { dg-line malloc } */
  /* { dg-warning "" "" { target *-*-* } malloc } */
  /* { dg-message "" "" { target *-*-* } malloc } */
  if (id_sequence == NULL) exit(1);

  id_sequence[0] = 13579;
  id_sequence[1] = 24680;
  id_sequence[2] = 97531;

  free (id_sequence);
}
