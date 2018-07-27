#!/bin/bash
if [ $# -eq 1 ] ; then
  cd $1
fi
for file in `find -name '*.c'` ; do
    echo -n `basename $file .c` | sed -e "s/^test_//"
    echo -n " "
    echo -n `grep TEST_SCHEMA_LOAD_FAIL $file | cut -d ' ' -f 3`
    data_file_count="`grep TEST_DATA_FILE_COUNT $file | cut -d ' ' -f 3`"
    if [ "$data_file_count" != "0" ] ; then
        echo -n " "
        echo -n `grep TEST_DATA_FILE_LOAD_FAIL $file | cut -d ' ' -f 3`
    fi
    echo
done
