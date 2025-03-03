set(SOURCES
	main.cpp
	Log.cpp
	Game.cpp
	Utils/LinkerUtils.cpp
	Utils/FileUtils.cpp

	# Systems
	Systems/RenderingSys.cpp
	Systems/ResourceSys/ResourceSys.cpp
	Systems/GameplaySys.cpp
	Systems/EventSys.cpp
	Systems/InputSys.cpp
	Systems/ResourceSys/Obj/ObjResource.cpp
	Systems/ResourceSys/Obj/WavefrontLoader.cpp
	Systems/ResourceSys/Obj/GltfLoader.cpp
	Systems/ResourceSys/Obj/GPUBuffer.cpp
	Systems/ResourceSys/Obj/ObjMesh.cpp
	Systems/ResourceSys/Obj/ObjAnimation.cpp
	Systems/ResourceSys/ShaderResource.cpp
	Systems/PhysicsSys.cpp
	Systems/AnimationSys.cpp

	# Entities
	Entities/PlayerEntity.cpp
	Entities/CameraEntity.cpp
)

set(HEADERS
	Game.hpp
	Log.hpp
	Constants.hpp

	Utils/getopt.h
	Utils/FileUtils.hpp
	Utils/TupleUtils.hpp

	# Systems
	Systems/RenderingSys.hpp
	Systems/GameplaySys.hpp
	Systems/EventSys.hpp
	Systems/InputSys.hpp
	Systems/ResourceSys/ResourceSys.hpp
	Systems/ResourceSys/Obj/ObjResource.hpp
	Systems/ResourceSys/Obj/ObjLoader.hpp
	Systems/ResourceSys/Obj/WavefrontLoader.hpp
	Systems/ResourceSys/Obj/GltfLoader.hpp
	Systems/ResourceSys/Obj/GPUBuffer.hpp
	Systems/ResourceSys/Obj/ObjMesh.hpp
	Systems/ResourceSys/Obj/ObjMaterial.hpp
	Systems/ResourceSys/Obj/ObjAnimation.hpp
	Systems/ResourceSys/ShaderResource.hpp
	Systems/PhysicsSys.hpp
	Systems/AnimationSys.hpp

	# Entities
	Entities/Entity.hpp
	Entities/EntityRegistry.hpp
	Entities/EntityFilter.hpp
	Entities/PlayerEntity.hpp
	Entities/CameraEntity.hpp

	# Components
	Components/PositionComp.hpp
	Components/RenderableComp.hpp
	Components/CameraInfoComp.hpp
	Components/AnimationComp.hpp
	Components/MotionComp.hpp
)

# Packages
find_package(SDL2 REQUIRED CONFIG)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(tinyobjloader REQUIRED)
find_package(TinyGLTF REQUIRED)

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
	PRIVATE TinyGLTF::TinyGLTF
)

target_compile_definitions(Vroom
    PRIVATE
        # If in debug configuration, define DEBUG
        $<$<CONFIG:Debug>:IS_DEBUG>
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
