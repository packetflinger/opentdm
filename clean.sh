#!/usr/bin/env bash
set -Eeuo pipefail

rm -rf debug
rm -rf release
make clean
make CONFIG_FILE=.config-win32 clean
make CONFIG_FILE=.config-win64 clean
