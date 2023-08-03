# Aiwnios

Welcome to Aiwnios, a HolyC Compiler/Runtime designed for 64-bit ARM and x86 machines, with plans for supporting other architectures in the future. 

*Please note that this project is currently a work in progress, so there will be regular updates and improvements.*

### Aiwnios in Action
<center>
    <img src="scrot.png" alt="Aiwnios Screenshot" width="80%">
</center>

## Building Aiwnios

While I primarily develop this software using a Raspberry Pi 3, I recommend using a more powerful machine for a smoother experience (I'm planning on getting a BananaPi soon!). 

To build Aiwnios, you will need the following dependencies:

- `gcc`
- `libsdl2-dev`
- `cmake`

Once you have the dependencies installed, building Aiwnios is straightforward:

```sh
# Build Aiwnios
mkdir build;cd build;
cmake ..;
make -j$(nproc);
cd ..;

# Bootstrap the HCRT2.BIN to run it
./aiwnios -b;

# Run the HCRT2.BIN
./aiwnios;
```

If you want to create a package for your system, you can use the [ESP Package Manager](https://github.com/jimjag/epm/). Simply run `epm aiwnios` to make a cool package for your system.

## Future Plans

In the future, I ([nrootconauto](mailto:nrootconauto@gmail.com)) plan to add an AARCH64 assembler accessible through HolyC. However, please note that assembly on ARM has limitations in terms of address space, which may not be ideal for certain tasks. Nonetheless, it would be valuable for optimizing functions and other purposes.

Additionally, I intend to develop a proper `ptrace` debugger for Linux and future FreeBSD builds. Although `ptrace` can be frustrating, I would appreciate any assistance in this area if you're up for the challenge. Please note that a "proper" debugger may not be ideal for real-time debugging.

Given that Aiwnios currently runs quite slowly on my current hardware, I may implement time-based (*tS*) animations instead of relying on `Sleep(n)`.

## Internals

The heart of Aiwnios lies primarily in `arm_backend.c`, where you'll find the compiler. I have replaced the TempleOS expression parsing code with calls to `__HC_ICAdd_XXXXX`, which are used in `arm_backend.c`. Additionally, there is a powerful AARCH64 assembler available in `arm64_asm.c` that you can utilize.

Please be aware that **THIS COMPILER USES REVERSE POLISH NOTATION**. Furthermore, statements are reversed, so the last statement resides at `head->base.next`, while the first statement concludes at `head->base.last`. For more information, feel free to contact me via email at [nrootconauto@gmail.com](mailto:nrootconauto@gmail.com). I apologize if my code is currently hard to read; I'll be happy to provide further explanations.

## Sockets

Aiwnios comes with a sockets API(currently tested on FreeBSD). This should be cross platform in the future

Here is a simple server for you to play with until Nroot documents the Sockets API,(currently only server stuff is here,not client stuff but it will be an easy fix)
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

Aiwnios would not be possible without the valuable contributions of the following:

- [argtable3](https://github.com/argtable/argtable3)
- [Cmake architecture detector by axr](https://github.com/axr/solar-cmake/blob/master/TargetArch.cmake)
- [Xbyak Arm assembler](https://github.com/fujitsu/xbyak_aarch64)
- [sdl2-cmake-modules](https://github.com/aminosbh/sdl2-cmake-modules)
