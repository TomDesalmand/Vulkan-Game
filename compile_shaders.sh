#!/bin/bash

# Check if glslangValidator is installed
if ! command -v glslangValidator &> /dev/null; then
  echo -e "\033[1;30;41mError: glslangValidator is not installed.\033[0m"
  exit 1
fi

# List of shaders
VERT_SHADERS=("shaders/simple_shader.vert")
FRAG_SHADERS=("shaders/simple_shader.frag")

# Function to compile a shader
compile_shader() {
  local SHADER_FILE=$1
  local EXTENSION=$2
  local OUTPUT_FILE="${SHADER_FILE%.*}.${EXTENSION}.spv" # Replace file extension with .spv

  echo -e "\033[1;30;43mCompiling $SHADER_FILE...\033[0m"
  glslangValidator -V "$SHADER_FILE" -o "$OUTPUT_FILE"

  if [ $? -eq 0 ]; then
    echo -e "\033[1;30;42mCompiled successfully: $OUTPUT_FILE\033[0m"
  else
    echo -e "\033[1;30;41mFailed to compile: $SHADER_FILE\033[0m"
    exit 1
  fi
}

# Compile vertex shaders
for VERT_SHADER in "${VERT_SHADERS[@]}"; do
  if [ -f "$VERT_SHADER" ]; then
    compile_shader "$VERT_SHADER" "vert"
  else
    echo -e "\033[1;30;41mVertex shader not found: $VERT_SHADER\033[0m"
  fi
done

# Compile fragment shaders
for FRAG_SHADER in "${FRAG_SHADERS[@]}"; do
  if [ -f "$FRAG_SHADER" ]; then
    compile_shader "$FRAG_SHADER" "frag"
  else
    echo -e "\033[1;30;41mFragment shader not found: $FRAG_SHADER\033[0m"
  fi
done

echo -e "\033[1;30;42mAll shaders compiled successfully.\033[0m"
exit 0
