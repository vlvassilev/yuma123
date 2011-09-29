export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../../target/lib/ 
export PATH=$PATH:../../../../target/bin/
export YUMA_HOME=../../../../../
export YUMA_MODPATH=../../../../../netconf/modules

BASENAME=$(basename $1 .yang)

yangdump format=h indent=4 module=$1 output=${BASENAME}.h
yangdump format=c indent=4 module=$1 output=${BASENAME}.cpp

