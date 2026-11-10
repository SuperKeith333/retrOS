
find_program(LLD_EXECUTABLE NAMES ld.lld REQUIRED)

set(KERNEL_SOURCE
	"${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)

set(KERNEL_OBJECT
	"${CMAKE_CURRENT_BINARY_DIR}/main.o"
)

set(KERNEL_IMAGE
	"${CMAKE_CURRENT_BINARY_DIR}/kernel.elf"
)

add_custom_command(
	OUTPUT "${KERNEL_IMAGE}"

	COMMAND "${CMAKE_CXX_COMPILER}"
		--target=x86_64-unknown-none-elf
		-std=c++23
		-ffreestanding
		-fno-exceptions
		-fno-rtti
		-fno-stack-protector
		-fno-pic
		-mno-red-zone
		-c "${KERNEL_SOURCE}"
		-o "${KERNEL_OBJECT}"

	COMMAND "${LLD_EXECUTABLE}"
		-flavor gnu
		-T "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld"
		--entry=kernel_main
		--build-id=none
		-o "${KERNEL_IMAGE}"
		"${KERNEL_OBJECT}"

	DEPENDS
		"${KERNEL_SOURCE}"
		"${CMAKE_CURRENT_SOURCE_DIR}/linker.ld"

	COMMENT "Building kernel"
	VERBATIM
)

add_custom_target(kernel ALL
	DEPENDS "${KERNEL_IMAGE}"
)
