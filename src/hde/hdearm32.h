/*
 * Hacker Disassembler Engine ARM32 (Thumb-2 subset)
 * Minimal decoder for Windows ARM32 (Thumb-2) used by MinHook.
 */

#pragma once

#include "pstdint.h"

/* Flags */
#define HDEARM_F_PC_REL     0x00000001  /* Instruction uses PC-relative immediate */
#define HDEARM_F_BRANCH     0x00000002  /* Control-flow change (B/BL/CBZ/...)    */
#define HDEARM_F_32BIT      0x00000004  /* Instruction is 32-bit Thumb-2         */
#define HDEARM_F_ERROR      0x00000008  /* Decode error                          */

#pragma pack(push,1)

typedef struct {
    uint8_t  len;        /* Instruction length: 2 or 4 bytes */
    uint8_t  opcode1;    /* First halfword (low 8 bits only, for debugging) */
    uint8_t  opcode2;    /* Second halfword (low 8 bits only, if 32-bit)    */
    uint32_t flags;      /* HDEARM_F_* flags */
    int32_t  imm;        /* Signed PC-relative immediate (in bytes) */
} hdearm32s;

#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif

unsigned int hdearm32_disasm(const void *code, hdearm32s *hs);

#ifdef __cplusplus
}
#endif
