#ifndef __TLV_H
#define __TLV_H

#include <stdint.h>

extern unsigned int __infoa;
extern unsigned int __infob;
extern unsigned int __infoc;
extern unsigned int __infod;

template<typename CLOCK,
	unsigned int *BEGIN>
struct TLV_T {
	static unsigned verify_info_chk(void) {
		const unsigned *p = BEGIN + 1;                      // Begin at word after checksum
		unsigned chk = 0;                                   // Init checksum
		while (p < BEGIN + 64) chk ^= *p++;                 // XOR all words in segment
		return chk + *BEGIN;                                // Add checksum - result will be zero if checksum is valid
	};

	static void const *find_tag(const unsigned tag) {
		const unsigned *p = BEGIN + 1;                      // Begin at word after checksum
		do {                                                //
			const unsigned d = *p++;                        // Get a word
			if ((d & 0xFF) == tag) return (void *)p;        // Return pointer if tag matches
			p += (d >> 9);                                  // Point to next tag
		} while (p < BEGIN + 32);                               // While still within segment
		return 0;                                           // Not found, return NULL pointer
	};

	template<typename ITERATOR>
	static void iterate(void) {
		unsigned *p = BEGIN + 1;
		do {
			unsigned d = *p++;
			ITERATOR::handle_tag(d & 0xff, d >> 8, reinterpret_cast<void *>(p));
			p += (d >> 9);
		} while (p < BEGIN + 32);
	};

	static constexpr bool frequency_ok(const uint8_t divider) {
		return CLOCK::frequency / (unsigned long) divider > 257000L && CLOCK::frequency / (unsigned long) divider < 476000L;
	};

	static constexpr uint8_t calculate_divider(const uint8_t divider) {
		return frequency_ok(divider) ? divider : calculate_divider(divider - 1);
	};

	static void clear(void) {
		FCTL2 = FWKEY + FSSEL_2 + (calculate_divider(64) - 1);
	};
};

#endif
