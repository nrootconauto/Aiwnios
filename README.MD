# Welcome to Aiwnios

This is a HolyC Compiler/Runtime written for 64bit x86, aarch64 (includes ***MacOS***) and RISC-V
machines,although other aritchecures are planned for the future. This project is a work in progress,so stay tuned.

## Support table

| Architecture | OS |
|--|--|
| x86_64 | Windows, Linux and FreeBSD |
| aarch64 | Linux, FreeBSD and **MacOS*** |
| rv64 (RISC-V 64) | Linux |

## Screenshot
<div align="center">
    <img src="img/scrot.png" alt="Aiwnios Screenshot" width="80%">
</div> <!-- whoever did the <center> was retarded, thats HTML3 (!) and only applies to text -->

## Building Aiwnios

### FreeBSD/Linux

I use a Raspberry Pi 3,as my daily machine for developing this peice of software,I would recomend 
something saucier if you want to be comfy expirience (M1 macs with Asahi Linux work smoothly if you ignore the chassis).
To build Aiwnios, you will need a C compiler, SDL2 development libraries and headers, and cmake. LTO with Clang/lld is supported if you like something saucy.

Build with the following after cloning:

```sh
# Build aiwnios
mkdir build;cd build;
cmake ..;
make -j$(nproc);
cd ..;

#Bootstrap the HCRT2.BIN to run it
./aiwnios -b;

#Run the HCRT2.BIN
./aiwnios; # Use -g or --grab-focus to grab the keyboard, -h for more options
```

If you want to create a cool package for your system, you can use the [ESP Package Manager](https://github.com/jimjag/epm/). Simply run `epm aiwnios` to make a cool package for your system.

### Windows

1. Install [msys2](https://msys2.org)
2. Run "MSYS2 MINGW64" (***MUST*** BE MINGW64)
3. `pacman -Sy git mingw-w64-x86_64-{gcc,SDL2,cmake}`
4. Clone this repository
5. Run the following after navigating to the directory
```
mkdir build
cd build
cmake ..
ninja
cd ..
```
5. You will see the `aiwnios` binary in the directory

### MacOS

Your on your own. Use homebrew to install packages, rest is same as FreeBSD

# Future

i plan on adding something lit like an arm assembler from HolyC.

## Internals

In aiwnios,the secret sauce is in mainly in `*_backend.c`. There you will find the compiler.
I have gutted out the TempleOS expression parsing code and replaced it with calls to `__HC_ICAdd_XXXXX`
 which will be used in `*_backend.c`. There is a super assembler in `*_asm.c` 
 which you can use. Look at `ffi.c` to see how its used.

**THIS COMPILER USES REVERSE POLISH NOTATION**. And statements are reversed too so
the last statement is at `head->base.next` and the first one ends at `head->base.last`.
Email *nrootconauto@gmail.com* for more info(I hear my code is unreadable so I will stop
explaining here). 

## Sockets

Aiwnios comes with a sockets API.

Here is a simple server for you to play with until Nroot documents the Sockets API
```c
U0 Main () {
  U8 buf[STR_LEN];
  I64 fd;
  I64 s=NetSocketNew;
  CNetAddr *addr;
  addr=NetAddrNew("127.0.0.1",8000);
  NetBindIn(s,addr);
  NetListen(s,4);
  while(TRUE) {
    if(-1==NetPollForRead(1,&s)) {
      Sleep(10); 
    } else {
      fd=NetAccept(s,NULL);
      while(-1==NetPollForRead(1,&fd))
        Sleep(10);
      buf[NetRead(fd,buf,STR_LEN)]=0;
      "GOT:%s\n",buf;
      NetClose(fd);
    }
    if(ScanKey)
     break;
  }
  NetClose(s);
  NetAddrDel(addr);
}
Main;
```


## Credits

- [argtable3](https://github.com/argtable/argtable3)
- [Cmake architecture detector by axr](https://github.com/axr/solar-cmake/blob/master/TargetArch.cmake)
- [Xbyak Arm assembler](https://github.com/fujitsu/xbyak_aarch64)
- [sdl2-cmake-modules](https://github.com/aminosbh/sdl2-cmake-modules)
- [AArch64-Encodung](https://github.com/CAS-Atlantic/AArch64-Encoding)

---

# Developer manual

If you want something saucier and want to understand the sauce, look at [the developer manual](DEVMAN.MD)
