#!/bin/bash

if [ "$RUN_WITH_CONFD" != "" ] ; then
    # skipped test return value
    exit 77
fi

rm -rf tmp || true
mkdir tmp

LIB_YANG_PATH=../../../../../libyang
MODULE=sec7_19_1/mod1.yang
FAIL=0
OK=0

EXPECTED="OK"
/usr/sbin/netconfd --validate-config-only --startup-error=stop --no-startup --module= ${LIB_YANG_PATH}/tests/conformance/${MODULE} 1>tmp/netconfd.stdout 2>tmp/netconfd.stderr
RES=$?
cat tmp/netconfd.stdout
if [ "$RES" != "0" ] ; then
  if [ "$EXPECTED" == "OK" ] ; then
    echo "FAIL: ${MODULE}"
    FAIL=$(($FAIL+1))
  else
    echo "OK:  ${MODULE}"
    OK=$(($OK+1))
  fi
else
  if [ "$EXPECTED" == "OK" ] ; then
    echo "FAIL: ${MODULE}"
    FAIL=$(($FAIL+1))
  else
    echo "OK: ${MODULE}"
    OK=$(($OK+1))
  fi
fi
echo "TOTAL: OKs=$OK FAILs=$FAIL"
exit $FAIL
