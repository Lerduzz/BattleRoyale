AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/EventParkourMgr.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/ModEventParkour.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/EventParkourData.cpp")

AC_ADD_SCRIPT_LOADER("ModEventParkour" "${CMAKE_CURRENT_LIST_DIR}/src/loader.h")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/eventparkour.conf.dist")