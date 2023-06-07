cmake_minimum_required(VERSION 1.00) #For sauce I have come,and to sauce I shall return
project(AIWNIOS)
if(WIN32) 
	set(
		ASM_SOURCES
		miscWIN.s
		swapctxWIN.s
		ffi_call_tos_WIN.s
	) 
else()
	set(
		ASM_SOURCES
		miscX86.s
		swapctxX86.s
		ffi_call_tos.s
	)
endif()
add_executable(
	aiwnios
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
find_package(PkgConfig)
pkg_check_modules(SDL2 REQUIRED sdl2)
target_compile_options(aiwnios PUBLIC ${SDL2_CFLAGS_OTHER})
target_include_directories(aiwnios PUBLIC ${SDL2_INCLUDE_DIRS})
set(ASM_OBJS "")
foreach(X IN LISTS ASM_SOURCES)
	add_custom_command(
		COMMAND yasm
		ARGS -f elf64 "${CMAKE_CURRENT_SOURCE_DIR}/${X}" -o "${X}.obj"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${X}"
		OUTPUT "${X}.obj"
	)
	set(ASM_OBJS "${ASM_OBJS}" "${X}.obj")
endforeach()
#https://stackoverflow.com/questions/34657287/cmakebuild-yasm-source-files
add_library(FFI ${ASM_OBJS}) 
set_target_properties(FFI PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(aiwnios ${SDL2_LIBRARIES} FFI m)
