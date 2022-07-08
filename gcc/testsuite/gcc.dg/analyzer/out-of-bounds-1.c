/* Tests for stack-based out-of-bounds warnings. */

void test1 ()
{
  int buf[4];
  for (int i = 0; i < 4; i++)
    buf[i] = 42;
}

void test2 ()
{
  int buf[4]; /* { dg-message } */
  for (int i = 0; i <= 4; i++) /* { dg-message } */
    buf[i] = 42; /* { dg-warning } */
}

void test3 ()
{
  int n = 4;
  int buf[n];
  for (int i = 0; i < n; i++)
    buf[i] = 42;
}

void test4 ()
{
  int n = 4;
  int buf[n]; /* { dg-message } */
  for (int i = 0; i <= n; i++) /* { dg-message } */
    buf[i] = 42; /* { dg-warning } */
}
