cmake_minimum_required(VERSION 3.20)

project(retrOS LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SHARED_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/shared")

add_subdirectory(bootloader)
add_subdirectory(kernel)

add_custom_target(iso
	COMMAND bash "${CMAKE_SOURCE_DIR}/tools/package_iso.sh"
	DEPENDS bootloader kernel
	COMMENT "Packaging retrOS ISO"
)

add_custom_target(run
	COMMAND bash "${CMAKE_SOURCE_DIR}/tools/run_vm.sh"
	DEPENDS iso
	COMMENT "Launching retrOS in QEMU"
)