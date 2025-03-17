from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.files import copy

class Vroom(ConanFile):
    name = "vroom"
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    
    def requirements(self):
        self.requires("sdl/[~2.26]")
        self.requires("glm/cci.20230113")
        self.requires("tinyobjloader/2.0.0-rc10")
        self.requires("tinygltf/[~2.9.0]")
        self.requires("imgui/[~1.91]")
        self.requires("eigen/[~3.4]")
    
    def layout(self):
        cmake_layout(self)

    def generate(self):
        extra_sources = [
            # Required by imgui
            "res/bindings/imgui_impl_sdl2.cpp",
            "res/bindings/imgui_impl_sdl2.h",
            "res/bindings/imgui_impl_opengl3.cpp",
            "res/bindings/imgui_impl_opengl3.h",
            "res/bindings/imgui_impl_opengl3_loader.h"
        ]
        for dep in self.dependencies.values():
            for f in extra_sources:
                copied = copy(self, f, dep.package_folder, self.build_folder + "/src")
                if len(copied) > 0:
                    print(f"Copied file(s): {copied}")
