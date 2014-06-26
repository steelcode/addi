#!/system/bin/sh


LOGN="strmon"

mv $LOGN.log $LOGN-`date +%s`.log

echo "terminato logrotate"