
find_program(LLD_EXECUTABLE NAMES ld.lld REQUIRED)
find_program(OBJCOPY_EXECUTABLE NAMES objcopy REQUIRED)

set(BOOTLOADER_ELF "${CMAKE_CURRENT_BINARY_DIR}/bootloader.so")
set(BOOTLOADER_IMAGE "${CMAKE_CURRENT_BINARY_DIR}/BOOTX64.EFI")

set(GNU_EFI_LINKER_SCRIPT "/usr/lib/elf_x86_64_efi.lds")
set(GNU_EFI_CRT "/usr/lib/crt0-efi-x86_64.o")

# CMake-managed compilation
add_library(bootloader_obj OBJECT
	src/main.cpp
)

target_compile_features(bootloader_obj PRIVATE cxx_std_23)

target_compile_options(bootloader_obj PRIVATE
	-ffreestanding
	-fpic
	-fshort-wchar
	-fno-exceptions
	-fno-rtti
	-fno-stack-protector
	-mno-red-zone
)

target_compile_definitions(bootloader_obj PRIVATE
	GNU_EFI_USE_MS_ABI
)

target_include_directories(bootloader_obj PRIVATE
	/usr/include/efi
	/usr/include/efi/x86_64
	"${SHARED_INCLUDE_DIR}"
)

# Link and convert to EFI
add_custom_command(
	OUTPUT "${BOOTLOADER_IMAGE}"

	COMMAND "${LLD_EXECUTABLE}"
		-nostdlib
		-znocombreloc
		-T "${GNU_EFI_LINKER_SCRIPT}"
		-shared
		-Bsymbolic
		-L/usr/lib
		"${GNU_EFI_CRT}"
		$<TARGET_OBJECTS:bootloader_obj>
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
		bootloader_obj
		$<TARGET_OBJECTS:bootloader_obj>
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
