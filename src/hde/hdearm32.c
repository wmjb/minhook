/*
 * Hacker Disassembler Engine ARM32 (Thumb-2 subset)
 *
 * Minimal Thumb-2 decoder for Windows RT / WoA ARMv7 user-mode code.
 * Not a full ARM ISA decoder; only what we need for MinHook-style trampolines.
 */

#include <string.h>
#include "hdearm32.h"

static int is_thumb32(uint16_t hw1)
{
    /* Thumb-2 32-bit encodings:
       11101x, 11110x, 11111x (with some exclusions).
       We use the common heuristic: top 5 bits 11101/11110/11111 and bits[11:10] != 00.
    */
    if ((hw1 & 0xE000) == 0xE000 && (hw1 & 0x1800) != 0)
        return 1;
    return 0;
}

/* Sign-extend helper */
static int32_t sign_extend(int32_t value, int bits)
{
    int32_t shift = 32 - bits;
    return (value << shift) >> shift;
}

/*
 * Decode a single Thumb/Thumb-2 instruction at 'code'.
 * Returns instruction length in bytes (2 or 4).
 */
unsigned int hdearm32_disasm(const void *code, hdearm32s *hs)
{
    const uint8_t *p = (const uint8_t *)code;
    uint16_t hw1 = *(const uint16_t *)p;

    memset(hs, 0, sizeof(*hs));

    hs->opcode1 = (uint8_t)(hw1 & 0xFF);
    hs->flags   = 0;
    hs->imm     = 0;

    if (is_thumb32(hw1)) {
        /* 32-bit Thumb-2 */
        uint16_t hw2 = *(const uint16_t *)(p + 2);
        hs->len      = 4;
        hs->flags   |= HDEARM_F_32BIT;
        hs->opcode2  = (uint8_t)(hw2 & 0xFF);

        /* Detect B.W / BL (unconditional / link branches) */

        /* B.W / BL: 11110 S cond(4) | 10 J1 J2 imm10; second halfword 11 J1 J2 imm11 */
        /* We simplify and just detect the common encodings used by MSVC for B.W/BL. */

        if ((hw1 & 0xF800) == 0xF000 && (hw2 & 0xD000) == 0x9000) {
            /* BL / BLX (immediate) */
            hs->flags |= HDEARM_F_BRANCH | HDEARM_F_PC_REL;

            /* Decode immediate (T3/T4 forms) */
            uint32_t S   = (hw1 >> 10) & 1;
            uint32_t imm10 = hw1 & 0x03FF;
            uint32_t J1  = (hw2 >> 13) & 1;
            uint32_t J2  = (hw2 >> 11) & 1;
            uint32_t imm11 = hw2 & 0x07FF;

            uint32_t I1 = !(J1 ^ S);
            uint32_t I2 = !(J2 ^ S);

            uint32_t imm = (S << 24) |
                           (I1 << 23) |
                           (I2 << 22) |
                           (imm10 << 12) |
                           (imm11 << 1);

            hs->imm = sign_extend((int32_t)imm, 25);
        }
        else if ((hw1 & 0xF800) == 0xF000 && (hw2 & 0xD000) == 0x8000) {
            /* B.W (unconditional) */
            hs->flags |= HDEARM_F_BRANCH | HDEARM_F_PC_REL;

            uint32_t S     = (hw1 >> 10) & 1;
            uint32_t imm10 = hw1 & 0x03FF;
            uint32_t J1    = (hw2 >> 13) & 1;
            uint32_t J2    = (hw2 >> 11) & 1;
            uint32_t imm11 = hw2 & 0x07FF;

            uint32_t I1 = !(J1 ^ S);
            uint32_t I2 = !(J2 ^ S);

            uint32_t imm = (S << 24) |
                           (I1 << 23) |
                           (I2 << 22) |
                           (imm10 << 12) |
                           (imm11 << 1);

            hs->imm = sign_extend((int32_t)imm, 25);
        }
        else {
            /* For now, we treat all other 32-bit instructions as non-PC-relative. */
        }
    } else {
        /* 16-bit Thumb */
        hs->len = 2;

        /* Unconditional B (T2): 11100 imm11 */
        if ((hw1 & 0xF800) == 0xE000) {
            hs->flags |= HDEARM_F_BRANCH | HDEARM_F_PC_REL;

            uint32_t imm11 = hw1 & 0x07FF;
            uint32_t imm   = imm11 << 1;

            hs->imm = sign_extend((int32_t)imm, 12);
        }
        /* Conditional B (T1): 1101 cond imm8, cond != 1110/1111 */
        else if ((hw1 & 0xF000) == 0xD000 && ((hw1 & 0x0F00) != 0x0F00)) {
            hs->flags |= HDEARM_F_BRANCH | HDEARM_F_PC_REL;

            uint32_t imm8 = hw1 & 0x00FF;
            uint32_t imm  = imm8 << 1;

            hs->imm = sign_extend((int32_t)imm, 9);
        }
        /* CBZ/CBNZ: 1011 0 op imm5 Rn */
        else if ((hw1 & 0xF500) == 0xB100) {
            hs->flags |= HDEARM_F_BRANCH | HDEARM_F_PC_REL;

            uint32_t imm5 = (hw1 >> 3) & 0x1F;
            uint32_t imm  = imm5 << 1;

            hs->imm = sign_extend((int32_t)imm, 7);
        }
        else {
            /* All other 16-bit instructions: treated as non-PC-relative. */
        }
    }

    /* Basic sanity: len must be 2 or 4 */
    if (hs->len != 2 && hs->len != 4) {
        hs->flags |= HDEARM_F_ERROR;
        hs->len = 2;
    }

    return (unsigned int)hs->len;
}
