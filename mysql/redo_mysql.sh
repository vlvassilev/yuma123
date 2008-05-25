#
# reload the Netconf Central database
# all data is removed and the latest SQL definition is loaded
#


echo ""
echo "Deleting and re-creating the netconfcentral.org database..."
echo ""

/usr/bin/mysqladmin -uroot -pchewieboy drop netconfcentral
/usr/bin/mysqladmin -uroot -pchewieboy create netconfcentral

echo ""
echo "Reloading the netconfcentral.org database from netconfcentral.sql..."
echo ""

/usr/bin/mysql -uandysql -pkaligirl netconfcentral < ./netconfcentral.sql

