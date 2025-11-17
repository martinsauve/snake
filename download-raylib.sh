#!/usr/bin/env bash

RAYLIB_RELEASE_URL="https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_linux_amd64.tar.gz"

echo "Downloading raylib..."
curl -L $RAYLIB_RELEASE_URL -o raylib.tar.gz
tar -xzf raylib.tar.gz
rm raylib.tar.gz
mv raylib-5.5_linux_amd64 raylib
