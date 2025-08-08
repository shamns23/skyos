#!/bin/bash
set -e


# Run make clean
if ! make clean; then
  echo "[ERROR] make clean failed!" >&2
  exit 1
fi

# Run make
if ! make; then
  echo "[ERROR] make failed!" >&2
  exit 1
fi

# Show build info
make info

# Run make run
if ! make run; then
  echo "[ERROR] make run failed!" >&2
  exit 1
fi
