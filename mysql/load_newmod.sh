#
# load the ncorg data, based on the output of the yangdump SQL
#

echo ""
echo "Loading the netconfcentral.org database from P1"
echo ""

/usr/bin/mysql -uandysql -pkaligirl netconfcentral < $1



