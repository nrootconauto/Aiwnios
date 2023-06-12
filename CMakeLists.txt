cmake_minimum_required(VERSION 3.00) #For sauce I have come,and to sauce I shall return
include("${CMAKE_CURRENT_LIST_DIR}/DetectArch.cmake")
project(AIWNIOS)
target_architecture(ARCH)
if("${ARCH}" STREQUAL "x86_64")
	if(WIN32)
		set(
			ASM_SOURCES
			miscWIN.s
			swapctxWIN.s
			ffi_call_tos_WIN.s
			miscTOSX86.s
		)
	else()
		set(
			ASM_SOURCES
			miscX86.s
			swapctxX86.s
			ffi_call_tos.s
			miscTOSX86.s
		)
	endif()
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
  )
elseif("${ARCH}" STREQUAL "arm64")
	set(
		ASM_SOURCES
		miscAARCH64.s
		swapctxAARCH64.s
		ffi_call_tos_aarch64.s
	)
  set(
    C_SOURCES
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
  )
endif()
add_executable(
  aiwnios
  ${C_SOURCES}
)
find_package(PkgConfig)
pkg_check_modules(SDL2 REQUIRED sdl2)
target_compile_options(aiwnios PUBLIC ${SDL2_CFLAGS_OTHER})
target_include_directories(aiwnios PUBLIC ${SDL2_INCLUDE_DIRS})
set(ASM_OBJS "")
if("${ARCH}" STREQUAL "x86_64")
  foreach(X IN LISTS ASM_SOURCES)
    add_custom_command(
      COMMAND yasm
      ARGS -f elf64 "${CMAKE_CURRENT_SOURCE_DIR}/${X}" -o "${CMAKE_CURRENT_BINARY_DIR}/${X}.obj"
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${X}"
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${X}.obj"
    )
    set(ASM_OBJS "${ASM_OBJS}" "${CMAKE_CURRENT_BINARY_DIR}/${X}.obj")
  endforeach()
elseif("${ARCH}" STREQUAL "arm64")
  foreach(X IN LISTS ASM_SOURCES)
    add_custom_command(
      COMMAND gcc
      ARGS -c "${CMAKE_CURRENT_SOURCE_DIR}/${X}" -o "${CMAKE_CURRENT_BINARY_DIR}/${X}.obj"
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${X}"
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${X}.obj"
    )
    set(ASM_OBJS "${ASM_OBJS}" "${CMAKE_CURRENT_BINARY_DIR}/${X}.obj")
  endforeach()
endif()
#https://stackoverflow.com/questions/34657287/cmakebuild-yasm-source-files
add_library(FFI ${ASM_OBJS}) 
set_target_properties(FFI PROPERTIES LINKER_LANGUAGE CXX)
target_link_directories(aiwnios PUBLIC ${SDL2_LIBRARY_DIRS})
if(WIN32)
	target_link_libraries(aiwnios ${SDL2_LIBRARIES} FFI m)
else()
	target_link_libraries(aiwnios ${SDL2_LIBRARIES} FFI m pthread)
endif()
