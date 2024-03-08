set(SOURCES
	main.cpp
)

set(HEADERS
    getopt.h
)

add_executable(
	Vroom
	${SOURCES}
	${HEADERS}
)

# Packages
find_package(SDL2 REQUIRED CONFIG)

target_include_directories(
	Vroom
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(
	Vroom
	PRIVATE ${SDL2_LIBRARIES}
)
