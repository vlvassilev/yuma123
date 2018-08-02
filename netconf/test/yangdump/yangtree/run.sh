#!/bin/bash

#./update-module-repository.sh

rm -rf tmp || true
mkdir -p tmp/pyang
mkdir -p tmp/yangdump

TOTAL=0
OKS=0
FAILS=0

modpath=/usr/share/yuma/modules

for filespec in `find /usr/share/yuma/modules/ -name '*.yang' | sort` ; do
  module=`basename $filespec .yang`
  echo $module >&2
  echo $module

  is_submodule="`head -n 1 ${filespec} | grep submodule`" || true
  if [ "${is_submodule}" != "" ] ; then
    echo "Skip submodule: ${module}"
    continue
  fi

  TOTAL=$((${TOTAL}+1))
  pyang_cmd="pyang -f tree --path ${modpath} ${filespec}"
  yangdump_cmd="yangdump --format=tree --modpath=/usr/share/yuma/modules/ ${filespec}"
  echo $pyang_cmd
  $pyang_cmd | grep -v '^$' | tee > tmp/pyang/${module}.tree
  pyang_ret=$?
  echo $yangdump_cmd
  $yangdump_cmd | grep -v '^$' | tee > tmp/yangdump/${module}.tree
  yangdump_ret=$?
  echo yangdump_ret=${yangdump_ret} >&2
  echo pyang_ret=${pyang_ret} >&2
  cmp tmp/pyang/${module}.tree tmp/yangdump/${module}.tree
  cmp_ret=$?
  if [ "${cmp_ret}" = "0" ] ; then
    echo OK: $module
    OKS=$((${OKS}+1))
  else
    FAILS=$((${FAILS}+1))
    echo FAIL: $module
  fi
done

  echo OKS=$OKS
  echo FAILS=$((${TOTAL}-${OKS}))
exit $FAILS
