#!/usr/bin/env bash
# Lõi phải thuần: core, protocol, services không được dính phần cứng hay adapter.
set -euo pipefail
cd "$(dirname "$0")/.."
bad=0
check() { # thư mục, mẫu cấm
  if rg -n --no-heading "$2" "$1" 2>/dev/null; then echo "VI PHẠM: $1 không được dùng: $2"; bad=1; fi
}
check src/core 'Arduino\.h|WiFi|PubSubClient|ArduinoJson|<(adapters_esp32|adapters_fake|services|protocol)/'
check src/protocol 'Arduino\.h|WiFi|PubSubClient|<(adapters_esp32|adapters_fake|services)/'
check src/services 'Arduino\.h|WiFi|PubSubClient|ArduinoJson|<(adapters_esp32|adapters_fake)/'
check src/adapters_fake 'Arduino\.h|WiFi|PubSubClient|ArduinoJson|<adapters_esp32/'
[ $bad -eq 0 ] && echo "Lõi thuần, không vi phạm."
exit $bad
