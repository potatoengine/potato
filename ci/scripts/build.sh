#/bin/bash
BUILD_TYPE="${1:-Debug}"
DIR=`dirname "$0"`

set -o xtrace

bash "${DIR}/generate.sh" "${BUILD_TYPE}"

cmake --build . --parallel
