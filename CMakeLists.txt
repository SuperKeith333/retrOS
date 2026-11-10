cmake_minimum_required(VERSION 3.20)

project(retrOS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(bootloader)
add_subdirectory(kernel)

add_custom_target(iso
	COMMAND bash "${CMAKE_SOURCE_DIR}/tools/package_iso.sh"
	DEPENDS bootloader kernel
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	USES_TERMINAL
)

add_custom_target(run
	COMMAND bash "${CMAKE_SOURCE_DIR}/tools/run_vm.sh"
	DEPENDS iso
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	USES_TERMINAL
)