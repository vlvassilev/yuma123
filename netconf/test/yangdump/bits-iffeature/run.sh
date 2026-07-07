#!/bin/sh -e
# YANG 1.1 if-feature inside a bit
cd "$(dirname "$0")"
out="$(yangdump modpath=. module=test-bits-iffeature 2>&1 || true)"
echo "$out"
case "$out" in
  *error\(246\)*) echo "FAIL: bits if-feature still rejected" >&2; exit 1 ;;
esac
echo "OK: bits if-feature parses"
