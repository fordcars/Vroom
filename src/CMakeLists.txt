set(SOURCES
	main.cpp
	Game.cpp

	# Systems
	Systems/RenderingSys.cpp
	Systems/ResourceSys.cpp

	# Entities
	Entities/CarEntity.cpp
)

set(HEADERS
    Utils/getopt.h
	Game.hpp
	Log.hpp
	Constants.hpp

	# Systems
	Systems/System.hpp
	Systems/SystemEntities.hpp
	Systems/RenderingSys.hpp
	Systems/ResourceSys.hpp

	# Entities
	Entities/Entity.hpp
	Entities/EntityRegistry.hpp
	Entities/CarEntity.hpp

	# Components
	Components/PositionComp.hpp
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

# Copy resources after buildin
add_custom_command(
    TARGET Vroom POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${RESOURCE_SRC_DIR}
        ${RESOURCE_DEST_DIR}
    COMMENT "Copying resources to output directory"
)
add_custom_target(copy_resources ALL DEPENDS ${RESOURCE_DEST_DIR})
