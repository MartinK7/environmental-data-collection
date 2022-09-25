
#ifndef CC_H
#define CC_H

#define LWIP_NO_STDINT_H     0
#include <stdint.h>

#define LWIP_NO_INTTYPES_H   0
#include <inttypes.h>

#define LWIP_TIMEVAL_PRIVATE 0
#include <sys/time.h>

#define LWIP_PROVIDE_ERRNO


// IP protocols use checksums (see RFC 1071). LwIP gives you a choice of 3 algorithms:
//
// 1: load byte by byte, construct 16 bits word and add: not efficient for most platforms
// 2: load first byte if odd address, loop processing 16 bits words, add last byte.
// 3: load first byte and word if not 4 byte aligned, loop processing 32 bits words, add last word/byte.
#define LWIP_CHKSUM_ALGORITHM 2

//
// Data Packing
//

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

//
// Debug
//

#ifndef SIMULATOR
// non-fatal, print a message
#define LWIP_PLATFORM_DIAG(x)

// fatal, print message and abandon execution
extern int printf(const char *__restrict, ...);
#include "stm32h7xx.h"
#define LWIP_PLATFORM_ASSERT(x) do { printf("LWIP ASSERT: '%s'\r\n", x); for(volatile uint32_t i = 0; i < 100000; ++i) {__NOP();} NVIC_SystemReset(); } while(0)
#else
#include <assert.h>
#define LWIP_PLATFORM_DIAG(x) printf("LWIP: '%s'\r\n")
#define LWIP_PLATFORM_ASSERT(x) assert(x)
#if defined(WIN32) || defined(WIN64)
#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS
#endif
#endif

//
// Rand
//

#include <stdlib.h>
#define LWIP_RAND() ((u32_t)rand())

#endif // CC_H
