#!/usr/bin/env bash
# Lõi phải thuần: core, protocol, services không được dính phần cứng hay adapter.
set -euo pipefail
cd "$(dirname "$0")/.."
bad=0
check() { # thư mục, mẫu cấm
  if rg -n --no-heading "$2" "$1" 2>/dev/null; then echo "VI PHẠM: $1 không được dùng: $2"; bad=1; fi
}
check src/shared/core 'Arduino\.h|WiFi|PubSubClient|ArduinoJson|<shared/(adapters_esp32|adapters_fake|services|protocol)/'
check src/shared/protocol 'Arduino\.h|WiFi|PubSubClient|<shared/(adapters_esp32|adapters_fake|services)/'
check src/shared/services 'Arduino\.h|WiFi|PubSubClient|ArduinoJson|<shared/(adapters_esp32|adapters_fake)/'
check src/shared/adapters_fake 'Arduino\.h|WiFi|PubSubClient|ArduinoJson|<shared/adapters_esp32/'
[ $bad -eq 0 ] && echo "Lõi thuần, không vi phạm."
exit $bad
