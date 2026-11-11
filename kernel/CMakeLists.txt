
find_program(LLD_EXECUTABLE NAMES ld.lld REQUIRED)

set(KERNEL_IMAGE "${CMAKE_CURRENT_BINARY_DIR}/kernel.elf")

# CMake-managed compilation
add_library(kernel_obj OBJECT
	src/main.cpp
)

file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.cpp")

target_compile_features(kernel_obj PRIVATE cxx_std_23)

target_compile_options(kernel_obj PRIVATE
	-ffreestanding
	-fno-exceptions
	-fno-rtti
	-fno-stack-protector
	-fno-pic
	-mno-red-zone
)

target_include_directories(kernel_obj PRIVATE
	"${SHARED_INCLUDE_DIR}"
    "include"
)

# Link kernel
add_custom_command(
    OUTPUT "${KERNEL_IMAGE}"
    COMMAND "${LLD_EXECUTABLE}"
        -flavor gnu
        -T "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld"
        --entry=kernel_main
        --build-id=none
        -o "${KERNEL_IMAGE}"
        $<TARGET_OBJECTS:kernel_obj>
    DEPENDS
        kernel_obj
        "{SOURCES}"
        $<TARGET_OBJECTS:kernel_obj>
        "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld"
    COMMENT "Building kernel"
    VERBATIM
)

add_custom_target(kernel ALL
	DEPENDS "${KERNEL_IMAGE}"
)
