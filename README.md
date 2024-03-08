# Vroom
Vroom!

# Dependencies
* [Conan](https://github.com/conan-io/conan)
* [CMake](https://cmake.org/)

For full list of dependencies, see `conanfile.txt`.

## Building
```bash
conan install . --output-folder=build --build=missing --settings=build_type=Debug # Or Release

# For CMake >= 3.23:
cmake --preset conan-default
# For CMake < 3.23, Conan will output an example command

cd build && make # Or run cmake --build, build in Microsoft Visual Studio, etc.
```
