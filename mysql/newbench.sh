#
# reload the Netconf Central database
# all data is removed and the latest SQL definition is loaded
#

echo ""
echo "Deleting and re-creating the yumabench database..."
echo ""

/usr/bin/mysqladmin -uroot -pchewieboy drop yumabench
/usr/bin/mysqladmin -uroot -pchewieboy create yumabench

echo ""
echo "You need to run 'paster setup-app development.ini' from your project directory."
echo "Use this URL in your development.ini file:"
echo "    sqlalchemy.url=mysql://andysql:kaligirl@localhost/yumabench"
echo ""

