include(FindPackageHandleStandardArgs)

# Check if package is already found
if(NOT glad_FOUND)
    # Add glad static library target
    set(glad_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/external/glad/include)
    find_path(glad_SOURCES NAMES glad.c PATHS ${PROJECT_SOURCE_DIR}/external/glad/src)
    add_library(glad STATIC ${glad_SOURCES})
    target_include_directories(glad PUBLIC ${glad_INCLUDE_DIRS})

    # Not sure why, but this is required to fix
    # 'can not determine linker language' error
    set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)
    
    # Set glad_LIBRARIES to target library
    set(glad_LIBRARIES glad)

    # Check if glad was found properly
    if(glad_INCLUDE_DIRS AND glad_SOURCES)
        set(glad_FOUND TRUE)
    else()
        set(glad_FOUND FALSE)
    endif()

    # Provide results to parent scope
    set(glad_FOUND ${glad_FOUND} PARENT_SCOPE)
    set(glad_INCLUDE_DIRS ${glad_INCLUDE_DIRS} PARENT_SCOPE)
    set(glad_LIBRARIES ${glad_LIBRARIES} PARENT_SCOPE)
endif()

# Support various find_package args, including REQUIRED
find_package_handle_standard_args(glad
  DEFAULT_MSG
  glad_FOUND
)
