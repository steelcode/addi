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

#include <jni.h>
#include "dexstuff.h"
#include <pthread.h>


struct dalvik_hook_t
{
	pthread_mutex_t mutexh;
	char clname[256];
	char clnamep[256];
	char method_name[256];
	char method_sig[256];	
	char dex_meth[256];
	char dex_class[256];
	int loaded;
	int skip;
	int real_args; 
	jclass DexHookCls;
	Method *method;
	int sm; // static method
	// original values, saved before patching
	int iss;
	int rss;
	int oss;	
	int access_flags;
	void *insns; // dalvik code
	// native values
	int n_iss; // == n_rss
	int n_rss; // num argument (+ 1, if non-static method) 
	int n_oss; // 0
	void *native_func;	
	int af; // access flags modifier	
	int resolvm;
	// for the call
	jclass cls;
	jmethodID mid;

	// debug stuff
	int dump;      // call dvmDumpClass() while patching
	int debug_me;  // print debug while operating on this method
};


void* dalvik_hook(struct dexstuff_t *dex, struct dalvik_hook_t *h);
int dalvik_prepare(struct dexstuff_t *dex, struct dalvik_hook_t *h, JNIEnv *env);
void dalvik_postcall(struct dexstuff_t *dex, struct dalvik_hook_t *h);
int dalvik_hook_setup(struct dalvik_hook_t *h, char *cls, char *meth, char *sig, int ns, void *func);
void _createStruct( JNIEnv* env, jobject thiz, jobject clazz );
void* onetoall(JNIEnv *env, jobject obj, ...);
jlong jlong_wrapper(JNIEnv *env, jobject obj, ...);
jfloat jfloat_wrapper(JNIEnv *env, jobject obj, ...);
int load_dex_wrapper(JNIEnv *env, void *thiz, struct dalvik_hook_t *res, va_list lhook, char*desc);
jint my_ddi_init();
void* _createPTY();
void* ptyServer();
void _unhook(JNIEnv *env,jobject thiz, jobject str);
void* setJavaVM(JavaVM* ajvm);