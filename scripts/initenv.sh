#!/system/xbin/bash

#initenv.sh
#eseguire sul device

BASEDIR=/data/local/tmp
RESFIFO=res.fifo
REQFIFO=req.fifo
DALVIKC=/data/dalvik-cache

cd $BASEDIR


#crea pipe
#if [ ! -e $RESFIFO ];
#then
#mknod -m 777 res.fifo p
#fi 
#if [ ! -e $REQFIFO ];
#then
#mknod -m 777 req.fifo p
#fi

#dalvik-cache
chmod 777 $DALVIKC

#permessi brutali
chmod -R 777 *

echo "terminato"




