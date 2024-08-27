#-*- vi: set et ft=cmake ts=2 sts=2 sw=2 fenc=utf-8            :vi -*-┐
#─────────────────────────────────────────────────────────────────────┘
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/aiwnios.desktop.in"
               "${CMAKE_CURRENT_BINARY_DIR}/aiwnios.desktop" @ONLY)
set_target_properties(${PROJECT_NAME}
  PROPERTIES
  MACOSX_BUNDLE TRUE
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist")
