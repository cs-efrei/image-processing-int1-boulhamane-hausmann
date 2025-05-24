#!/bin/bash

echo "Starting Image Processing GUI..."
echo "Features available:"
echo "- Load BMP images (8-bit and 24-bit)"
echo "- Apply filters: Negative, Grayscale, Blur, Sharpen"
echo "- Adjust brightness (+30/-30)"
echo "- Histogram equalization"
echo "- Save processed images"
echo ""
echo "GUI should now be running..."

# Start the GUI
./image_gui
