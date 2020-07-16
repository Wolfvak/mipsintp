#pragma once

#define LIKELY(x)	__builtin_expect(!!(x), 1)
#define UNLIKELY(x)	__builtin_expect(!!(x), 0)

#define BIT(x)	(1 << (x))

#include <cstdint>

static constexpr uint32_t ExtractBits(const uint32_t *arr, unsigned start, unsigned n) {
	uint32_t mask = (1ULL << n) - 1;
	unsigned off = start / 32;
	unsigned shift = start % 32;

	uint32_t ret = arr[off] >> shift;
	if ((n + shift) > 32)
		ret |= arr[off+1] << (32 - shift);
	return ret & mask;
}
