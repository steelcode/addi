#!/usr/bin/bash
#	Copia i file necessari sul device
#	- libdynsec.so
#	- initenv.sh
#	- runhijack.sh
#	- classes.dex
#	- hijack
#	- libmstring.so
#	- getinfo.sh

BASEDIR=`pwd`
REMOTEDIR=/data/local/tmp
BLACKFILE="pushfiles.sh"
DEX=$BASEDIR/../examples/JavaModule/FakeContext/bin/classes.dex
LIBSO=$BASEDIR/../examples/dynsec/lib/armeabi/libdynsec.so

for file in `echo *`
do
	if [ $file != $BLACKFILE ]; then
		adb push $file $REMOTEDIR
	fi
done

#adb fails
#adb push $DEX $REMOTEDIR
#adb push $LIBSO $REMOTEDIR
echo ""
echo "Ricorda di pushare sul device il file classes.dex e la lib dinamica"
echo ""