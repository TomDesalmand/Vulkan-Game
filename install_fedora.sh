#!/bin/bash

# Check if running as root
if [ "$(id -u)" != "0" ]; then
  echo -e "\033[1;30;41mPlease run this script as root.\033[0m"
  exit 1
fi

# Update the system
echo -e "\033[1;30;43mUpdating system...\033[0m"
dnf update -y && dnf upgrade -y
echo -e "\033[1;30;42mSystem updated.\033[0m"

# Install GCC
echo -e "\033[1;30;43mInstalling GCC...\033[0m"
dnf install -y gcc
if command -v gcc &> /dev/null; then
  echo -e "\033[1;30;42mGCC installed successfully.\033[0m"
else
  echo -e "\033[1;30;41mGCC installation failed.\033[0m"
  exit 1
fi

# Install pkg-config
echo -e "\033[1;30;43mInstalling pkg-config...\033[0m"
dnf install -y pkgconfig
if command -v pkg-config &> /dev/null; then
  echo -e "\033[1;30;42mpkg-config installed successfully.\033[0m"
else
  echo -e "\033[1;30;41mpkg-config installation failed.\033[0m"
  exit 1
fi

# Install CMake
echo -e "\033[1;30;43mInstalling CMake...\033[0m"
dnf install -y cmake
if command -v cmake &> /dev/null; then
  echo -e "\033[1;30;42mCMake installed successfully.\033[0m"
else
  echo -e "\033[1;30;41mCMake installation failed.\033[0m"
  exit 1
fi

# Install GLFW
echo -e "\033[1;30;43mInstalling GLFW...\033[0m"
dnf install -y glfw-devel
if pkg-config --exists glfw3; then
  echo -e "\033[1;30;42mGLFW installed successfully.\033[0m"
else
  echo -e "\033[1;30;41mGLFW installation failed.\033[0m"
  exit 1
fi

# Install Vulkan dependencies and tools
echo -e "\033[1;30;43mInstalling Vulkan SDK and tools...\033[0m"
dnf install -y vulkan-devel vulkan-tools
if command -v vulkaninfo &> /dev/null; then
  echo -e "\033[1;30;42mVulkan SDK and tools installed successfully.\033[0m"
else
  echo -e "\033[1;30;41mFailed to install Vulkan SDK and tools.\033[0m"
  exit 1
fi

# Optionally install Vulkan-Headers from source
echo -e "\033[1;30;43mInstalling Vulkan-Headers from source (optional)...\033[0m"
cd ~
git clone https://github.com/KhronosGroup/Vulkan-Headers.git
cd Vulkan-Headers
mkdir build && cd build
cmake .. && make && make install
if [ $? -eq 0 ]; then
  echo -e "\033[1;30;42mVulkan-Headers installed successfully.\033[0m"
else
  echo -e "\033[1;30;41mVulkan-Headers installation failed.\033[0m"
  exit 1
fi

# Install GLSL and its compiler
echo -e "\033[1;30;43mInstalling GLSL and its compiler...\033[0m"
dnf install -y glslang
if command -v glslangValidator &> /dev/null; then
  echo -e "\033[1;30;42mGLSL compiler installed successfully.\033[0m"
else
  echo -e "\033[1;30;41mGLSL compiler installation failed.\033[0m"
  exit 1
fi

# Cleanup
echo -e "\033[1;30;43mCleaning up...\033[0m"
rm -rf ~/Vulkan-Headers
echo -e "\033[1;30;42mCleanup complete.\033[0m"

echo -e "\033[1;30;42mInstallation complete.\033[0m"
exit 0