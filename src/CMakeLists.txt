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
	Systems/ResourceSys/Obj/ObjOrientedBoundingBox.cpp
	Systems/ResourceSys/Obj/ObjMesh.cpp
	Systems/ResourceSys/Obj/ObjImage.cpp
	Systems/ResourceSys/Obj/ObjTexture.cpp
	Systems/ResourceSys/Obj/Animation/AnimationContainer.cpp
	Systems/ResourceSys/Obj/Animation/Animation.cpp
	Systems/ResourceSys/Obj/Animation/Skin.cpp
	Systems/ResourceSys/ShaderResource.cpp
	Systems/PhysicsSys.cpp
	Systems/AnimationSys.cpp
	Systems/UISys.cpp

	# Entities
	Entities/CameraEntity.cpp
	Entities/LightEntity.cpp
	Entities/PlayerEntity.cpp
	Entities/PropEntity.cpp
	Entities/SkyboxEntity.cpp
)

set(HEADERS
	Game.hpp
	Log.hpp
	Constants.hpp

	Utils/FileUtils.hpp
	Utils/getopt.h
	Utils/Identifiable.hpp
	Utils/MathUtils.hpp
	Utils/StringIndexor.hpp
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
	Systems/ResourceSys/Obj/ObjOrientedBoundingBox.hpp
	Systems/ResourceSys/Obj/ObjMesh.hpp
	Systems/ResourceSys/Obj/ObjImage.hpp
	Systems/ResourceSys/Obj/ObjTexture.hpp
	Systems/ResourceSys/Obj/ObjMaterial.hpp
	Systems/ResourceSys/Obj/Animation/AnimationNode.hpp
	Systems/ResourceSys/Obj/Animation/AnimationContainer.hpp
	Systems/ResourceSys/Obj/Animation/Animation.hpp
	Systems/ResourceSys/Obj/Animation/Skin.hpp
	Systems/ResourceSys/ShaderResource.hpp
	Systems/PhysicsSys.hpp
	Systems/AnimationSys.hpp
	Systems/UISys.hpp

	# Entities
	Entities/Entity.hpp
	Entities/EntityRegistry.hpp
	Entities/EntityFilter.hpp
	Entities/CameraEntity.hpp
	Entities/LightEntity.hpp
	Entities/PlayerEntity.hpp
	Entities/PropEntity.hpp
	Entities/SkyboxEntity.hpp

	# Components
	Components/AnimationComp.hpp
	Components/CameraInfoComp.hpp
	Components/FrictionComp.hpp
	Components/GravityComp.hpp
	Components/LightComp.hpp
	Components/PositionComp.hpp
	Components/RenderableComp.hpp
	Components/PhysicsComp.hpp
)

set(EXTRA_SOURCES
	# imgui
	${CMAKE_CURRENT_BINARY_DIR}/res/bindings/imgui_impl_sdl2.cpp
	${CMAKE_CURRENT_BINARY_DIR}/res/bindings/imgui_impl_opengl3.cpp
)

set(EXTRA_INCLUDES
	${CMAKE_CURRENT_BINARY_DIR}/res/bindings
)

# Packages
find_package(SDL2 REQUIRED CONFIG)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(tinyobjloader REQUIRED)
find_package(TinyGLTF REQUIRED)
find_package(imgui REQUIRED)
find_package(Eigen3 REQUIRED)

add_executable(
	Vroom
	${SOURCES}
	${HEADERS}
	${EXTRA_SOURCES}
)

target_include_directories(
	Vroom
	PRIVATE ${EXTRA_INCLUDES}
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
	PRIVATE imgui::imgui
	PRIVATE Eigen3::Eigen
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
