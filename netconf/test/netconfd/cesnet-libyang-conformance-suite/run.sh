#!/bin/bash
LIB_YANG_PATH=../../../../../libyang/tests/conformance/

rm -rf tmp
mkdir tmp
cd tmp
../check-yang-conformance.sh ../${LIB_YANG_PATH} ../testspec.txt
