#pragma once

#define LIKELY(x)	__builtin_expect(!!(x), 1)
#define UNLIKELY(x)	__builtin_expect(!!(x), 0)

#define BIT(x)	(1 << (x))

static constexpr uint32_t ExtractBits(uint32_t *arr, uint start, uint n) {
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
