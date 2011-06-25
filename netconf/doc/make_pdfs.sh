#!/bin/sh
# make all the user manual PDF files

if [ -n "`which unoconv`" ]; then
  cd yuma_docs
  unoconv yuma-installation-guide.odt
  mv yuma-installation-guide.pdf pdf
  unoconv yuma-quickstart-guide.odt
  mv yuma-quickstart-guide.pdf pdf
  unoconv yuma-user-cmn-manual.odt
  mv yuma-user-cmn-manual.pdf pdf
  unoconv yuma-netconfd-manual.odt
  mv yuma-netconfd-manual.pdf pdf
  unoconv yuma-yangcli-manual.odt
  mv yuma-yangcli-manual.pdf pdf
  unoconv yuma-yangdiff-manual.odt
  mv yuma-yangdiff-manual.pdf pdf
  unoconv yuma-yangdump-manual.odt
  mv yuma-yangdump-manual.pdf pdf
  unoconv yuma-dev-manual.odt
  mv yuma-dev-manual.pdf pdf
else
  echo "unoconv program not found!"
fi
