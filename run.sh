#!/bin/bash
DIR="$(dirname "$0")"
export LD_LIBRARY_PATH="$DIR/lib:$LD_LIBRARY_PATH"
"$DIR/Mycraft-Linux"
