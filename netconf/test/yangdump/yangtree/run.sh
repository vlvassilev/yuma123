#!/bin/bash

#./update-module-repository.sh

rm -rf tmp || true
mkdir tmp

TOTAL=0
OKS=0
FAILS=0

for dst in ietf ; do
  mkdir -p tmp/pyang/${dst}
  mkdir -p tmp/yangdump/${dst}
for module in `ls ../../../modules/${dst}` ; do
  echo $module
  is_submodule="`head -n 1 ../../../modules/${dst}/${module} | grep submodule`" || true
  if [ "${is_submodule}" != "" ] ; then
    echo "Skip submodule: ${module}"
    continue
  fi
  TOTAL=$((${TOTAL}+1))

  pyang_cmd="pyang -f tree --path /usr/share/yuma/modules/:/usr/share/yuma/modules/yang:/usr/share/yuma/modules/ietf:/usr/share/yuma/modules/netconfcentral:/usr/share/yuma/modules/yuma123 ../../../modules/${dst}/${module}"
  yangdump_cmd="yangdump --format=tree --modpath=/usr/share/yuma/modules/:/usr/share/yuma/modules/yang:/usr/share/yuma/modules/ietf:/usr/share/yuma/modules/netconfcentral:/usr/share/yuma/modules/yuma123 ../../../modules/${dst}/${module}"
  echo $pyang_cmd
  $pyang_cmd | tee > tmp/pyang/${dst}/${module}.tree
  pyang_ret=$?
  echo $yangdump_cmd
  $yangdump_cmd | tee > tmp/yangdump/${dst}/${module}.tree
  yangdump_ret=$?
  echo $yangdump_ret
  echo $pyang_ret
  cmp tmp/pyang/${dst}/${module}.tree tmp/yangdump/${dst}/${module}.tree
  cmp_ret=$?
  if [ "${cmp_ret}" = "0" ] ; then
    echo OK
    OKS=$((${OKS}+1))
  else
    FAILS=$((${FAILS}+1))
    echo FAIL: $module
  fi
done
done
  echo OKS=$OKS
  echo FAILS=$((${TOTAL}-${OKS}))
exit $FAILS
