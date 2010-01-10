#
# Regular cron jobs for the yuma package
#
0 4	* * *	root	[ -x /usr/bin/yuma_maintenance ] && /usr/bin/yuma_maintenance
