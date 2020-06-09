#!/bin/sh

# Check that the schema tree contains the expanded grouping and has
# the notification defined therein.  yangdump is sufficient for this.


set -e

tmpfile=`mktemp`

yangdump \
	--format=tree \
	--modpath=`pwd` \
	--module=test-notification-in-grouping > ${tmpfile}

fgrep -q -- '---n something-happened' ${tmpfile} || {
	rm -f ${tmpfile}
	exit 1
}
grep -q -- '   +--.. some-information' ${tmpfile} || {
	rm -f ${tmpfile}
	exit 1
}
rm -f ${tmpfile}
exit 0
