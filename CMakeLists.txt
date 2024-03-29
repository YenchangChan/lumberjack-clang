
CMAKE_MINIMUM_REQUIRED(VERSION 3.17)

PROJECT(lumberjack VERSION 0.1 LANGUAGES C CXX)

ADD_SUBDIRECTORY(examples bin)

SET(SRC_LIST lumberjack.c utils.c)
#SET(CXX_FORMAT_SIGNEDNESS "-Wformat-signedness")
SET(CXX_FORMAT_SECURITY "-Werror=format-security")
SET(CXX_STACK_PROTECTOR "-fstack-protector-all")
SET(CXX_FLAGS_DEBUG "-O0")
SET(CMAKE_C_STANDARD 99)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1 -ggdb -Wall -Wextra -DNETDATA_INTERNAL_CHECKS=1 -DNETDATA_VERIFY_LOCKS=1 ${CXX_FORMAT_SIGNEDNESS} ${CXX_FORMAT_SECURITY} ${CXX_STACK_PROTECTOR} ${CXX_FLAGS_DEBUG}")

find_library(LIB_SSL libssl.a libcrypto.a /usr/local/eoitek/lib)
find_library(LIB_ZLIB libz.a /usr/local/eoitek/lib)

add_definitions(-DHAVE_ZLIB_H -DHAVE_SSL_H)

ADD_LIBRARY(lumberjack_static STATIC ${SRC_LIST})
SET_TARGET_PROPERTIES(lumberjack_static PROPERTIES OUTPUT_NAME "lumberjack")
SET_TARGET_PROPERTIES(lumberjack_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

ADD_LIBRARY(lumberjack_shared SHARED ${SRC_LIST})
SET_TARGET_PROPERTIES(lumberjack_shared PROPERTIES OUTPUT_NAME "lumberjack")
SET_TARGET_PROPERTIES(lumberjack_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_include_directories(lumberjack_static INTERFACE /usr/local/eoitek/include)
target_include_directories(lumberjack_shared INTERFACE /usr/local/eoitek/include)

INSTALL(FILES constant.h utils.h lumberjack.h DESTINATION include/lumberjack)
INSTALL(TARGETS lumberjack_static ARCHIVE DESTINATION lib/lumberjack)
INSTALL(TARGETS lumberjack_shared LIBRARY DESTINATION lib/lumberjack)