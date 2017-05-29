#include <stdio.h>

#include "cvec.h"

/*
 * A simple container structure to hold a copy of the argument and its length.
 */
typedef struct
{
  char *str;
  size_t len;
} Argument;

/*
 * Create a type alias for cvec_t(Argument *) so we can pass this type to other
 * functions.
 */
typedef cvec_t(Argument *) ArgVector;

/*
 * Allocate memory for a new Argument type, store the contents of argv and its
 * length into it, and finally return a pointer to it.
 */
static Argument *
argumentNew(const char *argv)
{
  Argument *arg = malloc(sizeof(Argument));
  if (!arg)
  {
    fprintf(stderr, "error: malloc() failed for new Argument for \"%s\"\n",
            argv);
    exit(EXIT_FAILURE);
  }

  arg->len = strlen(argv);
  arg->str = strndup(argv, arg->len);
  if (!arg->str)
  {
    fprintf(stderr, "error: strdup() failed for \"%s\"\n", argv);
    free(arg);
    exit(EXIT_FAILURE);
  }
  return arg;
}

/*
 * Free all memory associated with this Argument variable.
 */
static void
argumentFree(Argument *arg)
{
  free(arg->str);
  free(arg);
}

/*
 * The callback function for cvec_foreach.
 * It just prints the string and its length. Also userdata is ignored here.
 */
static void
argumentPrint(const Argument *arg, void *userdata /* UNUSED */)
{
  printf("str=\"%s\" len=%zu\n", arg->str, arg->len);
}

/*
 * Iterate over each item contained in the vector and print it using the
 * callback above.
 */
static void
argVectorPrint(const ArgVector *v)
{
  cvec_foreach(*v, Argument *, argumentPrint, NULL);
}

int
main(int argc, char **argv)
{
  /*
   * If no command line arguments were provided, then just return from the
   * program.
   */
  if (argc == 1)
  {
    fprintf(stderr, "error: nothing to do, no arguments provided\n");
    return EXIT_FAILURE;
  }

  /*
   * Create and initialize a new vector variable for Argument* types. Note that
   * argumentFree is passed in here so that when we call cvec_free() below,
   * argumentFree() will be called on each item of the vector.
   */
  ArgVector v = CVEC_INIT(Argument *, NULL, argumentFree);

  /*
   * Reserve enough space ahead of time for exactly the number of arguments
   * provided from the command line. Note that this step is optional but always
   * recommended if you know how many items you'll have beforehand.
   */
  cvec_reserve(v, argc - 1);

  /*
   * Append a new Argument pointer for each argument provided from the command
   * line.
   */
  for (int i = 1; argv[i]; ++i)
  {
    cvec_push_back(v, argumentNew(argv[i]));
    /*
     * Check for out of memory errors.
     */
    if (cvec_had_error(v))
    {
      fprintf(stderr, "error: %s\n", cvec_strerror(v));
      return EXIT_FAILURE;
    }
  }

  /*
   * Print each item in the vector.
   */
  argVectorPrint(&v);

  /*
   * Free all memory that was allocated for this vector, as well as call
   * argumentFree() on each item in the vector.
   */
  cvec_free(v);
  return EXIT_SUCCESS;
}
