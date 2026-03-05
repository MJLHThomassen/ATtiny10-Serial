# Use a lightweight Alpine base image
FROM mcr.microsoft.com/devcontainers/base:ubuntu

# Download and install AVR8 GNU toolchain
ENV AVR_TOOLCHAIN_URL=https://ww1.microchip.com/downloads/aemDocuments/documents/DEV/ProductDocuments/SoftwareTools/avr8-gnu-toolchain-4.0.0.52-linux.any.x86_64.tar.gz
ENV AVR_INSTALL_DIR=/opt/avr8-gnu-toolchain-linux_x86_64
RUN mkdir -p ${AVR_INSTALL_DIR} \
    && wget -qO /tmp/avr8-gnu-toolchain-linux_x86_64.tar.gz "${AVR_TOOLCHAIN_URL}" \
    && tar -xzf /tmp/avr8-gnu-toolchain-linux_x86_64.tar.gz -C /opt \
    && rm /tmp/avr8-gnu-toolchain-linux_x86_64.tar.gz \
    && ln -s ${AVR_INSTALL_DIR}/bin/avr-gcc /usr/local/bin/avr-gcc \
    && ln -s ${AVR_INSTALL_DIR}/bin/avr-g++ /usr/local/bin/avr-g++ \
    && ln -s ${AVR_INSTALL_DIR}/bin/avr-size /usr/local/bin/avr-size \
    && ln -s ${AVR_INSTALL_DIR}/bin/avr-objcopy /usr/local/bin/avr-objcopy

# Install avrdude for flashing AVR microcontrollers
RUN apt-get update && \
    apt-get install -y \
        avrdude \
        usbutils && \
    rm -rf /var/lib/apt/lists/*

# Ensure the new toolchain is on PATH
ENV PATH="${AVR_INSTALL_DIR}/bin:${PATH}"