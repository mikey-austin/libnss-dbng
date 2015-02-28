/**
 * @file utils.h
 * @brief Common support utilities.
 * @author Mikey Austin
 * @date 2015
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <err.h>

extern void *xmalloc(size_t size);
extern void xfree(void **p);

#endif
