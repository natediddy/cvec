/*
 * MIT License
 *
 * Copyright (c) 2017 Nathan Forbes
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*
 * The following are examples of a few different ways that you can declare
 * cvec_t types. They all use a hypothetical string type that would be
 * implemented on top of a cvec_t.
 *
 * Example 1:
 *    typedef cvec_t(char) string_t;
 *    string_t s1 = CVEC_INIT(char, '\0', NULL);
 *    ...
 *    cvec_free(s1);
 *
 * Example 2:
 *    cvec_t(char) s2 = CVEC_INIT(char, '\0', NULL);
 *    ...
 *    cvec_free(s2);
 *
 * Example 3:
 *    typedef cvec_t(char) string_t;
 *    string_t s;
 *    cvec_init(s, char, '\0', NULL);
 *    ...
 *    cvec_free(s);
 */

#ifndef __CVEC_H__
#define __CVEC_H__

#include <stdlib.h>
#include <string.h>

#define CVEC_EOK 0
#define CVEC_EOOM -1 /* Out Of Memory */

/* A private macro that is undefined later */
#define __cvec_maybe_grow(__v)                                                \
  if ((__v).__n == (__v).__m)                                                 \
  {                                                                           \
    if ((__v).__m)                                                            \
      (__v).__m <<= 1;                                                        \
    else                                                                      \
      (__v).__m = 2;                                                          \
    (__v).__data = realloc((__v).__data, (__v).__t * ++(__v).__m);            \
    if (!(__v).__data)                                                        \
    {                                                                         \
      (__v).__e = CVEC_EOOM;                                                  \
      break;                                                                  \
    }                                                                         \
  }

/*
 * cvec_iter_t: Returns the type of iterator for a vector of type __T.
 *
 * __T: The type of items a vector may hold.
 */
#define cvec_iter_t(__T) __T *

/*
 * cvec_t: Declare a new vector type.
 *
 * __T: The type of items that the vector contains.
 */
#define cvec_t(__T)                                                           \
  struct                                                                      \
  {                                                                           \
    size_t __n;                                                               \
    size_t __m;                                                               \
    size_t __t;                                                               \
    __T *__data;                                                              \
    void (*__on_free)(__T);                                                   \
    int __e;                                                                  \
    __T __sentinel;                                                           \
  }

/*
 * CVEC_INIT: Initializes all the fields of the vector struct.
 *
 * __T:              The type of items that a vector contains.
 * __sentinel_value: Some default value that can be used
 *                   as a fallback in case of an error.
 * __on_free:        A function to be called on an item of the
 *                   vector when it is destroyed.
 */
#define CVEC_INIT(__T, __sentinel_value, __on_free)                           \
  {                                                                           \
    0, 0, sizeof(__T), NULL, (__on_free), CVEC_EOK, (__sentinel_value)        \
  }

/*
 * cvec_init: Initializes all the fields of the vector struct.
 *
 * __v:              The vector to initialize.
 * __T:              The type of items that the vector contains.
 * __sentinel_value: A default value that can be used
 *                   as a fallback in case of an error.
 * __on_free:        A function to be called on an item of the
 *                   vector when it is destroyed.
 */
#define cvec_init(__v, __T, __sentinel_value, __on_free)                      \
  do                                                                          \
  {                                                                           \
    (__v).__n = 0;                                                            \
    (__v).__m = 0;                                                            \
    (__v).__t = sizeof(__T);                                                  \
    (__v).__data = NULL;                                                      \
    (__v).__on_free = (__on_free);                                            \
    (__v).__e = CVEC_EOK;                                                     \
    (__v).__sentinel = (__sentinel_value);                                    \
  } while (0)

/*
 * cvec_set_on_free: Set the function to be called when an item is destroyed.
 *
 * __v:       The vector.
 * __on_free: A function to be called on an item of the
 *            vector when it is destroyed.
 */
#define cvec_set_on_free(__v, __on_free) (__v).__on_free = (__on_free)

/*
 * cvec_free: Deallocates all the memory associated with this vector.
 *
 * __v: The vector.
 *
 * If __on_free is not null, then it is called on each item in the vector.
 */
#define cvec_free(__v)                                                        \
  do                                                                          \
  {                                                                           \
    if ((__v).__on_free && (__v).__data)                                      \
      for (size_t __i = 0; __i < (__v).__n; ++__i)                            \
        (__v).__on_free((__v).__data[__i]);                                   \
    free((__v).__data);                                                       \
    (__v).__data = NULL;                                                      \
    (__v).__n = 0;                                                            \
    (__v).__m = 0;                                                            \
    (__v).__e = CVEC_EOK;                                                     \
  } while (0)

/*
 * cvec_reserve: Reserve memory ahead of time.
 *
 * __v: The vector.
 * __n: The number of items to allocate space for.
 */
#define cvec_reserve(__v, __n)                                                \
  do                                                                          \
  {                                                                           \
    (__v).__data = realloc((__v).__data, (__v).__t * (__n));                  \
    if (!(__v).__data)                                                        \
      (__v).__e = CVEC_EOOM;                                                  \
    (__v).__m = (__n);                                                        \
  } while (0)

/*
 * cvec_clear: Set the size of the vector to zero.
 *
 * __v: The vector.
 *
 * If __on_free is not null, then it is called on each item currently in the
 * vector. Also note that the capacity of the vector is left unchanged.
 */
#define cvec_clear(__v)                                                       \
  do                                                                          \
  {                                                                           \
    if ((__v).__on_free)                                                      \
      for (size_t __i = 0; __i < (__v).__n; ++i)                              \
        (__v).__on_free((__v).__data[__i]);                                   \
    (__v).__n = 0;                                                            \
  } while (0)

/*
 * cvec_insert: Insert an item into a vector
 *
 * __v:    The vector.
 * __pos:  The position to insert at.
 * __item: The item to insert.
 */
#define cvec_insert(__v, __pos, __item)                                       \
  do                                                                          \
  {                                                                           \
    size_t __p = (__pos);                                                     \
    __cvec_maybe_grow(__v);                                                   \
    memcpy((__v).__data + (__p + 1), (__v).__data + __p,                      \
           (__v).__t * ((__v).__n++ - __p));                                  \
    (__v).__data[__p] = (__item);                                             \
  } while (0)

/*
 * cvec_erase: Erase an item in a vector.
 *
 * __v:   The vector.
 * __pos: The position of the item to erase.
 */
#define cvec_erase(__v, __pos)                                                \
  do                                                                          \
  {                                                                           \
    if ((__v).__n > 0)                                                        \
    {                                                                         \
      size_t __p = (__pos);                                                   \
      if ((__v).__on_free)                                                    \
        (__v).__on_free((__v).__data[__p]);                                   \
      memcpy((__v).__data + __p, (__v).__data + (__p + 1),                    \
             (__v).__t * ((__v).__n-- - (__p - 1)));                          \
    }                                                                         \
  } while (0)

/*
 * cvec_erase_n: Erase a number of items in a vector.
 *
 * __v:     The vector.
 * __pos:   The position of the first item to erase.
 * __count: The total number of items to erase.
 *
 * Erases everything from __pos to __pos + __count.
 */
#define cvec_erase_n(__v, __pos, __count)                                     \
  do                                                                          \
  {                                                                           \
    size_t __p = (__pos);                                                     \
    if (__p == 1)                                                             \
      cvec_erase(__v, __p);                                                   \
    else if (__p > 1 && (__v).__n > 0)                                        \
    {                                                                         \
      size_t __c = (__count);                                                 \
      size_t __t = 0;                                                         \
      size_t __x;                                                             \
      for (__x = __p; __t < __c && __x < (__v).__n; ++__t, ++__x)             \
        ;                                                                     \
      if ((__v).__on_free)                                                    \
      {                                                                       \
        size_t __stop = __p + __t;                                            \
        for (__x = __p; __x < __stop; ++__x)                                  \
          (__v).__on_free((__v).__data[__x]);                                 \
      }                                                                       \
      memcpy((__v).__data + __p, (__v).__data + (__p + __t),                  \
             (__v).__t * ((__v).__n - (__p - __t)));                          \
      (__v).__n -= __t;                                                       \
    }                                                                         \
  } while (0)

/*
 * cvec_begin: Returns an iterator to the beginning of a vector.
 *
 * __v: The vector.
 */
#define cvec_begin(__v) (__v).__data

/*
 * cvec_end: Returns an iterator to the end of a vector.
 *
 * __v: The vector.
 */
#define cvec_end(__v) (__v).__data + (__v).__n

/*
 * cvec_front: Returns the first item in a vector.
 *
 * __v: The vector.
 *
 * If the vector is empty, this will return the sentinel value provided during
 * initialization.
 */
#define cvec_front(__v) ((__v).__n > 0) ? *cvec_begin(__v) : (__v).__sentinel

/*
 * cvec_back: Returns the last item in a vector.
 *
 * __v: The vector.
 *
 * If the vector is empty, this will return the sentinel value provided during
 * initialization.
 */
#define cvec_back(__v)                                                        \
  ((__v).__n > 0) ? *(cvec_end(__v) - 1) : (__v).__sentinel

/*
 * cvec_push_front: Insert item at the beginning of a vector.
 *
 * __v:    The vector.
 * __item: The item to insert.
 */
#define cvec_push_front(__v, __item)                                          \
  do                                                                          \
  {                                                                           \
    __cvec_maybe_grow(__v);                                                   \
    memcpy(cvec_begin(__v) + 1, (__v).__data, (__v).__t * (__v).__n++);       \
    *(__v).__data = (__item);                                                 \
  } while (0)

/*
 * cvec_push_back: Insert item at the end of a vector.
 *
 * __v:    The vector.
 * __item: The item to insert.
 */
#define cvec_push_back(__v, __item)                                           \
  do                                                                          \
  {                                                                           \
    __cvec_maybe_grow(__v);                                                   \
    *((__v).__data + (__v).__n++) = (__item);                                 \
  } while (0)

#if 0
#define cvec_push_back(__v, __item)                                           \
  do                                                                          \
  {                                                                           \
    __cvec_maybe_grow(__v);                                                   \
    (__v).__data[(__v).__n++] = (__item);                                     \
  } while (0)
#endif

/*
 * cvec_pop_front: Remove item from the beginning of a vector.
 *
 * __v: The vector.
 */
#define cvec_pop_front(__v)                                                   \
  do                                                                          \
  {                                                                           \
    if ((__v).__n > 0)                                                        \
    {                                                                         \
      if ((__v).__on_free)                                                    \
        (__v).__on_free(cvec_front(__v));                                     \
      memcpy(cvec_begin(__v), cvec_begin(__v) + 1, (__v).__t * --(__v).__n);  \
    }                                                                         \
  } while (0)

/*
 * cvec_pop_back: Remove item from the end of a vector.
 *
 * __v: The vector.
 */
#define cvec_pop_back(__v)                                                    \
  do                                                                          \
  {                                                                           \
    if ((__v).__n > 0)                                                        \
    {                                                                         \
      if ((__v).__on_free)                                                    \
        (__v).__on_free(cvec_back(__v));                                      \
      --(__v).__n;                                                            \
    }                                                                         \
  } while (0)

#if 0
#define cvec_pop_back(__v)                                                    \
  do                                                                          \
  {                                                                           \
    if ((__v).__n > 0)                                                        \
    {                                                                         \
      if ((__v).__on_free)                                                    \
        (__v).__on_free((__v).__data[(__v).__n - 1]);                         \
      --(__v).__n;                                                            \
    }                                                                         \
  } while (0)
#endif

/*
 * cvec_shrink_to_fit: Shrink the capacity of a vector to the minimum required.
 *
 * __v: The vector.
 */
#define cvec_shrink_to_fit(__v)                                               \
  do                                                                          \
  {                                                                           \
    if ((__v).__m > (__v).__n)                                                \
    {                                                                         \
      (__v).__data = realloc((__v).__data, (__v).__t * (__v).__n);            \
      if (!(__v).__data)                                                      \
        (__v).__e = CVEC_EOOM;                                                \
      (__v).__m = (__v).__n;                                                  \
    }                                                                         \
  } while (0)

/*
 * cvec_empty: Returns whether or not the vector is empty.
 *
 * __v: The vector.
 */
#define cvec_empty(__v) ((__v).__n == 0)

/*
 * cvec_foreach: Iterates over a vector and performs an action on each item.
 *
 * __v:        The vector.
 * __T:        The type of the items contained in the vector.
 * __fun:      A callback function to be called on each item with the
 *             following signature:
 *                 void <func>(__T item, void *userdata);
 * __userdata: Any userdata to be passed along to the callback function.
 */
#define cvec_foreach(__v, __T, __fun, __userdata)                             \
  do                                                                          \
  {                                                                           \
    cvec_iter_t(__T) __e = cvec_end(__v);                                     \
    for (cvec_iter_t(__T) __i = cvec_begin(__v); __i != __e; ++__i)           \
      __fun(*__i, (__userdata));                                              \
  } while (0)

/*
 * cvec_size: Returns the total number of items in the vector.
 *
 * __v: The vector.
 */
#define cvec_size(__v) (cvec_end(__v) - cvec_begin(__v))

/*
 * cvec_cap: Returns the maximum capacity of the vector.
 *
 * __v: The vector.
 */
#define cvec_cap(__v) (__v).__m

/*
 * cvec_get: Returns the item at a specific position in a vector.
 *
 * __v: The vector.
 * __i: The position of the requested item.
 *
 * Note that no bounds checking is performed here.
 */
#define cvec_get(__v, __i) *(cvec_begin(__v) + (__i))

/*
 * cvec_at: Returns the item at a specific position in a vector.
 *
 * __v: The vector.
 * __i: The position of the requested item.
 *
 * Note that if __i is out of bounds then the sentinel value provided earlier
 * will be returned.
 */
#define cvec_at(__v, __i)                                                     \
  ((__i) < (__v).__n) ? *(cvec_begin(__v) + (__i)) : (__v).__sentinel

/*
 * cvec_had_error: Returns whether or not an error has recently occurred.
 *
 * __v: The vector.
 */
#define cvec_had_error(__v) ((__v).__e != CVEC_EOK)

/*
 * cvec_error: Returns the most recent error.
 *
 * __v: The vector.
 */
#define cvec_error(__v) (__v).__e

/*
 * cvec_strerror: Returns a string describing the most recent error status.
 *
 * __v: The vector.
 */
#define cvec_strerror(__v)                                                    \
  ((__v).__e == CVEC_EOOM) ? "Out of memory" : "No error"

#endif /* __CVEC_H__ */
