# Vroom
Vroom!

# Dependencies
* [Conan](https://github.com/conan-io/conan)
* [CMake](https://cmake.org/)

For full list of dependencies, see [`conanfile.txt`](https://github.com/fordcars/Vroom/blob/main/conanfile.txt).

## Building
```bash
# Setup
conan install . --output-folder=build --build=missing --settings=build_type=Debug # Or Release
cmake --preset conan-default

# Build
cmake --build --preset conan-debug # Or Release
# Or cd build && make, or use Microsoft Visual Studio, etc.
```
