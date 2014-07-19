#!/system/bin/sh

LOGN=strmon.log

su
rm  $LOGN
touch $LOGN
chmod 777 $LOGN