#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail

ROOT="${HOME}/behemoth_node"
PREFIX="${PREFIX:-/data/data/com.termux/files/usr}"

printf '[+] NWS SOVEREIGN TELEMETRY BOOTSTRAP — S25 ULTRA\n'
printf '[+] Installing native build toolchain...\n'
pkg update -y
pkg install clang binutils make -y

mkdir -p "${ROOT}/bin"

SRC="${ROOT}/behemoth_telemetry.c"
BIN="${ROOT}/bin/behemoth_telemetry"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
if [ -f "${REPO_ROOT}/telemetry/behemoth_telemetry.c" ]; then
  cp "${REPO_ROOT}/telemetry/behemoth_telemetry.c" "${SRC}"
else
  printf 'ERROR: telemetry/behemoth_telemetry.c not found in repository checkout.\n' >&2
  exit 1
fi

printf '[+] Compiling ARM64 native telemetry engine...\n'
clang -O2 -Wall -Wextra -Wpedantic -D_FORTIFY_SOURCE=2 \
  -o "${BIN}" "${SRC}"
chmod 755 "${BIN}"

cat > "${ROOT}/bin/telemetry-once" <<'EOF'
#!/data/data/com.termux/files/usr/bin/bash
exec "$HOME/behemoth_node/bin/behemoth_telemetry" "$@"
EOF

cat > "${ROOT}/bin/telemetry-watch" <<'EOF'
#!/data/data/com.termux/files/usr/bin/bash
exec "$HOME/behemoth_node/bin/behemoth_telemetry" --watch "${1:-2}"
EOF

chmod 755 "${ROOT}/bin/telemetry-once" "${ROOT}/bin/telemetry-watch"

printf '\n[+] Installed: %s\n' "${BIN}"
printf '[+] One-shot: %s\n' "${ROOT}/bin/telemetry-once"
printf '[+] Watch:    %s [seconds]\n' "${ROOT}/bin/telemetry-watch"
printf '[+] Done.\n'
