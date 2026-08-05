#!/usr/bin/env bash
set -Eeuo pipefail

make CONFIG_FILE=.config-win64 -j$(nproc)
