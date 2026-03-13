cmake_minimum_required(VERSION 4.2)

# Variables regarding the AVR chip and programmer
set(MCU attiny10)
set(F_CPU 1000000)
set(PROG_TYPE avrispmkII)

project(shared-buffer C ASM)

# Compiler flags
set(CSTANDARD "-std=c23")
set(CDEBUG    "-g")
set(CWARN     "-Wall -Wstrict-prototypes -Wl,--gc-sections -Wl,--relax")
set(CTUNING   "-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -ffunction-sections -fdata-sections")
set(COPT      "-Os -lm -lprintf_flt")
set(CMCU      "-mmcu=${MCU}")

set(CFLAGS   "${CMCU} ${CDEBUG} ${COPT} ${CWARN} ${CSTANDARD} ${CTUNING}")

set(CMAKE_C_FLAGS   "${CFLAGS}")
set(CMAKE_ASM_FLAGS   "${CFLAGS}")

# Project setup
add_executable(shared-buffer)
target_sources(shared-buffer
    PRIVATE
        main.c
        asm.S)
add_compile_definitions(F_CPU=${F_CPU})

# Compiling targets
add_custom_target(hex ALL ${CMAKE_OBJCOPY} -j .text -j .data -O ihex "${PROJECT_NAME}.elf" "${PROJECT_NAME}.hex" DEPENDS ${PROJECT_NAME})

add_custom_target(asm ${CMAKE_C_COMPILER} -S -c "${shared-buffer_SOURCE_DIR}/main.c" -o "main.s")
add_custom_target(dump ${CMAKE_OBJDUMP} -h -S "${PROJECT_NAME}.elf" > ${PROJECT_NAME}.lst DEPENDS ${PROJECT_NAME})
add_custom_target(flash ${AVRDUDE} -v -c ${PROG_TYPE} -p ${MCU} -U flash:w:${PROJECT_NAME}.hex DEPENDS hex)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PROJECT_NAME}.hex;${PROJECT_NAME}.eeprom;${PROJECT_NAME}.lst")