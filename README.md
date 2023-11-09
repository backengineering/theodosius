<h1 align="center">Theodosius</h1>

Theodosius (Theo for short) is a jit linker created for obfuscation. The project is extremely modular in design and supports both kernel and usermode projects. 
Theo works with static libraries rather than completely compiled binaries. This allows it to easily position, obfuscate, and scatter symbols anywhere as the project takes the place of the linker.

[Doxygen for this project can be found here](https://docs.back.engineering/theo/)

### Table Of Contents

* [Theodosius - Jit linker, Mapper, Mutator, and Obfuscator](https://githacks.org/_xeroxz/theodosius#theodosius-jit-linker-mapper-mutator-and-obfuscator)
    * [Credit And Dependencies](https://githacks.org/_xeroxz/theodosius#credit-and-dependencies)
    * [Linking - Dynamic And Static](https://githacks.org/_xeroxz/theodosius#linking-dynamic-and-static)
        * [What Is A Linker](https://githacks.org/_xeroxz/theodosius#what-is-a-linker)
        * [Object Files](https://githacks.org/_xeroxz/theodosius#object-files)
        * [Static Linking](https://githacks.org/_xeroxz/theodosius#static-linking)
        * [Dynamic Linking](https://githacks.org/_xeroxz/theodosius#dynamic-linking)
    * [Usage - Using Theodosius](https://githacks.org/_xeroxz/theodosius#usage-using-theodosius)
        * [Integrating Clang](https://githacks.org/_xeroxz/theodosius#integrating-clang)
            * [Requirements](https://githacks.org/_xeroxz/theodosius#requirements)    
* [RIP Relative Addressing](https://githacks.org/_xeroxz/theodosius#rip-relative-addressing)
* [License - BSD 3-Clause](https://githacks.org/_xeroxz/theodosius#bsd-3-clause-license)

### Credit And Dependencies

* [BTBD](https://github.com/btbd) - Huge thanks for providing suggestions and bouncing ideas back and forth.
    * [SMAP](https://github.com/btbd/smap) - scatter mapper, this project is heavily influenced by SMAP.
* [XED](https://github.com/intelxed/xed)
* [spdlog](https://github.com/gabime/spdlog)
* [linux-pe](https://github.com/can1357/linux-pe)
* [cmkr](https://github.com/build-cpp/cmkr)

### Building

Download and install cmake on your system, then execute the following command in the root dir of this project:

* `cmake -B build`

Then navigate into `dependencies/xed/` and run `python3 mfile.py`. Building XED can be tricky on windows, I suggest you use the visual studios console since it has env vars to everything needed to build XED. linux seems to build it just fine...

## Linking - Dynamic And Static

#### What Is A Linker

A linker is a program which takes object files produces by a compiler and generates a final executable native to the operating system. A linker interfaces with not only object files but also static libraries, "lib" files. What is a "lib" file? Well a lib file is just an archive of obj's. You can invision it as a zip/rar without any compression, just concatination of said object files.

Theo is a jit linker, which means it will link objs together and map them into memory all at once. For usability however, instead of handling object files, Theo can parse entire lib files and extract the objects out of the lib.

#### Object Files

If you define a c++ file called "main.cpp" the compiler will generate an object file by the name of "main.obj". When you refer to data or code defined in another c/c++ file, the linker uses a symbol table to resolve the address of said code/data. In this situation I am the linker and I resolve all of your symbols :).

#### Static Linking

Static linking is when the linker links entire routines not created by you, into your code. Say `memcpy` (if its not inlined), will be staticlly linked with the CRT. Static linking also allows for your code to be more independant as all the code you need you bring with you. However, with Theo, you cannot link static libraries which are not compiled with `mcmodel=large`. Theo supports actual static linking, in other words, using multiple static libraries at the same time.

#### Dynamic Linking

Dynamic linking is when external symbols are resolved at runtime. This is done by imports and exports in DLL's (dynamiclly linked libraries). Theo supports "dynamic linking", or in better terms, linking against exported routines. You can see examples of this inside of both usermode and kernelmode examples.

# Usage - Using Theodosius 

## Integrating Clang

For integration with visual studios please open install [llvm2019](https://marketplace.visualstudio.com/items?itemName=MarekAniola.mangh-llvm2019) extension, or [llvm2017](https://marketplace.visualstudio.com/items?itemName=LLVMExtensions.llvm-toolchain) extension. Once installed, create or open a visual studio project which you want to use with LLVM-Obfuscator and Theo. Open ***Properties*** --> ***Configuration Properties*** ---> ***General***, then set ***Platform Toolset*** to ***LLVM***.

Once LLVM is selected, under the ***LLVM*** tab change the clang-cl location to the place where you extracted [clang-cl.rar](https://githacks.org/_xeroxz/theodosius/-/blob/cc9496ccceba3d1f0916859ddb2583be9362c908/resources/clang-cl.rar). Finally under ***Additional Compiler Options*** (same LLVM tab), set the following: `-Xclang -std=c++1z -Xclang -mcode-model -Xclang large -Xclang -fno-jump-tables -mllvm -split -mllvm -split_num=4 -mllvm -sub_loop=4`. 

Please refer to the [LLVM-Obfuscator Wiki](https://github.com/obfuscator-llvm/obfuscator/wiki) for more information on commandline arguments.

#### Requirements

* No SEH support, do not add `__try/__except` in your code.
* No CFG (control flow guard) support. Please disable this in C/C++ ---> Code Generation ---> Control Flow Guard
* No Stack Security Check Support. Please disablel this in C/C++ ---> Code Generation ---> Security Check (/GS-)
* Your project must be set to produce a .lib file. 
* Your project must not link with other static libraries which are not compiled with `-Xclang -mcmodel-large`.
* Project must be compiled with the following flags
    * `-Xclang -mcmodel=large`, removes RIP relative addressing besides JCC's.
    * `-Xclang -fno-jump-tables`, removes jump tables created by switch cases.
    * `/Zc:threadSafeInit-`, static will not use TLS (thread local storage).

## RIP Relative Addressing

In order to allow for a routine to be scattered throughout a 64bit address space, RIP relative addressing must not be used. In order to facilitate this, a very special version
of clang-cl is used which can use `mcmodel=large`. This will generate instructions which do not use RIP relative addressing when referencing symbols outside of the routine in which the
instruction itself resides. The only exception to this is JCC instructions, (besides call) also known as branching instructions. Take this c++ code for an example:

```cpp
ObfuscateRoutine 
extern "C" int ModuleEntry()
{
	MessageBoxA(0, "Demo", "Hello From Obfuscated Routine!", 0);
	UsermodeMutateDemo();
	UsermodeNoObfuscation();
}
```

This c++ function, compiled by clang-cl with `mcmodel=large`, will generate a routine with the following instructions:

```
0x00:                               ; void UsermodeNoObfuscation(void)
0x00:                                               public ?UsermodeNoObfuscation@@YAXXZ
0x00:                               ?UsermodeNoObfuscation@@YAXXZ proc near ; CODE XREF: ModuleEntry+42↓p
0x00:                               var_4           = dword ptr -4
0x00: 48 83 EC 28                                   sub     rsp, 28h
0x04: C7 44 24 24 00 00 00 00                       mov     [rsp+28h+var_4], 0
0x0C:                               loc_C:
0x0C: 83 7C 24 24 05                                cmp     [rsp+28h+var_4], 5
0x11: 0F 83 38 00 00 00                             jnb     loc_4F
0x17: 31 C0                                         xor     eax, eax
0x19: 48 BA 28 01 00 00 00 00 00 00                 mov     rdx, offset ??_C@_04DKDMNOEB@Demo?$AA@ ; "Demo"
0x23: 49 B8 00 01 00 00 00 00 00 00                 mov     r8, offset ??_C@_0CD@JEJKPGNA@Hello?5... ; "Hello From Non-Obfuscated Routine!"
0x2D: 48 B8 A0 01 00 00 00 00 00 00                 mov     rax, offset MessageBoxA
0x37: 45 31 C9                                      xor     r9d, r9d        ; uType
0x3A: 44 89 C9                                      mov     ecx, r9d        ; hWnd
0x3D: FF D0                                         call    rax ; MessageBoxA
0x3F: 8B 44 24 24                                   mov     eax, [rsp+28h+var_4]
0x43: 83 C0 01                                      add     eax, 1
0x46: 89 44 24 24                                   mov     [rsp+28h+var_4], eax
0x4A: E9 BD FF FF FF                                jmp     loc_C
0x4F:                               loc_4F:
0x4F: 48 83 C4 28                                   add     rsp, 28h
0x53: C3                                            retn
0x53:                               ?UsermodeNoObfuscation@@YAXXZ endp
```

As you can see from the code above, (sorry for the terrible syntax highlighting), references to strings and calls to functions are done by first loading the address of the symbol into a register and then interfacing with the symbol. 

```
0x2D: 48 B8 A0 01 00 00 00 00 00 00                 mov     rax, offset MessageBoxA
; ...
0x3D: FF D0                                         call    rax ; MessageBoxA
```

Each of these instructions can be anywhere in virtual memory and it would not effect code execution one bit. 

# BSD 3-Clause License

Copyright (c) 2022, _xeroxz
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
