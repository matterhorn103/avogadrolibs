find_package(Qt6 REQUIRED COMPONENTS OpenGL OpenGLWidgets)

avogadro_plugin(PlayerTool
  "Player tool"
  ToolPlugin
  playertool.h
  PlayerTool
  "playertool.cpp"
  ""
  playertool.qrc
)

target_link_libraries(PlayerTool
  PRIVATE
    gwavi
    gif-h
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Avogadro::Rendering
    Avogadro::QtOpenGL)

target_include_directories(PlayerTool
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/../../rendering
    ${CMAKE_CURRENT_BINARY_DIR}/../../qtopengl)
