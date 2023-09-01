AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/BattleRoyaleMgr.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/ModBattleRoyale.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/BattleRoyaleData.cpp")

AC_ADD_SCRIPT_LOADER("ModBattleRoyale" "${CMAKE_CURRENT_LIST_DIR}/src/loader.h")

AC_ADD_CONFIG_FILE("${CMAKE_CURRENT_LIST_DIR}/conf/battleroyale.conf.dist")