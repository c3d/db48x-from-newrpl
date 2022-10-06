#!/bin/bash

BUILD_NUM=$(git rev-list --count HEAD)
BUILD_BRANCH=$(git rev-parse --abbrev-ref HEAD)
BUILD_COMMIT=$(git rev-parse --short HEAD)

echo \""\\\"newRPL build $BUILD_NUM ($BUILD_BRANCH-$BUILD_COMMIT) - © 2014-2022 The newRPL Team\\\""\"
