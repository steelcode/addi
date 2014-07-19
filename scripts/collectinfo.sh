#!/system/xbin/bash

OUT="/data/local/tmp/strmon.log"
echo "FILE DI LOG: $OUT"
#getprop
echo "----------------------" >> $OUT
echo -n "ro.hardware: " >> $OUT
getprop ro.hardware >> $OUT
echo -n "ro.product.model: " >> $OUT
getprop ro.product.model >> $OUT
echo -n "ro.build.fingerprint: " >> $OUT
getprop ro.build.fingerprint >> $OUT
echo -n "ro.build.host: " >> $OUT
getprop ro.build.host >> $OUT
echo -n "ro.build.product: " >> $OUT
getprop ro.build.product >> $OUT
echo -n "ro.build.version.codename: " >> $OUT
getprop ro.build.version.codename >> $OUT
echo -n "ro.build.version.release: " >> $OUT
getprop ro.build.version.release >> $OUT
echo -n "ro.build.version.sdk: " >> $OUT
getprop ro.build.version.sdk >> $OUT
echo "----------------------" >> $OUT
