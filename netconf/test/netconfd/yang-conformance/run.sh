#!/bin/bash

rm -rf tmp
mkdir tmp
cd tmp
../check-yang-conformance.sh .. ../testspec.txt
