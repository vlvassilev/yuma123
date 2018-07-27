#!/bin/bash
LIB_YANG_PATH=../../../../../libyang

rm -rf tmp
mkdir tmp
cd tmp
../genspec.sh ../${LIB_YANG_PATH}/tests/conformance | sort > ../testspec.txt
../check-yang-conformance.sh ../${LIB_YANG_PATH}/tests/conformance ../testspec.txt
