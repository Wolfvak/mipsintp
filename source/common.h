#pragma once

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define INLINE __attribute__((always_inline))
#define NOINLINE __attribute__((noinline))
#define NORETURN __attribute__((noreturn))

#define LIKELY(x)	__builtin_expect(!!(x), 1)
#define UNLIKELY(x)	__builtin_expect(!!(x), 0)

typedef unsigned int uint;

#define BIT(x)	(1 << (x))

static inline uint32_t ExtractBits(uint32_t *arr, uint start, uint n)
{
	uint32_t ret, mask;
	uint off, shift;

	mask = (1ULL << n) - 1;
	off = start / 32;
	shift = start % 32;

	ret = arr[off] >> shift;
	if ((n + shift) > 32)
		ret |= arr[off+1] << (32 - shift);
	return ret & mask;
}
