set(SOURCES
	main.cpp
	Log.cpp
	Game.cpp
	Utils/FileUtils.cpp

	# Systems
	Systems/RenderingSys.cpp
	Systems/ResourceSys/ResourceSys.cpp
	Systems/GameSys.cpp
	Systems/ResourceSys/Obj/ObjResource.cpp
	Systems/ResourceSys/Obj/GPUBuffer.cpp
	Systems/ResourceSys/Obj/ObjMesh.cpp
	Systems/ResourceSys/ShaderResource.cpp

	# Entities
	Entities/CarEntity.cpp
	Entities/CameraEntity.cpp
)

set(HEADERS
	Game.hpp
	Log.hpp
	Constants.hpp

	Utils/getopt.h
	Utils/FileUtils.hpp

	# Systems
	Systems/RenderingSys.hpp
	Systems/GameSys.hpp
	Systems/ResourceSys/ResourceSys.hpp
	Systems/ResourceSys/Obj/ObjResource.hpp
	Systems/ResourceSys/Obj/GPUBuffer.hpp
	Systems/ResourceSys/Obj/ObjMesh.hpp
	Systems/ResourceSys/Obj/ObjMaterial.hpp
	Systems/ResourceSys/ShaderResource.hpp

	# Entities
	Entities/Entity.hpp
	Entities/EntityRegistry.hpp
	Entities/EntityFilter.hpp
	Entities/CarEntity.hpp
	Entities/CameraEntity.hpp

	# Components
	Components/PositionComp.hpp
	Components/RenderableComp.hpp
	Components/CameraInfoComp.hpp
)

# Packages
find_package(SDL2 REQUIRED CONFIG)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(tinyobjloader REQUIRED)

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
	PRIVATE tinyobjloader::tinyobjloader
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
