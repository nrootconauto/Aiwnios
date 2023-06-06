# Welcome to Aiwnios

This is a HolyC Compiler/Runtime written for 64bit ARM machines,although other 
aritchecures are planned for the future. This project is a work in progress,so stay
tuned.

## Screenshot

![screenshot](scrot.png)

## Building Aiwnios

I use a Raspberry Pi 3,as my daily machine for developing this peice of software,I would recomend 
something saucier if you want to be comfy expirience(I plan on getting a BananaPi). To build Aiwnios, you
will need
- `gcc`
- `libsdl2-dev`
- `cmake`

Building is as easy as typing `cmake .` then `make`,and to run it,use `./aiwnios`

## Future

Me(nrootconauto@gmail.com) is planning on adding an AARCH64 assembler that is accessable through HolyC,but 
assembly on arm is limited in terms of address space(`adr reg,off` is 26 bits,`ldr reg,addr` is even less) 
so It wouldn't be ideal for some stuff,I think it would be better for optimizing functions and stuff.

I also want to add a proper `ptrace` debugger for linux and (future) FreeBSD builds.
`ptrace` infuriates me though so if you are up to helping me with it,I will be up to it.
**EDIT** Ptrace is not suited for a realtime debugger,so If I was to write a "proper" debgger,it be not
so great.

Because Aiwnios runs pretty slow on my current hardware,I could make some of the games animations
time(*tS*) not *Sleep(n)* dependant.

## Internals

In aiwnios,the secret sauce is in mainly in `arm_backend.c`. There you will find the compiler.
I have gutted out the TempleOS expression parsing code and replaced it with calls to `__HC_ICAdd_XXXXX`
 which will be used in `arm_backend.c`. There is a super aarch64-assembler in `arm64_asm.c` 
 which you can use.
 
**THIS COMPILER USES REVERSE POLISH NOTATION**. And statements are reversed too so
the last statement is at `head->base.next` and the first one ends at `head->base.last`.
Email *nrootconauto@gmail.com* for more info(I hear my code is unreadable so I will stop
explaining here). 
