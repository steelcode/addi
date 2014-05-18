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
 
#undef log

#define log(...) \
        {FILE *fp = fopen("/data/local/tmp/strmon.log", "a+");\
        fprintf(fp, __VA_ARGS__);\
        fclose(fp);}

static struct hook_t eph;
static struct hook_t eph2;

static struct dalvik_hook_t sb1;
static int debug = 0;

static const JNIEnv *env = NULL;
static JavaVM* g_JavaVM = NULL;

/**
char *ptsn;
int coms;
lista L;
pthread_mutex_t mutex; //list mutex
static struct dexstuff_t d;
static int debug = 1;
int cookie;
pthread_t pty_t;
*/
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

/**
static int aflag = 0;
static void* hookp(JNIEnv* env,jobject thiz, jobject a1, jobject a2){
	log("XX4 dentro hookp \n")
	jvalue args[2];
	args[0].l = a1;
	args[1].l = a2;
	dalvik_prepare(&d,&sb1, env);
	void * res = (*env)->CallObjectMethodA(env, thiz, sb1.mid, args);
	dalvik_postcall(&d,&sb1);
	if(aflag == 10){
		int ret = _tryLockThreadList(&d);
		log("XXX4 PRIMA DI SUSPEND THREAD ritornato + %d\n", ret)
		void * niente = getSelf(&d);
		niente = getSelf(&d);
		niente = getSelf(&d);
		callSuspendThread(&d);
		//_suspendSelf(&d); <---- sospende native
		_unlockThreadList(&d);
		log("XXX4 DOPO SUSPEND THREAD!!!\n")
		_createPTY(env);
	}
	aflag++;
	return res;
}
static void unaprova(){

	dalvik_hook_setup(&sb1, "Landroid/app/SharedPreferencesImpl;",  "getString",  "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", 3, hookp);
	dalvik_hook(&d, &sb1);

}
*/


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
static int my_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	int (*orig_epoll_wait)(int epfd, struct epoll_event *events, int maxevents, int timeout);
	orig_epoll_wait = (void*)eph.orig;
	// remove hook for epoll_wait
	hook_precall(&eph);
	log("XXX5 MYPID %d\n", getpid())
	my_ddi_init();
	_createPTY();

	// call original function
	int res = orig_epoll_wait(epfd, events, maxevents, timeout);    
	return res;
}

// set my_init as the entry point
void __attribute__ ((constructor)) my_init(void);

void my_init(void)
{
	log("DDI: started\n");
	//pthread_mutex_init(&mutex, NULL);
/**
	coms=1;
	ptsn = (char*) malloc(sizeof(char*));
	if(!start_coms(&coms, ptsn))
		log("XXX error start coms\n")
  	if(debug)
  		log("XXX ptsname: %s - %d\n", ptsn, coms)
*/
 	// set to 1 to turn on, this will be noisy
	debug = 1;

 	// set log function for  libbase (very important!)
	set_logfunction(my_log);
	// set log function for libdalvikhook (very important!)
	dalvikhook_set_logfunction(my_log);
	/**
	char* oldClassPath = getenv("CLASSPATH");
	log("XXX5 classpath = %s\n", oldClassPath)
	if(oldClassPath == NULL){
		setenv("CLASSPATH", "/data/local/tmp/tesiddi.jar" ,1);
	}
	*/

    hook(&eph, getpid(), "libc.", "epoll_wait", my_epoll_wait, 0);
   // hook(&eph2, getpid(), "libwhatsapp.", "connect", my_connect	, 0);

}