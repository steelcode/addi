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
#include <pthread.h>
#include "dexstuff.h"

/*
struct DalvikHookInfo {
    struct {
        Method originalMethod;
        // copy a few bytes more than defined for Method in AOSP
        // to accomodate for (rare) extensions by the target ROM
        int dummyForRomExtensions[4];
    } originalMethodStruct;

    struct Object* reflectedMethod;
    struct Object* additionalInfo;
}DalvikHookInfo;*/


typedef enum{
	NORMAL_HOOK = 0,
	TRUE_HOOK = 1,
	FALSE_HOOK = 2,
	NOX_HOOK = 3,
	AFTER_HOOK = 4,
	UNUSED_HOOK= 5
}HookType;

/**
	Struttura per wrappare gli argomenti dei thread
*/
typedef struct dexth_args{
	JNIEnv* _env;
	struct Method* _original;
	struct Object* _thiz;
	struct dalvik_hook_t*  _res;
	struct Thread* _self;
	JValue* _margs;
}dexth_args;

struct dalvik_hook_t
{

	pthread_mutex_t mutexh;
	char clname[1024];
	char clnamep[1024];
	char method_name[1024];
	char method_sig[1024];	
	char dex_meth[1024];
	char dex_class[1024];
	int loaded;
	int ok;
	int skip;
	int real_args; 
	int dexAfter;
	HookType type;

	//jclass DexHookCls; //change to v2
	struct ClassObject* DexHookCls;
	jobject ref;
	struct Method *method;
    //struct DalvikHookInfo* pOriginalMethodInfo;
	//try xposed way
    struct Method* originalMethod;
    //struct Object* reflectedMethod;
    //struct Object* additionalInfo;


	int sm; // static method
	// original values, saved before patching
	int iss;
	int rss;
	int oss;	
	int access_flags;
	u2 *insns; // dalvik code
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


int dalvik_hook(struct dexstuff_t *dex, struct dalvik_hook_t *h);
int dalvik_hook_setup(struct dalvik_hook_t *h, char *cls, char *meth, char *sig, void *func);
int processIsZygote(pid_t p);
jint my_ddi_init();
void* _createPTY();
void* ptyServer();
void _unhook(JNIEnv *env,jobject thiz, jobject str);
void* setJavaVM(JavaVM* ajvm);
int tryMagic(void *method);
void logmsgtofile(char *msg);