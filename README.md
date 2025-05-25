# Image Processing Project

**Authors:** Tom Hausmann & Amel Boulhamane


## Overview

This project is a C image processing application with both terminal and GUI interfaces, focused on BMP images (8-bit and 24-bit). The GUI version provides an intuitive graphical interface built with GTK3. 

---

## Features

- **BMP 8-bit and 24-bit support**
  - Load and save BMP images (grayscale and color)
  - Display image information (dimensions, color depth, etc.)

- **Image Filters**
  - Negative
  - Brightness adjustment
  - Black and white (threshold)
  - Box blur
  - Gaussian blur
  - Outline
  - Emboss
  - Sharpen

- **Histogram Equalization**
  - Enhance contrast for both grayscale and color images (YUV space for color)

---

## How to Use

### In the terminal 
1. **Build the Project**
- Use GCC or your preferred C compiler to build the project.
    - Example with GCC:
      ```
      gcc main.c bmp8.c bmp24.c equalize8.c equalize24.c -o image_processor -lm
      ```

2. **Run the Program**
   - Launch the executable (e.g., `./project_c_Amel_Tom` or `./untitled` depending on your build).
   - Follow the on-screen menu to:
     - Open a BMP image
     - Apply filters or histogram equalization
     - Save the processed image
     - View image information


---

## File Structure

### Main Application Files
- `main.c`: Terminal-based main menu and user interface
- `main_gui.c`: GUI-based main application using GTK+
- `Makefile.gui`: Build configuration for GUI application
- `demo_gui.sh`: Demo script to launch the GUI

### Core Image Processing Library (`project_c_amel_tom_int1/`)
- `bmp8.c` / `bmp8.h`: 8-bit BMP image handling and filters
- `bmp24.c` / `bmp24.h`: 24-bit BMP image handling and filters
- `equalize8.c` / `equalize8.h`: Histogram equalization for grayscale images
- `equalize24.c` / `equalize24.h`: Histogram equalization for color images

### Documentation & Testing
- `GUI_README.md`: Detailed GUI user documentation
- `IMPLEMENTATION_SUMMARY.md`: Technical implementation details
- `test_image_processing.c`: Automated filter tests (for development)
- `*.bmp`: Sample test images

### Build Files
- `CMakeLists.txt`: CMake build configuration
- `image_gui`: Compiled GUI executable

---


