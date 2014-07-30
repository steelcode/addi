### Advanced dalvik dynamic instrumentation ###


Il framework ADDI è composto da due 'side', uno nativo e uno Java. 
La parte nativa si occupa di gestire e impostare dinamicamente i Dalvik-hook a run-time, gestire l'interfaccia dei comandi, caricare a run-time i file DEX, etc...
La parte Java contiene la definizione degli hook, il codice da eseguire quando scatta un hook e le classi che permettono la creazione dei log e dei report di esecuzione, inoltre sono offerte delle API (come AndroidLib) per comunicare con la parte nativa.


### Struttura File ###

1. dalvikhook/jni ---> framework (crea lib statica)
	1. +dalvikhook/jni/dexstuff.c ---> dlsym su funzioni dalvik, dex support
	2. +dalvikhook/jni/dalvik_hook.c ---> controllo inline hook

2. examples/dynsec/jni	---> crea il .so da caricare in memoria
	1. +examples/JavaModule/ ---> Progetto Eclipse dei moduli java
	2. +examples/JavaModule/FakeContext/bin/classes.dex ---> DEX da caricare a runtime

3. AndroidLib/TesiDDI ---> API framework, android lib

4. scripts/ ---> helper scripts. I .sh citati sotto sono in questa dir


### Cosa serve ###

- root sul device 
- adbi https://github.com/crmulliner/adbi
- android ndk
- busybox in PATH: mknod e simili..

### Installazione ###

0. Compilare Adbi e copiare il binario "hijack" nella cartella "scripts"
1. cd scripts; ./pushfiles.sh

### Esecuzione ###

2. adb shell
3. cd /data/local/tmp;
4. chmod 775 initenv.sh; % solo una volta per ogni riavvio
5. ./initenv.sh  % solo una volta per ogni riavvio
6. ./runhijack.sh  % seguire l' help

### Log ###

Ogni run crea un file di log (strmon.log). 
di default ad ogni run se trova il file fa il rotate.

### Compilazione e Tips ###

Per compilare utilizzare il toolchain di android-ndk.
Se tra un run e un altro si vuole cambiare il file .dex, ricordarsi di eliminare il vecchio in /data/dalvik-cache.
impostare checkjni e checkmalloc con setprop per debug.
Per ulteriori info guardare nella cartella "docs/"




