find_program(LLD_LINK_EXECUTABLE NAMES lld-link REQUIRED)

set(BOOTLOADER_SOURCE
	"${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)

set(BOOTLOADER_OBJECT
	"${CMAKE_CURRENT_BINARY_DIR}/main.obj"
)

set(BOOTLOADER_IMAGE
	"${CMAKE_CURRENT_BINARY_DIR}/BOOTX64.EFI"
)

add_custom_command(
	OUTPUT "${BOOTLOADER_IMAGE}"

	COMMAND "${CMAKE_CXX_COMPILER}"
		--target=x86_64-pc-win32-coff
		-std=c++23
		-ffreestanding
		-fno-exceptions
		-fno-rtti
		-fno-stack-protector
		-mno-red-zone
		-c "${BOOTLOADER_SOURCE}"
		-o "${BOOTLOADER_OBJECT}"

	COMMAND "${LLD_LINK_EXECUTABLE}"
		/subsystem:efi_application
		/entry:efi_main
		/nodefaultlib
		/machine:x64
		"/out:${BOOTLOADER_IMAGE}"
		"${BOOTLOADER_OBJECT}"

	DEPENDS "${BOOTLOADER_SOURCE}"
	COMMENT "Building UEFI bootloader"
	VERBATIM
)

add_custom_target(bootloader ALL
	DEPENDS "${BOOTLOADER_IMAGE}"
)