# Use a lightweight Alpine base image
FROM mcr.microsoft.com/devcontainers/base:ubuntu

# Download and install AVR-GCC toolchain
ENV AVR_TOOLCHAIN_URL=https://github.com/ZakKemble/avr-gcc-build/releases/download/v15.2.0-1/avr-gcc-15.2.0-x64-linux.tar.bz2
ENV AVR_INSTALL_DIR=/opt/avr-gcc
RUN mkdir -p ${AVR_INSTALL_DIR} \
    && wget -qO /tmp/avr-gcc-15.2.0-x64-linux.tar.bz2 "${AVR_TOOLCHAIN_URL}" \
    && tar -xjf /tmp/avr-gcc-15.2.0-x64-linux.tar.bz2 -C ${AVR_INSTALL_DIR} --strip-components=1 \
    && rm /tmp/avr-gcc-15.2.0-x64-linux.tar.bz2


# Install Cmake and avrdude for flashing AVR microcontrollers
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null \
    && echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ '$(lsb_release -cs)' main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null \
    && apt-get update \
    && apt-get install -y \
        avrdude \
        kitware-archive-keyring\
        cmake \
        usbutils \
    && rm -rf /var/lib/apt/lists/*

# Ensure the new toolchain is on PATH
ENV PATH="${AVR_INSTALL_DIR}/bin:${PATH}"