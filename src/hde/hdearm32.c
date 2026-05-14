/*
 * Hacker Disassembler Engine ARM32 (Thumb-2 subset)
 * Minimal Thumb/Thumb-2 decoder for MinHook on Windows ARM32.
 */

#include <string.h>
#include "hdearm32.h"

/* Detect Thumb-2 32-bit instruction */
static int is_thumb32(uint16_t hw1)
{
    /* Thumb-2 encodings:
       Top 5 bits = 11101, 11110, or 11111
       AND bits[11:10] != 00
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

unsigned int hdearm32_disasm(const void *code, hdearm32s *hs)
{
    const uint8_t *p = (const uint8_t *)code;
    uint16_t hw1 = *(const uint16_t *)p;

    memset(hs, 0, sizeof(*hs));

    hs->opcode1 = (uint8_t)(hw1 & 0xFF);
    hs->flags   = 0;
    hs->imm     = 0;

    /* 32-bit Thumb-2 */
    if (is_thumb32(hw1)) {
        uint16_t hw2 = *(const uint16_t *)(p + 2);

        hs->len     = 4;
        hs->flags  |= HDEARM_F_32BIT;
        hs->opcode2 = (uint8_t)(hw2 & 0xFF);

        /* Detect BL / BLX immediate */
        if ((hw1 & 0xF800) == 0xF000 && (hw2 & 0xD000) == 0x9000) {
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
        /* Detect B.W */
        else if ((hw1 & 0xF800) == 0xF000 && (hw2 & 0xD000) == 0x8000) {
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
    }
    else {
        /* 16-bit Thumb */
        hs->len = 2;

        /* Unconditional B (T2): 11100 imm11 */
        if ((hw1 & 0xF800) == 0xE000) {
            hs->flags |= HDEARM_F_BRANCH | HDEARM_F_PC_REL;

            uint32_t imm11 = hw1 & 0x07FF;
            uint32_t imm   = imm11 << 1;

            hs->imm = sign_extend((int32_t)imm, 12);
        }
        /* Conditional B (T1): 1101 cond imm8 */
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
    }

    /* Sanity */
    if (hs->len != 2 && hs->len != 4) {
        hs->flags |= HDEARM_F_ERROR;
        hs->len = 2;
    }

    return hs->len;
}
