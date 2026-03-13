# Target System Information
set(CMAKE_SYSTEM_NAME       Generic-ELF)
set(CMAKE_SYSTEM_PROCESSOR  avr)

# Compilers
set(CMAKE_C_COMPILER    avr-gcc)
set(CMAKE_CXX_COMPILER  avr-g++)
set(CMAKE_ASM_COMPILER  avr-gcc)

# Setup search paths (prevents CMake from finding host libraries/headers)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# AVR tools
set(CACHE{AVRDUDE}  VALUE avrdude)
