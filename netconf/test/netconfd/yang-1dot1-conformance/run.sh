#!/bin/bash

rm -rf tmp
mkdir tmp
cd tmp
bash -x ../../yang-conformance/check-yang-conformance.sh .. ../testspec.txt
