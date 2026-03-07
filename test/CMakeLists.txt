
message( STATUS " --- inside of test CMakeLists --- " )

set( SRCS_TEST
     ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp 
)
set( SRCS_TEST ${SRCS_TEST} PARENT_SCOPE )

message( STATUS " --- finished of test CMakeLists --- " )