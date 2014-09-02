/*
 *  Collin's Dynamic Dalvik Instrumentation Toolkit for Android
 *  Collin Mulliner <collin[at]mulliner.org>
 *
 *  (c) 2012,2013
 *
 *	Sid's Advanced DDI
 *	(c) 2014
 *
 *  License: LGPL v2.1
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <string.h>
#include <termios.h>
#include <pthread.h>
#include <stdarg.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>

#include <jni.h>

#include <arpa/inet.h> //inet_addr
#include <sys/socket.h> 

#include <stdlib.h>
#include <errno.h>

/* PIPES*/
#include <inttypes.h>
#include <stdint.h>

#include "hook.h"
#include "dexstuff.h"
#include "dalvik_hook.h"
#include "base.h"
#include "lista.h"
#include "Misc.h"
#include "coffeecatch.h"
#include "coffeecatch.h"
 
#undef log

#define log(...) \
        {FILE *fp = fopen(logname, "a+");\
        fprintf(fp, __VA_ARGS__);\
        fclose(fp);}

struct hook_t eph;
struct hook_t eph2;
struct hook_t addTraceh;
struct dalvik_hook_t sb1;
int debug = 1;

char logname[] = "/data/local/tmp/dynsec/nhook.log";
//char logname[] = "/data/local/tmp/strmon.log";

static const JNIEnv *env = NULL;
static JavaVM* g_JavaVM = NULL;

pthread_mutex_t hookm;

static void my_log(char *msg)
{
	log("%s",msg)
}
static void my_log2(char *msg)
{
	if (debug)
		log("%s",msg)
}
// helper function
void printString(JNIEnv *env, jobject str, char *l)
{
	char *s = (*env)->GetStringUTFChars(env, str, 0);

	if (s) {
		log("%s%s\n", l, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}
}



// JNI OnLoad
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    g_JavaVM = vm;
    setJavaVM(g_JavaVM);
    log("XXX5 Java CHIAMATO ONLOAD, jvm = 0x%x\n", g_JavaVM)
    return JNI_VERSION_1_6;
}

static void my_connect(int socket, struct sockaddr *address, socklen_t length){
	log("chiamato connect!!!\n")
	int (*orig_connect)(int socket, struct sockaddr *address, socklen_t length);
	orig_connect = (void*)eph2.orig;
	if(address->sa_family == AF_INET){
		struct sockaddr_in *address_in = address;
		log("sinport = 0x%x\n", address_in->sin_port)
	}
	orig_connect(socket, address,  length);

}
//static void my_add_trace(void* self, void* method, int action, u4 cpuClockDiff, u4 wallClockDiff){
static void my_add_trace(void* self, void* method, int action){
	pthread_mutex_lock(&hookm);
	void (*origin_add_trace)(void* self, void* method, int action);
	origin_add_trace = (void*) addTraceh.orig;	
	log("XXX7 chiamato add trace\n")
	hook_precall(&addTraceh);
	origin_add_trace(self,method,action);
	/**
	if(tryMagic(method)){
		hook_precall(&addTraceh);
		origin_add_trace(self,method,action);
		hook_postcall(&addTraceh);	
	}
	*/
	log("XXX7 fine add trace\n")
	pthread_mutex_unlock(&hookm);
	return;
}
static void* my_dvmInitClass(jclass* clazz){
	log("XXX7 dentro hook dvminitclass = %p\n", clazz)
	/*
	void* (*origin_dvmInitClass)(jclass*);
	log("original: %p \n", eph2.orig)
	origin_dvmInitClass = (void*)eph2.orig;
	hook_precall(&eph2);
	void* res = origin_dvmInitClass(clazz);
	//hook_postcall(&eph2);
	log("XXX7 esco da hook dvminitclass %p, %x\n",res,res)
	return res;
	*/
}
static int my_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	log("dentro my_epool_wait, log vale = %s\n", logname)
	int (*orig_epoll_wait)(int epfd, struct epoll_event *events, int maxevents, int timeout);
	orig_epoll_wait = (void*)eph.orig;
	// remove hook for epoll_wait
	hook_precall(&eph);
	my_ddi_init();
	lancia_consumer_producer();
	_createPTY();
	//_newThread();
	// call original function
	int res = orig_epoll_wait(epfd, events, maxevents, timeout);    
	//hook_postcall(&eph);
	return res;
}


// set my_init as the entry point
void __attribute__ ((constructor)) my_init(void);

void my_init(void)
{
	pthread_mutex_init(&hookm, NULL);
	
 	// set to 1 to turn on, this will be noisy
	debug = 1;

 	// set log function for  libbase (very important!)
	set_logfunction(my_log2);
	// set log function for libdalvikhook (very important!)
	dalvikhook_set_logfunction(logmsgtofile);
	dalvikhook_set_stacklogfunction(logstacktofile);
	//dalvikhook_set_logfunction(my_log2);
	log("DDI: started, pid: %d logname: %s\n", getpid(),logname);
	//hook(&addTraceh, getpid(), "libdvm.", "_Z17dvmMethodTraceAddP6ThreadPK6Methodi", my_add_trace, my_add_trace);
    hook(&eph, getpid(), "libc.", "epoll_wait", my_epoll_wait, 0);
   // hook(&eph2, getpid(), "libdvm.", "_Z14dvmCallMethodAP6ThreadPK6MethodP6ObjectbP6JValuePK6jvalue", 0, my_dvmInitClass);
    
   // hook(&eph2, getpid(), "libwhatsapp.", "connect", my_connect	, 0);

}