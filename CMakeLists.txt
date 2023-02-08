cmake_minimum_required(VERSION 3.20)

project(rudp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Setup boost
set(Boost_USE_STATIC_LIBS on)
find_package(
	Boost 1.65 REQUIRED
	COMPONENTS system thread
)

# Library Definition
set(includes_list_lib ${Boost_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SOURCES_LIB "${CMAKE_CURRENT_SOURCE_DIR}/src/rudp.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/ConnectionController.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/Connection.cpp")
add_library(rudp STATIC ${SOURCES_LIB})
set_target_properties(rudp PROPERTIES PUBLIC_HEADER include/rudp.h)
target_include_directories(rudp PUBLIC ${includes_list_lib})
target_link_libraries(rudp ${Boost_LIBRARIES})

# Test Definition
# add_compile_definitions(DEBUG)
set(includes_list_test "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_executable(test_send "${CMAKE_CURRENT_SOURCE_DIR}/test/test_send.c")
add_executable(test_recv "${CMAKE_CURRENT_SOURCE_DIR}/test/test_recv.c")
add_executable(test_connection "${CMAKE_CURRENT_SOURCE_DIR}/test/test_connection.cpp" ${SOURCES_LIB})

target_include_directories(test_send PUBLIC ${includes_list_test})
target_include_directories(test_recv PUBLIC ${includes_list_test})
target_include_directories(test_connection PUBLIC ${includes_list_lib} "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(test_send rudp)
target_link_libraries(test_recv rudp)
target_link_libraries(test_connection rudp)
