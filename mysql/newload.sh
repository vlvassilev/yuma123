#
# load the ncorg data, based on the output of the yangdump SQL
#

echo ""
echo "Loading the yumabench database from P1"
echo ""

/usr/bin/mysql -uandysql -pkaligirl yumabench < $1



