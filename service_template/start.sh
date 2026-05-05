#!/bin/sh
set -e

echo "==> Configuring project..."
cmake -B build -S .

echo "==> Building project..."
cmake --build build --parallel

echo "==> Running unit tests..."
./build/service_template_unittest

echo "==> Starting service..."
./build/service_template --config /service_template/configs/static_config.yaml
