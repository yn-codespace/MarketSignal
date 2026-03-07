
message( STATUS " --- inside of src CMakeLists --- " )

set( SRCS_UTIL 
     ${CMAKE_CURRENT_SOURCE_DIR}/file_reader.cpp 
     ${CMAKE_CURRENT_SOURCE_DIR}/RSSFetcher.cpp 
)
set( SRCS_UTIL ${SRCS_UTIL} PARENT_SCOPE )

message( STATUS " --- finished of src CMakeLists --- " )