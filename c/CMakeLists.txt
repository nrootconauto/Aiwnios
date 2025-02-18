#-*- vi: set et ft=cmake ts=2 sts=2 sw=2 fenc=utf-8            :vi -*-┐
#─────────────────────────────────────────────────────────────────────┘
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/generated.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/generated.h" @ONLY)

set(C_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/aiwn.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/bungis.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/cque.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/dbg.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/except.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/ffi_gen.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/fs.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/hash.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/tui.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/lex.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/logo.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/loader.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/lzw.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/main.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/mem.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/misc.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/multic.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/optpass.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/parser.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/socket.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/sound.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/ulock.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/windows.c")

if (ARCH STREQUAL "riscv64")
  list(APPEND C_SOURCES
    riscv_asm.c
    riscv_backend.c)
elseif (ARCH STREQUAL "x86_64")
  list(APPEND C_SOURCES
    x86_64_backend.c)
elseif (ARCH STREQUAL "arm64")
  # For SDL2 cmake modules
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)
  list(APPEND C_SOURCES
    arm64_asm.c 
    arm_backend.c)
endif()

add_executable(aiwnios ${C_SOURCES})
target_compile_options(aiwnios PRIVATE -Wno-all -Werror-implicit-function-declaration)
set(COMP_OPTS
  -w
  -Wno-all
  -Wno-int-conversion
  -Wno-incompatible-function-pointer-types
  -Wno-incompatible-pointer-types
  -fno-math-errno
  -fno-trapping-math
  -fno-exceptions
  -fno-omit-frame-pointer
  -fno-signaling-nans
  -fno-stack-protector
  -fno-unwind-tables
  -fno-asynchronous-unwind-tables
  -fcf-protection=none)
target_compile_definitions(aiwnios PRIVATE _GNU_SOURCE=1)
if (ARCH STREQUAL "x86_64")
  list(APPEND COMP_OPTS -mno-shstk -fno-plt -mno-red-zone)
endif ()
if (ARCH STREQUAL "arm64")
  list(APPEND COMP_OPTS -ffixed-x28)
endif ()
if (CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
  list(APPEND COMP_OPTS -fno-ret-protector)
endif ()

target_sources(${PROJECT_NAME} PRIVATE ${C_SOURCES})
target_compile_options(${PROJECT_NAME} PRIVATE ${COMP_OPTS})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
