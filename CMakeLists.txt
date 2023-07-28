cmake_minimum_required(VERSION 3.10 FATAL_ERROR) #For sauce I have come,and to sauce I shall return
include("${CMAKE_CURRENT_LIST_DIR}/DetectArch.cmake")
project(AIWNIOS
  LANGUAGES C CXX ASM
)
option(BUILD_HCRT "Build HCRT" YES)
set(default_build_type "Release")

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}"
    CACHE STRING "Choose the type of build." FORCE
  )
  set_property(CACHE CMAKE_BUILD_TYPE
    PROPERTY
      STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
  )
endif ()

target_architecture(ARCH)
if ("${ARCH}" STREQUAL "x86_64")
  if (WIN32)
    set(ASM_SOURCES
      miscWIN.s
      swapctxWIN.s
      ffi_call_tos_WIN.s
      miscTOSX86.s
    )
  else ()
    set(ASM_SOURCES
      miscX86.s
      swapctxX86.s
      ffi_call_tos.s
      miscTOSX86.s
    )
endif ()
  set(
    C_SOURCES
    x86_64_backend.c 
    bungis.c
    cque.c
    except.c
    fs.c
    hash.c
    lex.c
    main.c
    mem.c
    misc.c
    optpass.c
    parser.c
    arm_loader.c
    windows.c
    sound.c
    dbg.c
    multic.c
    ffi_gen.c
    argtable3.c
    socket.c
  )
elseif ("${ARCH}" STREQUAL "arm64")
  #For SDL2 cmake modules
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

  set(ASM_SOURCES
    miscAARCH64.s
    swapctxAARCH64.s
    ffi_call_tos_aarch64.s
    miscTOSaarch64.s
  )
  set(C_SOURCES
    arm64_asm.c 
    arm_backend.c
    bungis.c
    cque.c
    except.c
    fs.c
    hash.c
    lex.c
    main.c
    mem.c
    misc.c
    optpass.c
    parser.c
    arm_loader.c
    windows.c
    sound.c
    dbg.c
    multic.c
    ffi_gen.c
    argtable3.c
    socket.c
  )
endif()
add_executable(
  aiwnios
  ${C_SOURCES}
)
find_package(SDL2 REQUIRED)
if (NOT SDL2_FOUND)
  message(FATAL_ERROR "Please install SDL2")
endif ()
target_include_directories(aiwnios PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(aiwnios PRIVATE SDL2::SDL2)

target_compile_options(aiwnios PRIVATE -w)

add_library(FFI STATIC ${ASM_SOURCES})
target_compile_options(FFI PRIVATE -w)
set_target_properties(FFI PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(aiwnios PRIVATE FFI m)

if (WIN32)
  target_link_libraries(aiwnios PRIVATE shlwapi winmm ws2_32 -static)
else()
  target_link_libraries(aiwnios PRIVATE pthread)
endif()

set_target_properties(aiwnios
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    INTERPROCEDURAL_OPTIMIZATION ON
    C_VISIBILITY_PRESET hidden
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS YES
)

if (BUILD_HCRT)
  set (USE_WINE NO)
  if(NOT (CMAKE_SYSTEM MATCHES Windows))
    if(WIN32)
      set(USE_WINE YES)
    endif()
  endif()

  if (NOT USE_WINE)
    add_custom_target(HCRT2.BIN
      ALL
      COMMAND aiwnios -b
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Src" aiwnios
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
  else ()
    add_custom_target(HCRT2.BIN
      ALL
      COMMAND wine64 aiwnios -b
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Src" aiwnios
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
  endif ()
endif ()

target_compile_options(aiwnios
  PRIVATE
    -fno-math-errno
    -fno-trapping-math
    -fno-exceptions
    -fno-omit-frame-pointer
    -fno-signaling-nans
    -fno-stack-protector
    -fno-unwind-tables
    -fno-asynchronous-unwind-tables
    -fcf-protection=none
    -mno-shstk
)

#
# Install section
#
set (CPACK_FREEBSD_PACKAGE_NAME "aiwnios")
set (CPACK_FREEBSD_PACKAGE_COMMENT "A HolyC Compiler/Runtime for FreeBSD,Linux and Windows.")
set (CPACK_FREEBSD_PACKAGE_DESCRIPTION "A HolyC Compiler/Runtime for FreeBSD,Linux and Windows.")
set (CPACK_FREEBSD_PACKAGE_WWW "Coming soon!")
set (CPACK_FREEBSD_PACKAGE_LICENSE "BSD-3 clause")
set (CPACK_FREEBSD_PACKAGE_MAINTAINER "nrootconauto@gmail.com")
set (CPACK_FREEBSD_PACKAGE_ORIGIN "devel/aiwnios")
set (CPACK_FREEBSD_PACKAGE_DEPS "devel/sdl20")

include (CPack)

set(
  TEMPLATE_FILES
  HSNotes.DD
  AfterEgypt
  Doc
  Apps
  Src
  Psalmody
  Vocab.DD
  PersonalMenu.DD
  HCRT2.BIN
  HCRT2.DBG.Z
  Demo
  Bible.TXT.Z
  ACDefs.DATA
  ACWords.DATA
)

install(FILES aiwnios TYPE BIN)

foreach(F IN LISTS TEMPLATE_FILES)
  if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${F}")
    install(
      DIRECTORY 
      "${CMAKE_CURRENT_SOURCE_DIR}/${F}"
      DESTINATION share/aiwnios
    )
  else()
    install (
      FILES
      "${CMAKE_CURRENT_SOURCE_DIR}/${F}"
      DESTINATION share/aiwnios
    )
  endif()
endforeach()

# vim: set expandtab ts=2 sw=2 :
