#!/bin/bash

echo "🔧 P-touch ESP32 Setup Script"
echo "============================"
echo

# Check if config.h already exists
if [ -f "include/config.h" ]; then
    echo "✅ Configuration file already exists at include/config.h"
    echo "   You can edit it manually or delete it to run this setup again."
else
    echo "📋 Creating configuration file..."
    cp include/config.example.h include/config.h
    echo "✅ Configuration file created at include/config.h"
fi

echo
echo "🔧 Next steps:"
echo "1. Edit include/config.h with your WiFi credentials"
echo "2. Run: pio run --target upload"
echo "3. Run: pio run --target uploadfs"
echo "4. Monitor: pio device monitor"
echo
echo "📝 To edit configuration:"
echo "   nano include/config.h"
echo "   code include/config.h"
echo "   vim include/config.h"
echo
echo "🔒 Security: Your config.h file is automatically excluded from Git"
echo "   This keeps your WiFi credentials safe when publishing to GitHub."
echo
echo "🌐 After uploading, connect to your ESP32's IP address in a web browser"
echo "   The IP address will be shown in the serial monitor."
echo 