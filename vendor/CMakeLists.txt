#-*- vi: set et ft=cmake ts=2 sts=2 sw=2 fenc=utf-8            :vi -*-┐
#─────────────────────────────────────────────────────────────────────┘
add_subdirectory(isocline)
add_subdirectory(argtable3)

set_target_properties(argtable3 isocline
  PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS YES
    C_VISIBILITY_PRESET hidden
    LINKER_LANGUAGE C)

add_library(c_opts INTERFACE)
target_compile_options(c_opts
  INTERFACE
    -fno-exceptions
    -fno-unwind-tables
    -fno-asynchronous-unwind-tables
    -fno-stack-protector
    -mno-shstk
    -mno-red-zone
    -fcf-protection=none)

if (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel"
    OR CMAKE_INTERPROCEDURAL_OPTIMIZATION)
  if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    set_target_properties(argtable3 isocline 
      PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION OFF)
    target_compile_options(c_opts INTERFACE -flto=full)
  else ()
    set_target_properties(argtable3 isocline 
      PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION ON)
  endif ()
endif ()

target_link_libraries(argtable3 PRIVATE c_opts)
target_link_libraries(isocline  PRIVATE c_opts)

target_link_libraries(${PROJECT_NAME} PRIVATE argtable3 isocline)
