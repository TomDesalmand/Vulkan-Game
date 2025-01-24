#!/bin/bash

# Check if running as root
if [ "$(id -u)" != "0" ]; then
  echo -e "\033[1;30;41mPlease run this script as root.\033[0m"
  exit 1
fi

# Update the system
echo -e "\033[1;30;43mUpdating system...\033[0m"
apt-get update -y && apt-get upgrade -y
echo -e "\033[1;30;42mSystem updated.\033[0m"

# Install development tools
echo -e "\033[1;30;43mInstalling development tools...\033[0m"
apt-get install -y gcc pkg-config cmake build-essential git
if command -v gcc && command -v pkg-config && command -v cmake; then
  echo -e "\033[1;30;42mDevelopment tools installed.\033[0m"
else
  echo -e "\033[1;30;41mDevelopment tools installation failed.\033[0m"
  exit 1
fi

# Install GLFW
echo -e "\033[1;30;43mInstalling GLFW...\033[0m"
apt-get install -y libglfw3-dev libglfw3-doc libglfw3
if pkg-config --exists glfw3; then
  echo -e "\033[1;30;42mGLFW installed.\033[0m"
else
  echo -e "\033[1;30;41mGLFW installation failed.\033[0m"
  exit 1
fi

# Install Vulkan dependencies
echo -e "\033[1;30;43mInstalling Vulkan dependencies...\033[0m"
apt-get install -y libvulkan-dev libvulkan1 mesa-vulkan-drivers vulkan-tools
if command -v vulkaninfo &> /dev/null; then
  echo -e "\033[1;30;42mVulkan dependencies installed.\033[0m"
else
  echo -e "\033[1;30;41mFailed to install Vulkan dependencies.\033[0m"
  exit 1
fi

# Optionally install Vulkan-Headers from source
echo -e "\033[1;30;43mInstalling Vulkan-Headers from source...\033[0m"
cd ~
git clone https://github.com/KhronosGroup/Vulkan-Headers.git
cd Vulkan-Headers
mkdir build && cd build
cmake .. && make && make install
if [ $? -eq 0 ]; then
  echo -e "\033[1;30;42mVulkan-Headers installed.\033[0m"
else
  echo -e "\033[1;30;41mVulkan-Headers installation failed.\033[0m"
  exit 1
fi

# Install GLSL and its compiler
echo -e "\033[1;30;43mInstalling GLSL and its compiler...\033[0m"
apt-get install -y glslang-tools
if command -v glslangValidator &> /dev/null; then
  echo -e "\033[1;30;42mGLSL compiler installed.\033[0m"
else
  echo -e "\033[1;30;41mGLSL compiler installation failed.\033[0m"
  exit 1
fi

# Cleanup
echo -e "\033[1;30;43mCleaning up...\033[0m"
rm -rf ~/Vulkan-Headers

echo -e "\033[1;30;42mInstallation complete.\033[0m"
exit 0
