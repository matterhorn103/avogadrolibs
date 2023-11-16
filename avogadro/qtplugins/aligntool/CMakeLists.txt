set(aligntool_srcs
  aligntool.cpp
)

set(aligntool_uis
)

set(aligntool_rcs
  aligntool.qrc
)

avogadro_plugin(AlignTool
  "AlignTool"
  ToolPlugin
  aligntool.h
  AlignTool
  "${aligntool_srcs}"
  "${aligntool_uis}"
  "${aligntool_rcs}"
)

target_link_libraries(AlignTool PRIVATE Avogadro::Rendering Avogadro::QtOpenGL)

target_include_directories(AlignTool
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/../../rendering)

