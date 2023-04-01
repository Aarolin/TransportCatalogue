cmake_minimum_required(VERSION 3.11)

project(transport_catalogue)
set(CMAKE_CXX_STANDARD 17)

if(CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

set(SOURCE_DIR src)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS "${SOURCE_DIR}/transport_catalogue.proto")

set(TRANSPORT_CATALOGUE_FILES ${SOURCE_DIR}/domain.cpp ${SOURCE_DIR}/domain.h ${SOURCE_DIR}/geo.cpp ${SOURCE_DIR}/geo.h ${SOURCE_DIR}/json.cpp ${SOURCE_DIR}/json.h ${SOURCE_DIR}/json_builder.cpp ${SOURCE_DIR}/json_builder.h ${SOURCE_DIR}/json_reader.cpp ${SOURCE_DIR}/json_reader.h ${SOURCE_DIR}/map_renderer.cpp ${SOURCE_DIR}/map_renderer.h ${SOURCE_DIR}/serialization.h ${SOURCE_DIR}/serialization.cpp ${SOURCE_DIR}/transport_catalogue.proto ${SOURCE_DIR}/svg.cpp ${SOURCE_DIR}/svg.h ${SOURCE_DIR}/graph.h ${SOURCE_DIR}/ranges.h ${SOURCE_DIR}/request_handler.cpp ${SOURCE_DIR}/request_handler.h ${SOURCE_DIR}/router.h ${SOURCE_DIR}/transport_router.cpp ${SOURCE_DIR}/transport_router.h ${SOURCE_DIR}/main.cpp ${SOURCE_DIR}/transport_catalogue.cpp ${SOURCE_DIR}/transport_catalogue.h)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TRANSPORT_CATALOGUE_FILES})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG}" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG}" "${Protobuf_LIBRARY_DEBUG}}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)


