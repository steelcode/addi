Advanced dalvik dynamic instrumentation

### Struttura File ###
1. dalvikhook/jni ---> framework
	1. +dalvikhook/jni/dexstuff.c ---> dlsym su funzioni dalvik, dex support
	2. +dalvikhook/jni/dalvik_hook.c ---> controllo inline hook

2. examples/dynsec/jni	---> crea il .so da caricare in memoria
	1. +examples/JavaModule/ ---> Progetto Eclipse dei moduli java
	2. +examples/JavaModule/FakeContext/bin/classes.dex ---> DEX da caricare a runtime

3. AndroidLib/TesiDDI ---> API framework, android lib

4. scripts/ ---> helper scripts. I .sh citati sotto sono in questa dir


### Cosa serve ###
- adbi https://github.com/crmulliner/adbi
- android ndk
- root sul device 
- busybox in PATH: mknod e simili..

### Installazione ###

1. cd scripts; ./pushfiles.sh

### Esecuzione ###

2. adb shell
3. cd /data/local/tmp;
4. chmod 775 initenv.sh; ./initenv.sh  % solo una volta per ogni riavvio
5. ./runhijack.sh $args  % noargs per help

### Log ###

Ogni run crea un file di log (strmon.log). 
Vedere help di runhijack.sh per le modalità di log (di default se trova il file fa il rotate, cosi poi me li mandate :) 

### Compilazione e Tips ###

Per compilare utilizzare il toolchain di android-ndk.
Se tra un run e un altro si vuole cambiare il file .dex, ricordarsi di eliminare il vecchio in /data/dalvik-cache.
impostare checkjni e checkmalloc con setprop.




