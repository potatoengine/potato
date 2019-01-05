#/bin/bash
DIR=`dirname "$0"`

PYTHON=`which python3 2>/dev/null`
if [ -z "${PYTHON}" ] ; then PYTHON=`which python` ; fi

CLANG_FORMAT=`which clang-format-${CLANG_VERSION} 2>/dev/null`
if [ -z "${CLANG_FORMAT}" ] ; then CLANG_FORMAT=`which clang-format` ; fi

set -o xtrace

bash "${DIR}/generate.sh" Debug

#find /source/source -name '*.cpp' | xargs -t clang-tidy-${CLANG_VERSION} -p /build -header-filter='/source/source/*'

"${PYTHON}" /source/ci/scripts/run-clang-format.py /source/source -r --clang-format-executable="${CLANG_FORMAT}" -e '*/debug.windows.h' -e 'external/'
