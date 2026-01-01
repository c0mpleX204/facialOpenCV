cmake_minimum_required(VERSION 3.15)
project(facialOpenCV LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV (use -DCMAKE_TOOLCHAIN_FILE=.../vcpkg/scripts/buildsystems/vcpkg.cmake when using vcpkg)
find_package(OpenCV REQUIRED COMPONENTS core imgproc highgui dnn face)

add_executable(privacy_protector src/main.cpp src/face_detector.cpp)
target_include_directories(privacy_protector PRIVATE ${CMAKE_SOURCE_DIR}/src ${OpenCV_INCLUDE_DIRS})
target_link_libraries(privacy_protector PRIVATE ${OpenCV_LIBS})

## scan_cameras target removed: source file deleted (no longer needed)

message(STATUS "OpenCV include: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV libs: ${OpenCV_LIBS}")
