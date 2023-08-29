#!/bin/sh

make -j15 || exit 1
[[ "$1" == "debug" ]] && (
    gf2 -ex run ./whisper &
) || (
    ./whisper &
)
sleep 1

echo "// comment" >> './src/areas/instances/static.c'
