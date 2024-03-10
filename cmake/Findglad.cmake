include(FindPackageHandleStandardArgs)

if(NOT glad_FOUND)
    # Add glad static library target
    set(glad_DIR ${PROJECT_SOURCE_DIR}/external/glad)
    set(glad_INCLUDE_DIRS ${glad_DIR}/include)
    set(glad_SOURCES ${glad_DIR}/src/glad.c)
    add_library(glad STATIC ${glad_SOURCES})
    target_include_directories(glad PRIVATE ${glad_INCLUDE_DIRS})
    
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
