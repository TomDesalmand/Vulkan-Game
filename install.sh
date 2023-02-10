#!/bin/bash

# Check if running as root
if [ "$(id -u)" != "0" ]; then
  echo -e "\033[1;30;41mPlease run this script as root.\033[0m"
  exit 1
fi

#Update
echo -e "\033[1;3043mUpdating...\033[0m"
apt-get update
apt-get upgrade

# Install required packages for Cmake
echo -e "\033[1;30;43mInstalling Cmake...\033[0m"
sudo apt-get install -y cmake
if which cmake &> /dev/null; then
  echo -e "\033[1;30;42mCMake has been successfully installed.\033[0m"
else
  echo -e "\033[1;30;41mCMake installation failed.\033[0m"
fi

# Install required packages for GLFW
echo -e "\033[1;30;43mInstalling GLFW...\033[0m"
sudo apt-get install -y libglfw3-dev libglfw3-doc libglfw3
if pkg-config --exists glfw3; then
  echo -e "\033[1;30;42mGLFW has been successfully installed.\033[0m"
else
  echo -e "\033[1;30;41mGLFW installation failed.\033[0m"
fi

# Install required packages for building the Vulkan SDK
echo -e "\033[1;30;43mInstalling GLFW...\033[0m"
apt-get install -y build-essential git libx11-dev libxcb1-dev libxcb-dri3-dev libxcb-present-dev libxcb-randr0-dev libxcb-sync-dev libxcb-xfixes0-dev libxcb-shape0-dev libxcb-xinerama0-dev libxcursor-dev libxcomposite-dev libxdamage-dev libvulkan-dev libvulkan1 mesa-vulkan-drivers vulkan-tools
cd ~
git clone https://github.com/KhronosGroup/Vulkan-Headers.git
cd Vulkan-Headers
mkdir build && cd build
cmake ..
make && make install
cd ~
sudo rm -r Vulkan-Headers
if hash vulkaninfo 2>/dev/null; then
  echo -e "\033[1;30;42mVulkan SDK installed successfully.\033[0m"
else
  echo -e "\033[1;30;41mVulkan SDK installation failed.\033[0m"
fi
