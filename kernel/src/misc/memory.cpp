#include <stddef.h>

extern "C" void* memset(void* dest, int value, size_t count) {
	auto* ptr = static_cast<unsigned char*>(dest);

	for (size_t i = 0; i < count; i++)
		ptr[i] = static_cast<unsigned char>(value);

	return dest;
}

extern "C" void* memcpy(void* dest, const void* src, size_t count) {
	auto* dst = static_cast<unsigned char*>(dest);
	auto* source = static_cast<const unsigned char*>(src);

	for (size_t i = 0; i < count; i++)
		dst[i] = source[i];

	return dest;
}