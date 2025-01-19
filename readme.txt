VitaminC is an experimental fully LLE (new)3DS research emulator that targets extremely high accuracy!

...
At least in theory.

Doesn't even get past the bootrom yet--



Build Pre-Reqs:
    Some way to parse a makefile and compile C. (make? msys2? IDK you can probably google it for your OS.)
    A compiler that supports C23; preferably clang.

Building:
    1. Enter VitaminC folder
    2. Type "make" into a terminal.
    3. Profit
    4. (Optional) Open a github issue if you can't figure it out/it doesn't work.

Tested and developed on Linux.
Probably works on every platform, there's not actually any platform specific code currently.



Emulation Setup:
    1. Requires a 3DS ARM11 bios named `bios_ctr11.bin` (it's case-sensitive) located in the same directory as the executable to do anything of value. (it's an lle emu! We need bios files!)



Credits:
    Jakly:
        Lead dev; Author; Artist(?); Creator; Uhhh... What other nonsense accolades can I give myself---

    Arisotura & The melonDS Team (I guess that technically includes me too now?):
        This emulator's design was heavily influenced and inspired by the design of melonDS.
        Not to mention that it provided a great resource and reference for validating the implementations of certain ARM instructions.

    burhanr13:
        Offered their makefile. (I yoinked a couple lines from it.) (Shoutouts Tanuki3DS!!!)

    AntyMew:
        Basically walked me through understanding how to make my makefile not suck.
