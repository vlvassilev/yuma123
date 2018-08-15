#!/bin/bash

#./update-module-repository.sh

rm -rf tmp || true
mkdir tmp

OKS=0
FAILS=0

modpath=/usr/share/yuma/modules

for filespec in `find /usr/share/yuma/modules/ -name '*.yang' | sort` ; do
  module=`basename $filespec .yang`
  echo $module >&2

  is_submodule="`head -n 1 ${filespec} | grep submodule`" || true
  if [ "${is_submodule}" != "" ] ; then
    echo "Skip submodule: ${module}"
    continue
  fi

  if [ "$RUN_WITH_PYANG" != "" ] ; then
      pyang --path ${modpath}/ ${filespec}  1>&2
      RES=$?
  elif [ "$RUN_WITH_YANGDUMP" != "" ] ; then
      yangdump --modpath=${modpath} ${filespec}  1>&2
      RES=$?
  elif [ "$RUN_WITH_YANGLINT" != "" ] ; then
      yanglint --path=${modpath} ${filespec} 1>&2
      RES=$?
  elif [ "$RUN_WITH_CONFD" != "" ] ; then
      killall -KILL confd || true
      source $RUN_WITH_CONFD/confdrc
      for subdir in `find ${modpath} -type d`
      do
          confd_modpath=${confd_modpath}${subdir}":"
      done
      confd_modpath=${confd_modpath}"."

      cmd="confdc -c ${filespec} --yangpath ${confd_modpath} -o tmp/${module}.fxs"
      echo $cmd  >&2
      $cmd  >&2
      RES=$?
  else
      cmd="/usr/sbin/netconfd --validate-config-only --startup-error=stop --module=${filespec} --no-startup --modpath=${modpath}"
      echo $cmd  >&2
      $cmd  >&2
      RES=$?
  fi

  if [ "${RES}" = "0" ] ; then
    OKS=$((${OKS}+1))
    echo OK: $module
  else
    FAILS=$((${FAILS}+1))
    echo FAIL: $module
  fi
done
  echo OKS=$OKS
  echo FAILS=$FAILS
exit $FAILS
