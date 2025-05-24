#!/bin/bash

# Image Processing GUI Demo Script
echo "========================================="
echo "    Image Processing GUI Demo"
echo "========================================="
echo ""

# Check if GUI executable exists
if [ ! -f "./image_gui" ]; then
    echo "❌ GUI executable not found. Building..."
    make -f Makefile.gui
    if [ $? -ne 0 ]; then
        echo "❌ Failed to build GUI. Please check dependencies."
        exit 1
    fi
fi

# Check for test images
echo "📁 Available test images:"
for img in *.bmp; do
    if [ -f "$img" ]; then
        echo "   - $img"
    fi
done

echo ""
echo "🚀 Starting Image Processing GUI..."
echo ""
echo "GUI Features:"
echo "  📷 Load BMP images (8-bit and 24-bit)"
echo "  🎨 Apply various filters:"
echo "     • Negative"
echo "     • Grayscale (24-bit only)"
echo "     • Box Blur (24-bit only)"
echo "     • Sharpen (24-bit only)"
echo "     • Histogram Equalization"
echo "     • Brightness adjustment"
echo "  💾 Save processed images"
echo ""
echo "Usage Instructions:"
echo "  1. File → Open Image (select a .bmp file)"
echo "  2. Filters → Choose desired filter"
echo "  3. File → Save Image (save result)"
echo ""
echo "Press Ctrl+C to stop the GUI"
echo ""

# Launch the GUI
./image_gui
