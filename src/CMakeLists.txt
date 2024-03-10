set(SOURCES
	main.cpp
	Game.cpp
)

set(HEADERS
    getopt.h
	Game.hpp
	Log.hpp
	Constants.hpp
)

# Packages
find_package(SDL2 REQUIRED CONFIG)
find_package(glad REQUIRED)

add_executable(
	Vroom
	${SOURCES}
	${HEADERS}
)

target_include_directories(
	Vroom
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${SDL2_INCLUDE_DIRS}
	PRIVATE ${glad_INCLUDE_DIRS}
)

target_link_libraries(
	Vroom
	PRIVATE ${SDL2_LIBRARIES}
	PRIVATE ${glad_LIBRARIES}
)
