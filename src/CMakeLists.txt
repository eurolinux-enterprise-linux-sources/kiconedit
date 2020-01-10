project(kiconedit)

find_package (KDE4 REQUIRED)
include(KDE4Defaults)
include_directories ( ${KDE4_INCLUDES} )
add_subdirectory( pics ) 

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

set(kiconedit_SRCS 
   utils.cpp 
   main.cpp 
   kiconedit.cpp 
   kicongrid.cpp 
   kiconcolors.cpp 
   kcolorgrid.cpp 
   palettetoolbar.cpp 
   kiconediticon.cpp 
   kresize.cpp 
   knew.cpp 
   properties.cpp 
   kiconeditslots.cpp 
   kiconconfig.cpp )

kde4_add_executable(kiconedit ${kiconedit_SRCS})

target_link_libraries(kiconedit  ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KIO_LIBS} )

install(TARGETS kiconedit  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( FILES kiconedit.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kiconeditui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kiconedit )
find_package(Msgfmt REQUIRED)
find_package(Gettext REQUIRED)
add_subdirectory( po )
add_subdirectory( doc-translations )
add_subdirectory( doc )
