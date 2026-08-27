#!/usr/bin/env bash

set -e

PRESET="mac_intel_release"

echo "Configuring..."
cmake --preset "$PRESET"

echo "Building..."
cmake --build "out/build/$PRESET"

echo "Installing..."
cmake --install "out/build/$PRESET" --prefix "out/install/$PRESET"