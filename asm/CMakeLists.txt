#-*- vi: set et ft=cmake ts=2 sts=2 sw=2 fenc=utf-8            :vi -*-┐
#─────────────────────────────────────────────────────────────────────┘
if (ARCH STREQUAL "riscv64")
  set(ASM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/miscRISCV.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/swapctxRISCV.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/ffi_call_tos_RISCV.s")
elseif (ARCH STREQUAL "x86_64")
  if (WIN32)
    set(ASM_SOURCES
      "${CMAKE_CURRENT_SOURCE_DIR}/miscWIN.s"
      "${CMAKE_CURRENT_SOURCE_DIR}/swapctxWIN.s"
      "${CMAKE_CURRENT_SOURCE_DIR}/ffi_call_tos_WIN.s"
      "${CMAKE_CURRENT_SOURCE_DIR}/miscTOSX86.s")
  else ()
    set(ASM_SOURCES
      "${CMAKE_CURRENT_SOURCE_DIR}/miscX86.s"
      "${CMAKE_CURRENT_SOURCE_DIR}/swapctxX86.s"
      "${CMAKE_CURRENT_SOURCE_DIR}/ffi_call_tos.s"
      "${CMAKE_CURRENT_SOURCE_DIR}/miscTOSX86.s")
  endif ()
elseif (ARCH STREQUAL "arm64")
  set(ASM_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/miscAARCH64.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/swapctxAARCH64.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/ffi_call_tos_aarch64.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/miscTOSaarch64.s")
endif ()

set_property(SOURCE ${ASM_SOURCES}
  APPEND PROPERTY
  COMPILE_OPTIONS "-x" "assembler-with-cpp")
add_library(FFI STATIC ${ASM_SOURCES})
set_target_properties(FFI PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(${PROJECT_NAME} PRIVATE FFI)
