find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)

avogadro_plugin(ResetView
  "Manipulate the view camera."
  ExtensionPlugin
  resetview.h
  ResetView
  "resetview.cpp"
  ""
)

target_link_libraries(ResetView PRIVATE Qt6::OpenGLWidgets Avogadro::Rendering Avogadro::QtOpenGL)

target_include_directories(ResetView
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/../../rendering
    ${CMAKE_CURRENT_BINARY_DIR}/../../qtopengl)
