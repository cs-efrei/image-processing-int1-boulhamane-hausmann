# Image Processing Project

**Authors:** Tom Hausmann & Amel Boulhamane

---

## Overview

This project is a C image processing terminal based programmed focused on BMP images (8-bit and 24-bit ). 

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

### Terminal Version
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

### GUI Version
1. **Build the GUI Project**
   - Install GTK+ development libraries (e.g., `sudo apt-get install libgtk-3-dev` on Ubuntu)
   - Compile with GTK+ flags:
     ```
     gcc main_gui.c bmp8.c bmp24.c equalize8.c equalize24.c -o image_processor_gui `pkg-config --cflags --libs gtk+-3.0` -lm
     ```

2. **Run the GUI Program**
   - Launch the GUI executable: `./image_processor_gui`
   - Use the graphical interface to:
     - Open BMP images via File menu
     - Apply filters via Filters menu
     - Save processed images

---

## File Structure

- `main.c`: Terminal-based main menu and user interface
- `main_gui.c`: GUI-based main application using GTK+
- `bmp8.c` / `bmp8.h`: 8-bit BMP image handling and filters
- `bmp24.c` / `bmp24.h`: 24-bit BMP image handling and filters
- `equalize8.c` / `equalize8.h`: Histogram equalization for grayscale images
- `equalize24.c` / `equalize24.h`: Histogram equalization for color images
- `test_image_processing.c`: Automated filter tests (for development)
- `CMakeLists.txt`: Build configuration

---


