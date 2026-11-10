
find_program(LLD_EXECUTABLE NAMES ld.lld REQUIRED)
find_program(OBJCOPY_EXECUTABLE NAMES objcopy REQUIRED)

set(BOOTLOADER_SOURCE
	"${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)

set(BOOTLOADER_OBJECT
	"${CMAKE_CURRENT_BINARY_DIR}/main.o"
)

set(BOOTLOADER_ELF
	"${CMAKE_CURRENT_BINARY_DIR}/bootloader.so"
)

set(BOOTLOADER_IMAGE
	"${CMAKE_CURRENT_BINARY_DIR}/BOOTX64.EFI"
)

set(GNU_EFI_LINKER_SCRIPT
	"/usr/lib/elf_x86_64_efi.lds"
)

set(GNU_EFI_CRT
	"/usr/lib/crt0-efi-x86_64.o"
)

add_custom_command(
	OUTPUT "${BOOTLOADER_IMAGE}"

	COMMAND "${CMAKE_CXX_COMPILER}"
		-std=c++23
		-target x86_64-unknown-linux-gnu
		-ffreestanding
		-fpic
		-fshort-wchar
		-fno-exceptions
		-fno-rtti
		-fno-stack-protector
		-mno-red-zone
		-DGNU_EFI_USE_MS_ABI
		-I/usr/include/efi
		-I/usr/include/efi/x86_64
		-c "${BOOTLOADER_SOURCE}"
		-o "${BOOTLOADER_OBJECT}"

	COMMAND "${LLD_EXECUTABLE}"
		-nostdlib
		-znocombreloc
		-T "${GNU_EFI_LINKER_SCRIPT}"
		-shared
		-Bsymbolic
		-L/usr/lib
		"${GNU_EFI_CRT}"
		"${BOOTLOADER_OBJECT}"
		-lgnuefi
		-lefi
		-o "${BOOTLOADER_ELF}"

	COMMAND "${OBJCOPY_EXECUTABLE}"
		-j .text
		-j .sdata
		-j .data
		-j ".rodata*"
		-j .dynamic
		-j .dynsym
		-j .rel
		-j .rela
		-j ".rela.*"
		-j .reloc
		-O efi-app-x86_64
		"${BOOTLOADER_ELF}"
		"${BOOTLOADER_IMAGE}"

	DEPENDS
		"${BOOTLOADER_SOURCE}"
		"${GNU_EFI_LINKER_SCRIPT}"
		"${GNU_EFI_CRT}"
		"/usr/lib/libgnuefi.a"
		"/usr/lib/libefi.a"

	COMMENT "Building GNU-EFI bootloader"
	VERBATIM
)

add_custom_target(bootloader ALL
	DEPENDS "${BOOTLOADER_IMAGE}"
)
