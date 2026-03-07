
message( STATUS " --- inside of root CMakeLists --- " )

cmake_minimum_required( VERSION 3.16 )


project( news_tracker )

# Add src subdirectory
set( SRC_DIR ${CMAKE_SOURCE_DIR}/src )
add_subdirectory( ${SRC_DIR} )

message( STATUS " sources added from ${SRC_DIR} are ${SRCS} " )

# Add util subdirectory
set( UTIL_DIR ${CMAKE_SOURCE_DIR}/util )
add_subdirectory( ${UTIL_DIR} )

message( STATUS " sources added from ${UTIL_DIR} are ${SRCS_UTIL} " )



# Add test subdirectory
set( TEST_DIR ${CMAKE_SOURCE_DIR}/test )
add_subdirectory( ${TEST_DIR} )
message( STATUS " sources added from ${UTIL_DIR} are ${SRCS_TEST} " )

# Add external subdirectory
set( EXTENRAL_DIR ${CMAKE_SOURCE_DIR}/external )

set( XML2_DIR ${EXTENRAL_DIR}/tinyxml2 )
set( SRC_XML2 ${XML2_DIR}/tinyxml2.cpp )

set( JSON_DIR ${EXTENRAL_DIR}/json )

# Add config directory
set( CONFIG_DIR ${CMAKE_SOURCE_DIR}/config )

# Add packages
find_package(CURL REQUIRED)

# Add a target
add_executable( news_tracker ${SRCS} ${SRCS_UTIL} ${SRC_XML2} )

# Add links
target_link_libraries( news_tracker PRIVATE CURL::libcurl )

# Add include directories
target_include_directories( news_tracker PRIVATE ${UTIL_DIR} ${XML2_DIR} ${JSON_DIR}/single_include ${CONFIG_DIR} )


# Add a target for test
add_executable( news_tracker_test ${SRCS_TEST} )

# Add links for test
target_link_libraries( news_tracker_test PRIVATE CURL::libcurl )

# Add include directories for test
target_include_directories( news_tracker_test PRIVATE ${JSON_DIR}/single_include )



message( STATUS " --- finished of root CMakeLists --- " )






