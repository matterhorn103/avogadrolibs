if(QT_VERSION EQUAL 6)
  find_package(Qt6 COMPONENTS Gui REQUIRED)
else()
  find_package(Qt5 COMPONENTS Widgets REQUIRED)
endif()

set(forcefield_srcs
  forcefield.cpp
  forcefielddialog.cpp
  obmmenergy.cpp
  scriptenergy.cpp
)

avogadro_plugin(Forcefield
  "Force field optimization and dynamics"
  ExtensionPlugin
  forcefield.h
  Forcefield
  "${forcefield_srcs}"
  forcefielddialog.ui
)

target_link_libraries(Forcefield PRIVATE Avogadro::Calc)

if(QT_VERSION EQUAL 6)
  target_link_libraries(Forcefield PRIVATE Qt6::Gui)
else()
  target_link_libraries(Forcefield PRIVATE Qt5::Widgets)
endif()

# Bundled forcefield scripts
set(forcefields
  scripts/ani2x.py
  scripts/gaff.py
  scripts/gfn1.py
  scripts/gfn2.py
  scripts/gfnff.py
  scripts/mmff94.py
  scripts/uff.py
)

install(PROGRAMS ${forcefields}
DESTINATION "${INSTALL_LIBRARY_DIR}/avogadro2/scripts/energy/")
