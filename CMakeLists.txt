cmake_minimum_required(VERSION 3.10 FATAL_ERROR) #For sauce I have come,and to sauce I shall return
if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  set(CMAKE_INSTALL_PREFIX "/usr/local")
elseif(NOT WIN32)
  set(CMAKE_INSTALL_PREFIX "/usr")
endif ()

configure_file(generated.h.in generated.h @ONLY)
configure_file(aiwnios.desktop.in aiwnios.desktop @ONLY)

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
message("PROCESSOR ARCHITECTURE IS ${ARCH}")
if (ARCH STREQUAL "riscv64")
  set(
    C_SOURCES
    riscv_asm.c
    riscv_backend.c
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
    loader.c
    windows.c
    sound.c
    dbg.c
    multic.c
    ffi_gen.c
    argtable3.c
    socket.c
  )
  set(ASM_SOURCES
      miscRISCV.s
      swapctxRISCV.s
      ffi_call_tos_RISCV.s
    )
elseif ("${ARCH}" STREQUAL "x86_64")
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
    loader.c
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
    loader.c
    windows.c
    sound.c
    dbg.c
    multic.c
    ffi_gen.c
    argtable3.c
    socket.c
  )
endif()
if (NOT WIN32)
  list(APPEND C_SOURCES cli_vendor/bestline.c)
else ()
  set(CXX_SOURCES cli_vendor/linenoise.cpp
                  cli_vendor/wcwidth.cpp
                  cli_vendor/ConvertUTF.cpp)
endif ()
add_executable(
  aiwnios
  ${C_SOURCES}
  ${CXX_SOURCES}
)
find_package(SDL2 REQUIRED)
if (NOT SDL2_FOUND)
  message(FATAL_ERROR "Please install SDL2")
endif ()
target_include_directories(aiwnios PUBLIC ${SDL2_INCLUDE_DIRS})
if (ARCH STREQUAL arm64)
    target_link_directories(aiwnios PUBLIC /usr/local/lib)
    target_link_libraries(aiwnios PRIVATE SDL2)
elseif (ARCH STREQUAL x86_64)
  if(NOT WIN32)
    target_link_libraries(aiwnios PRIVATE SDL2::SDL2)
  else()
    #Try packaging SDL2 for cpack,please dont change this
    target_link_libraries(aiwnios PRIVATE SDL2::SDL2-static)
  endif()
endif ()

target_include_directories(aiwnios PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel"
    OR CMAKE_INTERPROCEDURAL_OPTIMIZATION)
  if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    find_program(LLD lld)
    if (NOT LLD)
      message(FATAL_ERROR "LTO with Clang requires LLD")
    endif ()
    set_target_properties(aiwnios
      PROPERTIES
      INTERPROCEDURAL_OPTIMIZATION OFF)
    target_link_options(aiwnios PRIVATE -flto=full -fuse-ld=lld)
    target_compile_options(aiwnios PRIVATE -flto=full)
  else ()
    set_target_properties(aiwnios
      PROPERTIES
      INTERPROCEDURAL_OPTIMIZATION ON)
  endif ()
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  target_link_libraries(aiwnios PRIVATE procstat)
endif ()
target_compile_options(aiwnios PRIVATE -w)

add_library(FFI STATIC ${ASM_SOURCES})
target_compile_options(FFI PRIVATE -w)
set_target_properties(FFI PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(aiwnios PRIVATE FFI m)

if (WIN32)
  target_link_libraries(aiwnios PRIVATE shlwapi winmm ws2_32 ntdll -static)
else()
  target_link_libraries(aiwnios PRIVATE pthread)
  if (ARCH STREQUAL "riscv64")
    target_link_libraries(aiwnios PRIVATE SDL2)
  endif ()
endif()


set(COMP_OPTS
  -Wno-all
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
  -fcf-protection=none
)
if (ARCH STREQUAL "x86_64")
  list(APPEND COMP_OPTS -mno-shstk)
endif ()
set_source_files_properties(${C_SOURCES}
  PROPERTIES
    LANGUAGE C
    COMPILE_FLAGS "-Wno-int-conversion"
)
target_compile_options(aiwnios
  PRIVATE
    ${COMP_OPTS}
)

set_target_properties(aiwnios
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    INTERPROCEDURAL_OPTIMIZATION $<IF:$<CONFIG:Debug>,OFF,ON>
    CXX_VISIBILITY_PRESET hidden
    C_VISIBILITY_PRESET hidden
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS YES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS YES
)

if (BUILD_HCRT)
  set (USE_WINE NO)
  if (NOT CMAKE_SYSTEM MATCHES Windows)
    if (WIN32)
      set(USE_WINE YES)
    endif ()
  endif ()

  set(BUILD_CMD aiwnios -b)
  if (USE_WINE)
    list(PREPEND BUILD_CMD wine64)
  endif ()
  add_custom_target(HCRT2.BIN
    ALL
    COMMAND ${BUILD_CMD}
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Src" aiwnios
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
  add_custom_target(Registry.HC.Z
    ALL
    COMMAND aiwnios -c FreshOnce.HC
    DEPENDS HCRT2.BIN aiwnios
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
  add_custom_command(
    TARGET HCRT2.BIN Registry.HC.Z
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --green "BUILD SUCCESSFULLY COMPLETED!!!!!"
  )
endif ()

#
# Gay section
#
set_target_properties(aiwnios PROPERTIES
  MACOSX_BUNDLE TRUE
  MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
)

#
# Install section
#
if(NOT WIN32)
  #Nroot here,CMAKE_INSTALL_PREFIX on windows has a C:/..... (So it gets combined with cwd to somethinf like)
  #  C:/dsdasd/C:/foofie
  set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
endif()

set (AIWNIOS_VERSION "0.0.1")

# FreeBSD
set (CPACK_FREEBSD_PACKAGE_NAME "aiwnios")
set (CPACK_FREEBSD_PACKAGE_COMMENT "A HolyC Compiler/Runtime for FreeBSD,Linux and Windows.")
set (CPACK_FREEBSD_PACKAGE_DESCRIPTION "A HolyC Compiler/Runtime for FreeBSD,Linux and Windows.")
set (CPACK_FREEBSD_PACKAGE_WWW "Coming soon!")
set (CPACK_FREEBSD_PACKAGE_LICENSE "BSD-3 clause")
set (CPACK_FREEBSD_PACKAGE_MAINTAINER "nrootconauto@gmail.com")
set (CPACK_FREEBSD_PACKAGE_ORIGIN "devel/aiwnios")
set (CPACK_FREEBSD_PACKAGE_DEPS "devel/sdl20")

# Debian
set (CPACK_DEBIAN_PACKAGE_PRIORITY "standard")
set (CPACK_DEBIAN_PACKAGE_SECTION "devel")
set (CPACK_DEBIAN_PACKAGE_DESCRIPTION "A HolyC Compiler/Runtime for Linux,Windows and FreeBSD")
set (CPACK_DEBIAN_PACKAGE_MAINTAINER "nrootconauto@gmail.com")
set (CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2-2.0-0")
set (CPACK_DEBIAN_PACKAGE_VERSION "${AIWNIOS_VERSION}")

#Windows
if(WIN32)
  set(CPACK_NSIS_IGNORE_LICENSE_PAGE)
  set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
  set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Aiwnios.lnk' '$INSTDIR\\\\bin\\\\aiwnios.exe' '' '$INSTDIR\\\\logo.ico' \n \
    CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\AiwniosFreshEnviroment.lnk' '$INSTDIR\\\\bin\\\\aiwnios.exe' '-n' 	'$INSTDIR\\\\logo.ico'"
  )
  set(CPACK_NSIS_DELETE_ICONS_EXTRA
    "Delete '$SMPROGRAMS\\\\$START_MENU\\\\Aiwnios.lnk' \n \
    Delete '$SMPROGRAMS\\\\$START_MENU\\\\AiwniosFreshEnviroment.lnk'")
endif()
include (CPack)

set(
  TEMPLATE_FILES
  HSNotes.DD
  AfterEgypt
  Doc
  Apps
  AiwniosHelp
  Src
  Vocab.DD
  PersonalMenu.DD
  HCRT2.BIN
  HCRT2.DBG.Z
  Demo
  Bible.TXT.Z
  ACDefs.DATA
  ACWords.DATA
)

if(NOT WIN32)
  install(PROGRAMS aiwnios TYPE BIN)
  install(FILES aiwnios.png DESTINATION share/icons)
  install(FILES aiwnios.desktop DESTINATION share/applications)
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
else()
  install(PROGRAMS aiwnios.exe TYPE BIN)
  install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/logo.ico" DESTINATION .)
  foreach(F IN LISTS TEMPLATE_FILES)
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${F}")
      install(
        DIRECTORY 
        "${CMAKE_CURRENT_SOURCE_DIR}/${F}"
        DESTINATION T
      )
     else()
       install (
         FILES
         "${CMAKE_CURRENT_SOURCE_DIR}/${F}"
         DESTINATION T
       )
     endif()
   endforeach()     
endif()

# vim: set expandtab ts=2 sw=2 :
