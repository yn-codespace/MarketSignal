
message( STATUS " --- inside of src CMakeLists --- " )

set( SRCS ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp )
set( SRCS ${SRCS} PARENT_SCOPE )

message( STATUS " --- finished of src CMakeLists --- " )
