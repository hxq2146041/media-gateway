#ifndef GATEWAY_REFCOUNT_H
#define GATEWAY_REFCOUNT_H

#include "atomic.h"

#define gateway_refcount_containerof(refptr, type, member) \
	((type *)((char *)(refptr) - offsetof(type, member)))

typedef struct gateway_refcount gateway_refcount;
struct gateway_refcount {
	int count;
	void (*free)(const gateway_refcount *);
};

#define gateway_refcount_init(refp, free_fn) { \
	(refp)->count = 1; \
	(refp)->free = free_fn; \
}

#define gateway_refcount_increase(refp) { \
  atomic_int_inc((int *)&(refp)->count); \
}

#define gateway_refcount_decrease(refp) { \
	if(atomic_int_dec_and_test((int *)&(refp)->count)) { \
		(refp)->free(refp); \
	} \
}

#endif
