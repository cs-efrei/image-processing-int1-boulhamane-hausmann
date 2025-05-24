# Image Processing GUI

This is a GTK3-based graphical user interface for the image processing project.

## Features

### File Operations
- **Open Image**: Load BMP images (8-bit and 24-bit supported)
- **Save Image**: Save processed images to disk

### Image Filters
- **Negative**: Invert image colors
- **Grayscale**: Convert 24-bit images to grayscale
- **Blur**: Apply box blur filter (24-bit images only)
- **Sharpen**: Apply sharpening filter (24-bit images only)
- **Histogram Equalization**: Enhance image contrast
- **Brightness Adjustment**: Increase (+30) or decrease (-30) brightness

## How to Build

### Prerequisites
- GTK3 development libraries
- GCC compiler
- pkg-config

### Building
```bash
# Using the provided Makefile
make -f Makefile.gui

# Or manually
gcc main_gui.c project_c_amel_tom_int1/bmp8.c project_c_amel_tom_int1/bmp24.c project_c_amel_tom_int1/equalize8.c project_c_amel_tom_int1/equalize24.c `pkg-config --cflags --libs gtk+-3.0` -o image_gui -lm
```

## How to Run

```bash
./image_gui
```

## Usage Instructions

1. **Load an Image**: 
   - Click "File" → "Open Image"
   - Select a BMP file (8-bit or 24-bit)
   - The image will be displayed and image information will be shown

2. **Apply Filters**:
   - Click "Filters" and select desired filter
   - The filter will be applied and the display will update
   - Some filters are only available for specific bit depths

3. **Save Processed Image**:
   - Click "File" → "Save Image"
   - Choose a filename and location
   - The processed image will be saved

## Filter Compatibility

| Filter | 8-bit | 24-bit |
|--------|-------|--------|
| Negative | ✓ | ✓ |
| Grayscale | - | ✓ |
| Blur | - | ✓ |
| Sharpen | - | ✓ |
| Histogram Equalization | ✓ | ✓ |
| Brightness | ✓ | ✓ |

## Notes

- The GUI automatically detects the bit depth of loaded images
- Images are scaled to fit the display window (max 400px)
- Temporary files are created during processing and automatically cleaned up
- The status bar shows current operation and image information
