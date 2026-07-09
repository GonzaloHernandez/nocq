---
name: /configure
description: Instructions for performing a clean build configuration for the NOCQ project.
---

# Configuring and Cleaning NOCQ

This skill provides the standard procedure to clean existing build cache/artifacts and perform a fresh CMake configuration for the NOCQ project with the bundled Chuffed solver.

## Configuration Steps

To completely clean and reconfigure the project:

1. **Clean Existing Build Directory**:
   Remove the existing `build` directory to discard any cached configurations or compiled objects:
   ```bash
   rm -rf build
   ```

2. **Configure the Project**:
   Run CMake to generate build files, ensuring the bundled version of Chuffed is selected:
   ```bash
   cmake -B build -S . -DUSE_SYSTEM_CHUFFED=OFF
   ```
