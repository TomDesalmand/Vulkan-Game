#!/bin/bash

# Check if running as root
if ["$(id -u)" != "0"]; then
  echo -e "\033[1;30;41mPlease run this script as root.\033[0m"
  exit 84
fi

# Update
echo -e "\033[1;30;43mUpdating...\033[0m"
dnf update -y
dnf upgrade -y
echo -e "\033[1;30;42mUpdate Finished.\033[0m"

# Install required packages for pkg-config
echo -e "\033[1;30;43mInstalling pkg-config...\033[0m"
dnf install -y pkgconfig
if which pkg-config &> /dev/null; then
  echo -e "\033[1;30;42mpkg-config has been successfully installed.\033[0m"
else
  echo -e "\033[1;30;41mpkg-config installation failed.\033[0m"
  exit 84
fi

# Install required packages for Cmake
echo -e "\033[1;30;43mInstalling Cmake...\033[0m"
dnf install -y cmake
if which cmake &> /dev/null; then
  echo -e "\033[1;30;42mCMake has been successfully installed.\033[0m"
else
  echo -e "\033[1;30;41mCMake installation failed.\033[0m"
  exit 84
fi

# Install required packages for GLFW
echo -e "\033[1;30;43mInstalling GLFW...\033[0m"
dnf install -y glfw-devel
if pkg-config --exists glfw3; then
  echo -e "\033[1;30;42mGLFW has been successfully installed.\033[0m"
else
  echo -e "\033[1;30;41mGLFW installation failed.\033[0m"
  exit 84
fi

# Install required packages for building the Vulkan SDK
echo -e "\033[1;30;43mInstalling Vulkan SDK...\033[0m"
dnf install -y git vulkan-devel
cd ~
git clone https://github.com/KhronosGroup/Vulkan-Headers.git
cd Vulkan-Headers
mkdir build && cd build
cmake ..
make && make install
cd ~
rm -rf Vulkan-Headers
if which vulkaninfo &>/dev/null; then
  echo -e "\033[1;30;42mVulkan SDK has been successfully installed.\033[0m"
else
  echo -e "\033[1;30;41mVulkan SDK installation failed.\033[0m"
  exit 84
fi
