#!/bin/sh
#
# Copyright (C) 2011 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

lib=-XXlib:libartd.so
invoke_with=

while true; do
  if [ "$1" = "--invoke-with" ]; then
    shift
    invoke_with="$1"
    shift
  elif [ "$1" = "-d" ]; then
    lib="-XXlib:libartd.so"
    shift
  elif expr "$1" : "--" >/dev/null 2>&1; then
    echo "unknown option: $1" 1>&2
    exit 1
  else
    break
  fi
done

unset ANDROID_PRODUCT_OUT # avoid defaulting dex2oat --host-prefix to target output
mkdir -p /tmp/android-data/dalvik-cache
cd $ANDROID_BUILD_TOP
ANDROID_DATA=/tmp/android-data \
  ANDROID_ROOT=$ANDROID_HOST_OUT \
  LD_LIBRARY_PATH=$ANDROID_HOST_OUT/lib \
  exec $invoke_with $ANDROID_HOST_OUT/bin/dalvikvm $lib \
    -Xbootclasspath:$ANDROID_HOST_OUT/framework/core-libart-hostdex.jar \
    -Ximage:$ANDROID_HOST_OUT/framework/core.art \
    -cp AndroidSPECjvm2008.jar -Dspecjvm.home.dir=/home/merck/References/SPECjvm2008 spec.harness.Launch -coe -ict -ikv
