#!/bin/sh

# Check that the schema tree contains the expanded grouping and has
# the action defined therein.  yangdump is sufficient for this.

set -e

tmpfile=`mktemp`

yangdump \
	--format=tree \
	--modpath=`pwd` \
	--module=test-action-in-grouping > ${tmpfile}

fgrep -q -- '---x make-something-happen' ${tmpfile} || {
	rm -f ${tmpfile}
	exit 1
}
grep -q -- '   +--.. some-information' ${tmpfile} || {
	rm -f ${tmpfile}
	exit 1
}
rm -f ${tmpfile}
exit 0
