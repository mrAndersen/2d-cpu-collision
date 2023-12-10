#!/bin/bash

path=$(readlink -f "${BASH_SOURCE:-$0}")
dir=$(dirname "$path")
dir=$(readlink -f "${dir}/../")

rm -rf "${dir}"/vendor || true
mkdir "${dir}"/vendor

git clone -b 5.0 --depth 1 https://github.com/raysan5/raylib.git "${dir}"/vendor/raylib