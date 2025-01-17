#include "../../utils.h"

struct RL78K0R // checkme: correct model/isa?
{
    u32 PC;
    u16 SP; // stack pointer
    u8 ES; // checkme: can this have high 4 bits set?
    u8 CS; // checkme: can this have high 4 bits set?
    union alignas(8) // general purpose registers
    {
        u8 R[8];
        struct
        {
            u8 A;
            u8 X;
            u8 B;
            u8 C;
            u8 D;
            u8 E;
            u8 H;
            u8 L;
        };
        u16 RP[4];
        struct
        {
            u16 AX;
            u16 BC;
            u16 DE;
            u16 HL;
        };
    } Bank[4];
    union
    {
        u8 PSW; // program status word
        struct
        {
            bool Carry : 1;
            bool ISPrio0 : 1; // in-service priority bit0
            bool ISPrio1 : 1; // in-service priority bit1
            bool RBankSel0 : 1; // register bank select bit 0
            bool AuxCarry : 1;
            bool RBankSel1 : 1; // register bank select bit 1
            bool Zero : 1;
            bool IRQEnable : 1; 
        };
    };
};
