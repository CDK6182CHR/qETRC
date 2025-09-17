#!/bin/bash
set -e

echo "=== 自动构建 qETRC（macOS + Qt6 自动检测版本） ==="

# -------------------------
# 0. 检测系统架构
# -------------------------
ARCH=$(uname -m)
echo "系统架构: $ARCH"

# -------------------------
# 1. 自动检测 Homebrew Qt6
# -------------------------
QT_DIR=""
for VER in 6.9.2 6.9.1 6.8.0 6.7.0 6.6.0 6.5.1; do
    if [ -d "/opt/homebrew/opt/qt@$VER" ]; then
        QT_DIR="/opt/homebrew/opt/qt@$VER"
        break
    elif [ -d "/usr/local/opt/qt@$VER" ]; then
        QT_DIR="/usr/local/opt/qt@$VER"
        break
    fi
done

# 如果没有找到 Homebrew Qt，尝试 PATH 中 qmake
if [ -z "$QT_DIR" ]; then
    QT_PATH=$(which qmake || true)
    if [ -n "$QT_PATH" ]; then
        QT_DIR=$(dirname $(dirname "$QT_PATH"))
    fi
fi

if [ -z "$QT_DIR" ]; then
    echo "❌ 未找到 Qt6，请通过 Homebrew 或官方安装 Qt6"
    exit 1
fi
echo "✅ 检测 Qt6 路径: $QT_DIR"

# -------------------------
# 2. 构建 SARibbon
# -------------------------
echo "=== 构建 SARibbon ==="
cd external/SARibbon
mkdir -p build && cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=../bin_mac \
    -DSARIBBON_USE_FRAMELESS_LIB=OFF \
    -DCMAKE_PREFIX_PATH="$QT_DIR"

# 修复 Qt5 target → Qt6 target
if [ -f ../bin_mac/lib/cmake/SARibbonBar/SARibbonBarTargets.cmake ]; then
    sed -i '' 's/Qt5::Core/Qt6::Core/g' ../bin_mac/lib/cmake/SARibbonBar/SARibbonBarTargets.cmake
    sed -i '' 's/Qt5::Widgets/Qt6::Widgets/g' ../bin_mac/lib/cmake/SARibbonBar/SARibbonBarTargets.cmake
fi

cmake --build . --target install -j$(sysctl -n hw.ncpu)
SARIBBON_DIR=$(pwd)/../bin_mac/lib/cmake/SARibbonBar
echo "✅ SARibbon 构建完成: $SARIBBON_DIR"

cd ../../../  # 回到 qETRC 根目录

# -------------------------
# 3. 构建 qETRC
# -------------------------
echo "=== 构建 qETRC ==="
mkdir -p build && cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$QT_DIR" \
    -DSARibbonBar_DIR="$SARIBBON_DIR"

cmake --build . -j$(sysctl -n hw.ncpu)

# -------------------------
# 4. 打包 .app
# -------------------------
APP_NAME="qETRC"
APP_DIR="$PWD/${APP_NAME}.app"
BIN_PATH="$PWD/qETRC"

echo "=== 打包 qETRC.app ==="
mkdir -p "$APP_DIR/Contents/MacOS"
mkdir -p "$APP_DIR/Contents/Resources"
cp "$BIN_PATH" "$APP_DIR/Contents/MacOS/$APP_NAME"

if [ -f ../dmg_resources/qETRC.icns ]; then
    cp ../dmg_resources/qETRC.icns "$APP_DIR/Contents/Resources/qETRC.icns"
fi

cat > "$APP_DIR/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleExecutable</key>
  <string>$APP_NAME</string>
  <key>CFBundleIdentifier</key>
  <string>com.example.$APP_NAME</string>
  <key>CFBundleName</key>
  <string>$APP_NAME</string>
  <key>CFBundleVersion</key>
  <string>1.0</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>CFBundleIconFile</key>
  <string>qETRC.icns</string>
</dict>
</plist>
EOF

# -------------------------
# 5. macdeployqt 打包 Qt 库
# -------------------------
echo "=== 使用 macdeployqt 打包 Qt 依赖 ==="
"$QT_DIR/bin/macdeployqt" "$APP_DIR" -verbose=2

# -------------------------
# 6. 生成 dmg
# -------------------------
echo "=== 生成 dmg 安装包 ==="
TEMP_DIR=$(mktemp -d)
VOLUME_NAME="$APP_NAME"

mkdir -p "$TEMP_DIR/$APP_NAME"
cp -R "$APP_DIR" "$TEMP_DIR/$APP_NAME/"
ln -s /Applications "$TEMP_DIR/$APP_NAME/Applications"

if [ -f ../dmg_resources/background.png ]; then
    mkdir -p "$TEMP_DIR/$APP_NAME/.background"
    cp ../dmg_resources/background.png "$TEMP_DIR/$APP_NAME/.background/"
fi

RW_DMG="$PWD/${APP_NAME}_rw.dmg"
hdiutil create -volname "$VOLUME_NAME" -srcfolder "$TEMP_DIR/$APP_NAME" \
  -ov -format UDRW "$RW_DMG"

MOUNT_DIR=$(hdiutil attach "$RW_DMG" -readwrite -noverify -mountpoint /Volumes/"$VOLUME_NAME" | grep "/Volumes/" | awk '{print $3}')

osascript <<EOF
tell application "Finder"
  tell disk "$VOLUME_NAME"
    open
    set current view of container window to icon view
    set toolbar visible of container window to false
    set statusbar visible of container window to false
    set the bounds of container window to {100, 100, 600, 400}

    set icon view options of container window to icon view options
    set arrangement of icon view options of container window to not arranged
    if exists file ".background:background.png" then
        set background picture of icon view options of container window to file ".background:background.png"
    end if

    delay 1
    set position of disk item "$APP_NAME.app" of container window to {100, 100}
    set position of disk item "Applications" of container window to {400, 100}

    update without registering applications
    delay 2
    close
  end tell
end tell
EOF

hdiutil detach "$MOUNT_DIR"
hdiutil convert "$RW_DMG" -format UDZO -imagekey zlib-level=9 -o "$PWD/$APP_NAME.dmg"

rm "$RW_DMG"
rm -rf "$TEMP_DIR"

echo "✅ 构建完成，生成 dmg: $PWD/$APP_NAME.dmg"
echo "🎉 支持自动检测 Qt6 版本和 M/Intel Mac 构建完成！"
