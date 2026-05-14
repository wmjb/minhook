#ifndef _HDEARM32_H_
#define _HDEARM32_H_

#include "pstdint.h"

/* Flags */
#define HDEARM_F_PC_REL     0x00000001  /* instruction uses PC-relative immediate */
#define HDEARM_F_BRANCH     0x00000002  /* control-flow change (B/BL/CBZ/...)    */
#define HDEARM_F_32BIT      0x00000004  /* instruction is 32-bit Thumb-2         */
#define HDEARM_F_ERROR      0x00000008  /* decode error                          */

/* For now we only care about a small subset of metadata */
#pragma pack(push,1)

typedef struct {
    uint8_t  len;        /* 2 or 4 bytes */
    uint8_t  opcode1;    /* first halfword (low 16 bits) */
    uint8_t  opcode2;    /* second halfword (if 32-bit; low 8 bits) */
    uint32_t flags;

    /* For PC-relative instructions, this is the *signed* immediate in bytes
       relative to the instruction's PC (architectural PC, not address of first halfword). */
    int32_t  imm;        /* in bytes */
} hdearm32s;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

/* __cdecl */
unsigned int hdearm32_disasm(const void *code, hdearm32s *hs);

#ifdef __cplusplus
}
#endif

#endif /* _HDEARM32_H_ */
