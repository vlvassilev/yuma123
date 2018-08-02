#!/bin/bash

#./update-module-repository.sh

if [ "$RUN_WITH_CONFD" != "" ] ; then
    # skipped test return value
    exit 77
fi

rm -rf tmp || true
mkdir tmp

OKS=0
FAILS=0

modpath=/usr/share/yuma/modules/

for filespec in `find /usr/share/yuma/modules/ -name '*.yang' | sort` ; do
  module=`basename $filespec`
  echo $module >&2

  is_submodule="`head -n 1 ${filespec} | grep submodule`" || true
  if [ "${is_submodule}" != "" ] ; then
    echo "Skip submodule: ${module}"
    continue
  fi
  cmd="/usr/sbin/netconfd --validate-config-only --startup-error=stop --module=${filespec} --no-startup --modpath=${modpath}"
  echo $cmd  >&2
  $cmd  >&2
  ret=$?
  if [ "${ret}" = "0" ] ; then
    OKS=$((${OKS}+1))
    echo OK: $module
  else
    FAILS=$((${FAILS}+1))
    echo FAIL: $module
  fi
done
done
  echo OKS=$OKS
  echo FAILS=$FAILS
exit $FAILS
