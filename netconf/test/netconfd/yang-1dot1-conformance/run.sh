#!/bin/bash

rm -rf tmp
mkdir tmp
cd tmp
../../yang-conformance/check-yang-conformance.sh .. ../testspec.txt
