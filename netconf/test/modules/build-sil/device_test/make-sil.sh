export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../../../target/lib/ 
export PATH=$PATH:../../../../target/bin/
export YUMA_HOME=../../../../../
export YUMA_MODPATH=$YUMA_MODPATH:../../yang/:$YUMA_HOME/netconf/modules

BASENAME=$(basename $1 .yang)

yangdump format=h indent=4 module=$1 output=${BASENAME}.h
yangdump format=cpp_test indent=4 module=$1 output=${BASENAME}.cpp

