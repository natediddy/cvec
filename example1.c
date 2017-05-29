/*
 * A small example program that demonstrates a simple usage of a cvec_t type.
 */

#include <stdio.h>

#include "cvec.h"

/*
 * Create a new type alias for cvec_t(int) so we can pass variables of this
 * type to other functions.
 */
typedef cvec_t(int) IntVector;

/*
 * This callback is used when cvec_foreach is called.
 * Here, userdata is only used to keep track of the index of each item so that
 * we can print it out.
 */
static void
printVecItemCallback(int item, void *userdata)
{
  int *index = (int *) userdata;
  printf("iv[%d]=%d\n", (*index)++, item);
}

/*
 * Iterates over each item contained in the vector and prints it. We also keep
 * track of the index and pass its address along to the callback above to be
 * printed and incremented.
 */
static void
printVec(const IntVector *iv)
{
  int index = 0;
  cvec_foreach(*iv, int, printVecItemCallback, &index);
}

int
main(int argc, char **argv)
{
  /*
   * Create and initialize a new vector variable.
   */
  IntVector iv = CVEC_INIT(int, -1, NULL);

  /*
   * Reserve space for 100 integers. This step is optional but nevertheless
   * recommended if you know how many items will be stored in the container
   * ahead of time.
   */
  cvec_reserve(iv, 100);

  /*
   * Append the integers 1-100 to the end of the vector.
   */
  for (int i = 1; i <= 100; ++i)
  {
    cvec_push_back(iv, i);
    /*
     * Check for out of memory errors.
     */
    if (cvec_had_error(iv))
    {
      fprintf(stderr, "error: %s\n", cvec_strerror(iv));
      return EXIT_FAILURE;
    }
  }

  /*
   * Print each item in the vector.
   */
  printVec(&iv);

  /*
   * Free all the memory allocated for this vector.
   */
  cvec_free(iv);

  return EXIT_SUCCESS;
}
