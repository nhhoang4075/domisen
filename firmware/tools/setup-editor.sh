#!/usr/bin/env bash
# Cho Zed (clangd) hiểu đường dẫn thư viện ESP32, hết báo "Arduino.h file not found".
# Chạy một lần sau khi clone, và chạy lại khi thêm thư viện hoặc đổi platformio.ini.
# Hai file sinh ra (compile_commands.json, .clangd) chứa đường dẫn riêng của máy, không commit.
# Không cần khi dùng VS Code với PlatformIO IDE, tiện ích đó tự lo.
set -euo pipefail
cd "$(dirname "$0")/.."

pio run -e comfort -t compiledb

TC="$HOME/.platformio/packages/toolchain-xtensa-esp32"
CXX="$(ls -d "$TC"/xtensa-esp32-elf/include/c++/* | head -1)"

cat > .clangd <<CFG
CompileFlags:
  Compiler: clang++
  Add: [--target=armv7em-none-eabi, -D__XTENSA__=1, -isystem$CXX, -isystem$CXX/xtensa-esp32-elf, -isystem$TC/xtensa-esp32-elf/include]
  Remove: [-mlongcalls, -mtext-section-literals, -fstrict-volatile-bitfields, -fno-tree-switch-conversion, -freorder-blocks, -mfix-esp32-psram-cache-issue, -fno-shrink-wrap, -mno-target-align, -mforce-l32]
CFG
echo "Xong. Trong Zed, mở lại file hoặc chạy 'editor: restart language server'."
