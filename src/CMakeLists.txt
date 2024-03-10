set(SOURCES
	main.cpp
	Game.cpp
	Systems/RenderingSys.cpp
	Entities/CarEntity.cpp

	Entities/TempEntity.cpp
)

set(HEADERS
    Utils/getopt.h
	Game.hpp
	Log.hpp
	Constants.hpp

	Systems/System.hpp
	Systems/SystemEntities.hpp
	Systems/RenderingSys.hpp
	Entities/Entity.hpp
	Entities/EntityRegistry.hpp
	Entities/CarEntity.hpp
	Components/PositionComp.hpp

	Entities/TempEntity.hpp
	Components/TempComp.hpp
)

# Packages
find_package(SDL2 REQUIRED CONFIG)
find_package(glad REQUIRED)
find_package(glm REQUIRED)

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
	PRIVATE glm::glm
)
