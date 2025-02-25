#-*- vi: set et ft=cmake ts=2 sts=2 sw=2 fenc=utf-8            :vi -*-┐
#─────────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

option(BUILD_HCRT "Build kernel" ON)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "MAY NOT BUILD FROM ROOT")
endif ()


if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  set(CMAKE_INSTALL_PREFIX "/usr/local")
elseif (NOT WIN32)
  set(CMAKE_INSTALL_PREFIX "/usr")
endif ()

set(default_build_type "Release")
if ((NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE MATCHES "None")
    AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}"
    CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE
    PROPERTY
      STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(aiwnios
  LANGUAGES C ASM
  DESCRIPTION "HolyC compiler")

include (DetectArch)
target_architecture(ARCH)

add_subdirectory(c)
add_subdirectory(asm)

set_target_properties(aiwnios
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    C_VISIBILITY_PRESET hidden
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS YES)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(SDL2 REQUIRED)
if (NOT SDL2_FOUND)
  message(FATAL_ERROR "Please install SDL2")
endif ()
target_include_directories(aiwnios PUBLIC ${SDL2_INCLUDE_DIRS})

if (ARCH STREQUAL arm64)
  target_link_directories(aiwnios PUBLIC /usr/local/lib)
  target_link_libraries(aiwnios PRIVATE SDL2::SDL2)
elseif (ARCH STREQUAL x86_64)
  if (NOT WIN32)
    if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
      target_link_directories(aiwnios PUBLIC /usr/local/lib)
    endif ()
    target_link_libraries(aiwnios PRIVATE SDL2::SDL2)
  else ()
    #please dont change this
    target_link_libraries(aiwnios PRIVATE SDL2::SDL2-static)
  endif ()
endif ()

add_subdirectory(vendor)

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
target_link_libraries(aiwnios PRIVATE m)

if (WIN32)
  target_link_libraries(aiwnios PRIVATE shlwapi winmm ws2_32 ntdll bcrypt -static)
else()
  target_link_libraries(aiwnios PRIVATE pthread)
  if (ARCH STREQUAL "riscv64")
    target_link_libraries(aiwnios PRIVATE SDL2)
  endif ()
endif()

if (BUILD_HCRT)
  set(USE_WINE NO)
  if ((NOT CMAKE_SYSTEM MATCHES Windows) AND WIN32)
    set(USE_WINE YES)
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
  # for dumb people
  add_custom_command(
    TARGET HCRT2.BIN Registry.HC.Z
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --green "BUILD SUCCESSFULLY COMPLETED!!!!!")
endif ()

add_subdirectory(misc)

#
# Install section
#
if(NOT WIN32)
  #Nroot here,CMAKE_INSTALL_PREFIX on windows has a C:/..... (So it gets combined with cwd to somethinf like)
  #  C:/dsdasd/C:/foofie
  set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
endif()


set (CPACK_PACKAGE_VERSION_MAJOR 0)
set (CPACK_PACKAGE_VERSION_MINOR 9)
set (CPACK_PACKAGE_VERSION_PATCH 0)
set (AIWNIOS_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

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
  set(CPACK_NSIS_IGNORE_LICENSE_PAGE 1)
  set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Aiwnios.lnk' '$INSTDIR\\\\bin\\\\aiwnios.exe' '' '$INSTDIR\\\\logo.ico' \n \
    CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\AiwniosFreshEnviroment.lnk' '$INSTDIR\\\\bin\\\\aiwnios.exe' '-n' 	'$INSTDIR\\\\logo.ico'"
  )
  set(CPACK_NSIS_DELETE_ICONS_EXTRA
    "Delete '$SMPROGRAMS\\\\$START_MENU\\\\Aiwnios.lnk' \n \
    Delete '$SMPROGRAMS\\\\$START_MENU\\\\AiwniosFreshEnviroment.lnk'")
 set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS " 
   SetRegView 64
   WriteRegStr HKLM 'SOFTWARE\\\\Aiwnios' 'InstallAt' '$INSTDIR\\\\T_Template' #Dont use T(use T_Template),I will commit in the future to check permisions
   SetRegView Default
                ")
 set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS " 
    SetRegView 64
    DeleteRegKey HKLM 'SOFTWARE\\\\Aiwnios'
    SetRegView Default
    ")
 #set (CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CMAKE_CURRENT_SOURCE_DIR}/Install.bmp")
 #set (CPACK_NSIS_MUI_UNWELCOMEFINISHPAGE_BITMAP "${CMAKE_CURRENT_SOURCE_DIR}/Uninstall.bmp")
 set (CPACK_NSIS_BRANDING_TEXT "")
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
  God/Vocab.DD
  PersonalMenu.DD
  HCRT2.BIN
  HCRT2.DBG.Z
  Demo
  misc/Bible.TXT.Z
  Src/ACDefs.DATA
  Src/ACWords.DATA
)

if(NOT WIN32)
  install(PROGRAMS aiwnios TYPE BIN)
  install(FILES img/aiwnios.png DESTINATION share/icons)
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
  install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/img/logo.ico" DESTINATION .)
  foreach(F IN LISTS TEMPLATE_FILES)
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${F}")
      install(
        DIRECTORY 
        "${CMAKE_CURRENT_SOURCE_DIR}/${F}"
        DESTINATION T_Template
      )
     else()
       install (
         FILES
         "${CMAKE_CURRENT_SOURCE_DIR}/${F}"
         DESTINATION T_Template
       )
     endif()
   endforeach()     
endif()

# vim: set expandtab ts=2 sw=2 :
