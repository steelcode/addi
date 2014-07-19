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


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/mman.h>

#include "dexstuff.h"
#include "dalvik_hook.h"
#include "lista.h"
#include "Misc.h"
#include "log.h"
#include "coffeecatch.h"


#define APICLASS "Lorg/sid"
#define HOOKCLASS "Lorg/tesi"
#define DALVIKHOOKCLS "org/sid/addi/core/DalvikHook" //used by env->findclass()
#define DALVIKHOOKCLS_2 "Lorg/sid/addi/core/DalvikHook;"

struct dexstuff_t d;
lista L; //dalvikhook list
pthread_mutex_t mutex; //list mutex
int debug = 1;
static int cookie = NULL;
pthread_t pty_t;
static JavaVM* g_JavaVM = NULL;
//static jclass jbclazz = NULL;
//static jclass jiclazz = NULL;
//static jclass dalvikCls = NULL; //(*env)->FindClass(env, DALVIKHOOKCLS);

//int dalvik_hook_setup(struct dalvik_hook_t *h, char *cls, char *meth, char *sig, int ns, void *func)
int dalvik_hook_setup(struct dalvik_hook_t *h, char *cls, char *meth, char *sig,  void *func)
{
	if (!h)
		return 0;

	pthread_mutex_init(&h->mutexh, NULL);
	strcpy(h->clname, cls);
	
	strncpy(h->clnamep, cls+1, strlen(cls)-2);
	h->clnamep[strlen(cls)-2] = '\0';

	strcpy(h->method_name, meth);
	strcpy(h->method_sig, sig);
	h->n_iss = 0;
	h->n_rss = 0;
	h->n_oss = 0;
	h->native_func = func;

	h->sm = 0; // set by hand if needed

	h->af = 0x0100; // native, modify by hand if needed
	
	h->resolvm = 0; // don't resolve method on-the-fly, change by hand if needed

	h->debug_me = 0;
	h->dump = 0;
	return 1;
}


int dalvik_hook(struct dexstuff_t *dex, struct dalvik_hook_t *h,JNIEnv* env)
{
	jthrowable exc;
	log("DENTRO DALVIK HOOK MAIN, cerco classe = %s\n",h->clname)
	if (h->debug_me)
		log("dalvik_hook: class %s\n", h->clname)
	void *target_cls = dex->dvmFindLoadedClass_fnPtr(h->clname); //return classobject*
	
	log("class = 0x%x\n", target_cls)

	if (!target_cls) {
		if (h->debug_me)		
		target_cls = dex->dvmFindClass_fnPtr(h->clname,NULL);
		exc = (*env)->ExceptionOccurred(env);
  		if (exc) {
  			(*env)->ExceptionClear(env);
  		}

		if(!target_cls)
			return 0;
	}

	// print class in logcat
	if (h->dump && dex && target_cls)
		dex->dvmDumpClass_fnPtr(target_cls, (void*)1);

	if(h->debug_me){
		log("stampo debug in logcat!!\n");
		dex->dvmDumpClass_fnPtr(target_cls, (void*)1);		
	}
		

	h->method = dex->dvmFindVirtualMethodHierByDescriptor_fnPtr(target_cls, h->method_name, h->method_sig);
	if (h->method == 0) {
		//if(h->debug_me)
		log("calling dvmFindDirectMethodByDescriptor_fnPtr\n");
		h->method = dex->dvmFindDirectMethodByDescriptor_fnPtr(target_cls, h->method_name, h->method_sig);
	}
	// constructor workaround, see "dalvik_prepare" below
	if (!h->resolvm) {
		h->cls = target_cls;
		h->mid = (void*)h->method;
		log("XXXD: h->method vale: 0x%08x, 0x%08x\n", h->method, h->mid)
	}
	
	if (h->debug_me)
		log("%s(%s) = 0x%x\n", h->method_name, h->method_sig, h->method)

	if (h->method) {
		log("DALVIK HOOK COPIO METHOD %p %s\n", h->method, h->method_name)
		
		struct Method* cippa = (struct Method*) dex->dvmLinearAlloc_fnPtr(((struct ClassObject*)target_cls)->classLoader,sizeof(struct Method));//(struct Method*)calloc(1,sizeof(struct Method));
		memcpy( cippa, h->method, sizeof(struct Method));
		h->originalMethod = cippa;
		//h->method->shouldTrace = true;
		//h->originalMethod->shouldTrace = true;
		h->sm = is_static(dex, h->method);
		h->insns = h->method->insns;

		if (h->debug_me) {
			log("DALVIK_HOOK h->method->nativeFunc 0x%x, h->nativefunc: 0x%x\n", h->method->nativeFunc, h->native_func)		
			log("DALVIK_HOOK insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->method->insSize, h->method->registersSize, h->method->outsSize)
		}
		h->iss = h->method->insSize;
		h->rss = h->method->registersSize;
		h->oss = h->method->outsSize;
	
		int res = dex->dvmComputeMethodArgsSize_fnPtr(h->method);
		//if(!h->sm){
		//	res++;
		//}
		/* collin way new 
		h->method->insSize = res; 
		h->method->registersSize = res;
		h->method->outsSize = 0;
		h->n_iss  = res;
		h->n_rss = res;
		*/
		/* old collin way
		h->method->insSize = h->n_iss;
		h->method->registersSize = h->n_rss;
		h->method->outsSize = h->n_oss;
		*/
		if (h->debug_me) {
			log("shorty %s\n", h->method->shorty)
			log("name %s\n", h->method->name)
			log("arginfo %x\n", h->method->jniArgInfo)
			log("#args = %d\n", res)
		}
		//h->method->jniArgInfo = 0x80000000; // <--- also important why this is not used in Xposed?
		//h->access_flags = h->method->a;

		h->method->a = h->method->a | h->af; // make method native
		if (h->debug_me)
			log("access %x\n", h->method->a)
		if (h->debug_me) {
			log("DALVIK_HOOK nativeFunc %p, dalvin_hook_t: %p, method->insns = %p\n", h->method->nativeFunc, h->native_func, h->method->insns)		
			log("DALVIK_HOOK insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->method->insSize, h->method->registersSize, h->method->outsSize)
			log("DALVIK HOOK ORIGINAL nativeFunc %p,  method->insns = %p\n", h->originalMethod->nativeFunc, h->originalMethod->insns)		
			log("DALVIK HOOK ORIGINAL insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->originalMethod->insSize, h->originalMethod->registersSize, h->originalMethod->outsSize)
		}

		//dex->dvmUseJNIBridge_fnPtr(h->method, h->native_func);		

		/* Xposed Way */
		h->method->nativeFunc = h->native_func;
		h->method->insns = (const u2*)  cippa;
		h->method->registersSize = cippa->insSize;
		h->method->outsSize = 0;
		h->n_iss  = res;

		log("METODO CON %d ARGOMENTI \n", res)
		log("DALVIK_HOOK nativeFunc %p, dalvin_hook_t: %p, method->insns = %p\n", h->method->nativeFunc, h->native_func, h->method->insns)		
		log("DALVIK_HOOK insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->method->insSize, h->method->registersSize, h->method->outsSize)
		log("DALVIK HOOK ORIGINAL nativeFunc %p,  method->insns = %p\n", h->originalMethod->nativeFunc, h->originalMethod->insns)		
		log("DALVIK HOOK ORIGINAL insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->originalMethod->insSize, h->originalMethod->registersSize, h->originalMethod->outsSize)


		if (h->debug_me)
			log("patched %s to: 0x%x\n", h->method_name, h->native_func)
		if (h->debug_me) {
			log("DALVIK_HOOK nativeFunc 0x%x, dalvin_hook_t: 0x%x  method->insns = %p\n", h->method->nativeFunc, h->native_func, h->method->insns)		
			log("DALVIK_HOOK insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->method->insSize, h->method->registersSize, h->method->outsSize)
		}
		log("DALVIKHOOK PROVA PROVA \n")
		log("DALVIKHOOK  %p, nome = %s \n", cippa, cippa->name);
		//log("test %p nome = %s \n", h->pOriginalMethodInfo->originalMethodStruct.originalMethod,h->pOriginalMethodInfo->originalMethodStruct.originalMethod.name)
		//log(" nativefunc = %p, insns = %p \n", h->pOriginalMethodInfo->originalMethodStruct.originalMethod.nativeFunc, h->pOriginalMethodInfo->originalMethodStruct.originalMethod.insns)

		/*
		log("INIZIO CONTROLLO !! \n")
		int i = 0;
		struct ClassObject* clz = dex->dvmFindLoadedClass_fnPtr(h->clname);
		for(i=0; i < clz->vtableCount; i++){
			log("NOME VTABLE [%d] = %s \n", i, clz->vtable[i]->name)
			log("INSNS  = %p, nativeFunc = %p  \n", clz->vtable[i]->insns, clz->vtable[i]->nativeFunc)
		}
		*/
		/*
		log("HO TROVATO %d METODI DIRETTI!!\n", clz->directMethodCount)
		for(i=0; i<clz->directMethodCount;i++){
			struct Method* mz = &clz->directMethods[i];
			log("trovato direct metodo: %s, insns = %p, nativef = %p \n", mz->name, mz->insns, mz->nativeFunc)
		}
		*/
		return 1;
	}
	else {
		if (h->debug_me)
			log("could NOT patch %s\n", h->method_name)
	}
	return 0;
}

int dalvik_prepare(struct dexstuff_t *dex, struct dalvik_hook_t *h, JNIEnv *env)
{

	// this seems to crash when hooking "constructors"
	if (h->resolvm) {
		log("-----DENTRO RESOLVM BY HANDS!!!!!!------\n")
		log("clnamep = %s\n", h->clnamep)
		h->cls = (*env)->FindClass(env, h->clnamep);
		log("DOPO FIND CLASS\n")
		if (h->debug_me)
			log("cls = 0x%x\n", h->cls)
		if (!h->cls)
			return 0;
		if (h->sm)
			h->mid = (*env)->GetStaticMethodID(env, h->cls, h->method_name, h->method_sig);
		else
			h->mid = (*env)->GetMethodID(env, h->cls, h->method_name, h->method_sig);
		if (h->debug_me)
			log("mid = 0x%x\n", h-> mid)
		if (!h->mid)
			return 0;
	}
	log("XXX5 -----DENTRO DALVIK PREPARE!!!!!!------\n")
	if (h->debug_me) {
		log("DALVIK PREPARE nativeFunc %p, dalvin_hook_t: %p, method->insns = %p\n", h->method->nativeFunc, h->native_func, h->method->insns)		
		log("DALVIK PREPARE insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->method->insSize, h->method->registersSize, h->method->outsSize)
	}
	h->method->insSize = h->iss;
	h->method->registersSize = h->rss;
	h->method->outsSize = h->oss;
	h->method->a = h->access_flags;
	h->method->jniArgInfo = 0;
	h->method->insns = h->insns; 
	log(" FINITO PREPARE\n")
	if (h->debug_me) {
		log("DALVIK PREPARE nativeFunc %p, dalvin_hook_t: %p, method->insns = %p\n", h->method->nativeFunc, h->native_func, h->method->insns)		
		log("DALVIK PREPARE insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->method->insSize, h->method->registersSize, h->method->outsSize)
	}
}

void dalvik_postcall(struct dexstuff_t *dex, struct dalvik_hook_t *h)
{
	h->method->insSize = h->n_iss;
	h->method->registersSize = h->n_rss;
	h->method->outsSize = h->n_oss;

	//log("shorty %s\n", h->method->shorty)
	//log("name %s\n", h->method->name)
	//log("arginfo %x\n", h->method->jniArgInfo)
	h->method->jniArgInfo = 0x80000000;
	//log("noref %c\n", h->method->noRef)
	//log("access %x\n", h->method->a)
	h->access_flags = h->method->a;
	h->method->a = h->method->a | h->af;
	//log("access %x\n", h->method->a)
	//log("XXX6 POSTCALL PRIMA insns = 0x%x\n", h->method->insns)
	if (h->debug_me) {
	log("DALVIK POSTCALL nativeFunc %p, dalvin_hook_t: %p, method->insns = %p\n", h->method->nativeFunc, h->native_func, h->method->insns)		
	log("DALVIK POSTCALL insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->method->insSize, h->method->registersSize, h->method->outsSize)
	}
	dex->dvmUseJNIBridge_fnPtr(h->method, h->native_func);
	if (h->debug_me) {
	log("DALVIK POSTCALL nativeFunc %p, dalvin_hook_t: %p, method->insns = %p\n", h->method->nativeFunc, h->native_func, h->method->insns)		
	log("DALVIK POSTCALL insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->method->insSize, h->method->registersSize, h->method->outsSize)
	}
	//log("XXX6 POSTCALL DOPO insns = 0x%x\n", h->method->insns)
	if (h->debug_me)
		log("patched BACK %s to:: 0x%x\n", h->method_name, h->native_func)
}

//static void __attribute__ ((constructor)) dalvikhook_my_init(void);

static char logfile[] = "/data/local/tmp/log";

static void logmsgtofile(char *msg)
{
    int fp = open(logfile, O_WRONLY|O_APPEND);
    write(fp, msg, strlen(msg));
    close(fp);
}

static void logmsgtostdout(char *msg)
{
	write(1, msg, strlen(msg));
}

void* dalvikhook_set_logfunction(void *func)
{
    void *old = libdalvikhook_log_function;
    libdalvikhook_log_function = func;
    //logmsgtofile("dalvik_set_logfunction called\n");
    return old;
}

static void dalvikhook_my_init(void)
{
    // set the log_function
	//logmsgtofile("dalvik init\n");
    libdalvikhook_log_function = logmsgtofile;
}



static jint _wrapper_dexstuff_loaddex(JNIEnv *env, char* dexpath){
	int mycookie  = NULL;
	//devo caricare un DEX esterno
	mycookie = dexstuff_loaddex(&d, dexpath);
	if(!mycookie){
		//error loaddex
		log("XXX7 ERROR LOADDEX\n")
		return 0;
	}
	else
		return mycookie;
}
static jobject _wrapper_dexstuff_defineclass(JNIEnv *env,int c, char* clsname){
	jclass *cls = (jclass*)dexstuff_defineclass(&d, clsname, c);
	if(!cls){
		//error defineclass
		return 0;
	}
	else
		return cls;
}
int load_dex_wrapper(JNIEnv *env, void *thiz, struct dalvik_hook_t *res, va_list lhook, char*desc){

	jclass gclazz, mycls, *clazz;
	jmethodID dex_meth;
	jobject gthiz;
	jclass jbclazz = (*env)->FindClass(env, "java/lang/Boolean");
	jclass jiclazz = (*env)->FindClass(env, "java/lang/Integer");

	jmethodID idb = (*env)->GetMethodID(env, jbclazz, "<init>", "(Z)V");
	jmethodID idi = (*env)->GetMethodID(env, jiclazz, "<init>", "(I)V");

	int real_args = res->n_iss;//_dvmComputeMethodArgsSize(&d,res->method);
	if(!res->sm){
		real_args--;
	}

	if(debug)
		log("XXX ho un dex method da chiamare = %s!!!\n", res->dex_meth)
	if(!res->loaded){
		if(debug)
			log("XXX DEVO CARICARE LA CLASSE DAL DEX %s\n", res->dex_class)
		clazz = (jclass *)dexstuff_defineclass(&d, res->dex_class, cookie);
		mycls = (*env)->FindClass(env, DALVIKHOOKCLS);
		gclazz = (*env)->NewGlobalRef(env, mycls);
		res->DexHookCls = gclazz;
		res->loaded=1;
	}
	else{
		log("XXX DEX CLASS CACHED!!!\n")
	}
	if(thiz != NULL){
		log("XXX6 DENTRO SET THIZ\n")
		jmethodID set_thiz = (*env)->GetMethodID(env, res->DexHookCls, "setThiz", "(Ljava/lang/Object;)V");
		gthiz = (*env)->NewGlobalRef(env, thiz);
		if(set_thiz){
			(*env)->CallVoidMethod(env,res->ref, set_thiz, gthiz);
		}
	}
	if(real_args == 0){
		if(debug)
			log("XXX5 chiamo il metodo DEX 0 args con clazz = 0x%x, gclazz = 0x%x\n",  clazz, res->DexHookCls)
		log("XXX5  prima  di getmethod\n")
		jmethodID initid = (*env)->GetMethodID(env, res->DexHookCls, "initFunc", "()V");
		log("XXX5 prima di call initid\n")
		(*env)->CallVoidMethod(env,res->ref, (struct Method*)initid);
		dex_meth = (*env)->GetMethodID(env, res->DexHookCls, "myexecute", "()V");
		(*env)->CallVoidMethod(env,res->ref, dex_meth, NULL);	
		if(debug)
			log("XXX5 method %s chiamato\n", res->method_name)
	}
	else{	
		log("XXX5 il metodo ha argomenti!\n")
		jobjectArray ja;
		jbooleanArray jba;
		jboolean jbargs[real_args];
		int jaboolean = 0;
		
		ja = (*env)->NewObjectArray(env, real_args, (*env)->FindClass(env, "java/lang/Object"), NULL);
		jba = (*env)->NewBooleanArray(env, real_args);

		jobject gja = (*env)->NewGlobalRef(env, ja);
		
		jobject jbobj;
		char *pshorty  = res->method->shorty;
		pshorty++; //[0] is return  type
		int i = 0;

		if(debug)
			log("XXX5 chiamo il metodo DEX con ja = 0x%x, clazz = 0x%x, gclazz = 0x%x con pshorty = %s\n", gja, clazz, res->DexHookCls, pshorty)
		
		//problemi quando costruisco arg di tipo nativo jboolean, ecc..
		for(i=0;i<real_args;i++){
			log("XXX5  DEBUG1 = %c\n", *pshorty)
			switch(*pshorty){
				case 'Z':	
					log("dentro boolean creo oggetto\n")
					jbobj = (*env)->NewObject(env, jbclazz, idb, va_arg(lhook, uint32_t));
					log("creato oggetto!!\n")
					(*env)->SetObjectArrayElement(env, gja, i, jbobj);
					log("inserito oggetto nell'array\n")
					jaboolean = 1;	
					pshorty++;
					break;
				case 'I':
					log("XXX5 load dex argomento[%d] tipo int\n", i)
					jbobj = (*env)->NewObject(env, jiclazz, idi, va_arg(lhook, uint32_t));
					(*env)->SetObjectArrayElement(env,gja,i,jbobj);
					pshorty++;
					break;
				default:
					log("XXX5 load dex argomento[%d] tipo object\n", i)
					(*env)->SetObjectArrayElement(env,gja,i,va_arg(lhook,jobject *));
					pshorty++;
					break;
			}
			
		}
		log("XXX5 impostato gli argomenti!\n")
		//dump_jni_ref_tables(&d);
		if(jaboolean){
			log("XXX5  prima  di getmethod\n")
			jmethodID initid = (*env)->GetMethodID(env, res->DexHookCls, "initFunc", "()V");
			log("XXX5 prima di call initid, ref = 0x%x\n", res->ref)
			(*env)->CallVoidMethod(env,res->ref, initid);
			dex_meth = (*env)->GetMethodID(env, res->DexHookCls, "myexecute", "([Ljava/lang/Object;)V");
			(*env)->CallVoidMethod(env,res->ref, dex_meth, gja);
			//log("XXX5 chiamato metodo\n")
		}
		else{
			log("XXX5  prima  di getmethod\n")
			jmethodID initid = (*env)->GetMethodID(env, res->DexHookCls, "initFunc", "()V");
			log("XXX5 prima di call initid\n")
			(*env)->CallVoidMethod(env,res->ref, initid);
			dex_meth = (*env)->GetMethodID(env, res->DexHookCls, "myexecute", "([Ljava/lang/Object;)V");
			log("XXX5 chiamo metodo in classe = 0x%x con mid = 0x%x \n", res->ref, dex_meth)
			(*env)->CallVoidMethod(env,res->ref, dex_meth, gja);	
			//log("XXX5 chiamato metodo\n")
		}
		(*env)->DeleteGlobalRef(env, gja);
	}
	//(*env)->DeleteGlobalRef(env, gclazz);
	//(*env)->DeleteGlobalRef(env, gthiz);		
	log("XXX5 FINITO USO DEX\n")
	return 1;
}

 jfloat jfloat_wrapper(JNIEnv *env, jobject obj, ...){
	log("--------------------- JFLOAT-------------------------------\n")

	char * cl = (char*)malloc(sizeof(char)*1024);
	char * name = (char*)malloc(sizeof(char)*1024);
	char * descriptor = (char*)malloc(sizeof(char)*1024);
	char * hash = (char*)malloc(sizeof(char)*2048);

	memset(cl,0,sizeof(cl));
	memset(name,0,sizeof(name));
	memset(descriptor,0,sizeof(descriptor));
	memset(hash,0,sizeof(hash));

	struct dalvik_hook_t *res;
	va_list l, lhook;

	va_start(l,obj);
	va_start(lhook,obj);

	get_caller_class(env,obj,cl);
	get_method(&d,name,descriptor);

	strcpy(hash,name);
	strcat(hash,descriptor);
	if(debug)
		log("CERCO NELLA LISTA : %s\n", hash)
	//stampa_lista(L);
	pthread_mutex_lock(&mutex);
	res = (struct dalvik_hook_t *) cerca(L, hash);
	pthread_mutex_unlock(&mutex);
	if(!res){
		log("XXX errore RES = 0\n")
	}

	pthread_mutex_lock(&(res->mutexh));
	int real_args = res->n_iss;//_dvmComputeMethodArgsSize(&d,res->method);
	if(!res->sm){
		real_args--;
	}
	dalvik_prepare(&d, res, env);
	jfloat ret;
	if(real_args == 0){
				//method have zero arguments
		if(res->sm){	
			ret = (*env)->CallStaticFloatMethod(env, obj, res->mid);		
		}
		else{
			ret = (*env)->CallFloatMethod(env, obj, res->mid);
		}

	}
	else{
		if(res->sm){
			if(debug)
				log("call static object method v : %s\n", res->method_name)
			
			ret = (*env)->CallStaticFloatMethodV(env, obj, res->mid, l);
			
			if(debug)
				log("success calling static: %s\n", res->method_name)
		}
		else{
			if(debug)
				log("call object method v : %s mid = 0x%x\n", res->method_name, res->mid)
			
			ret = (*env)->CallFloatMethodV(env, obj, res->mid, l);
			
			if(debug)
				log("success calling : %s\n", res->method_name)
		}
	}
	dalvik_postcall(&d, res);
	free(cl);
	free(name);
	free(descriptor);
	free(hash);
	log("----------------------------------------------------\n")
	pthread_mutex_unlock(&(res->mutexh));
	return ret;
}
jlong jlong_wrapper(JNIEnv *env, jobject obj, ...){
	log("--------------------- JLONG-------------------------------\n")
	char * cl = (char*)malloc(sizeof(char)*1024);
	char * name = (char*)malloc(sizeof(char)*1024);
	char * descriptor = (char*)malloc(sizeof(char)*1024);
	char * hash = (char*)malloc(sizeof(char)*512);

	memset(cl,0,sizeof(cl));
	memset(cl,0,sizeof(name));
	memset(cl,0,sizeof(descriptor));
	memset(cl,0,sizeof(hash));

	struct dalvik_hook_t *res;
	va_list l, lhook;
	int j=0;
	va_start(l,obj);
	va_start(lhook,obj);
	get_caller_class(env,obj,cl);
	get_method(&d,name,descriptor);
	strcpy(hash,name);
	strcat(hash,descriptor);
	if(debug)
		log("CERCO NELLA LISTA : %s\n", hash)
	//stampa_lista(L);
	pthread_mutex_lock(&mutex);
	res = (struct dalvik_hook_t *) cerca(L, hash);
	pthread_mutex_unlock(&mutex);
	if(!res){
		log("XXX errore RES = 0\n")
	}
	pthread_mutex_lock(&(res->mutexh));
	int real_args = res->n_iss;//_dvmComputeMethodArgsSize(&d,res->method);
	if(!res->sm){
		real_args--;
	}
	dalvik_prepare(&d, res, env);
	jlong ret;
	if(real_args == 0){
				//method have zero arguments
		if(res->sm){	
			ret = (*env)->CallStaticLongMethod(env, obj, res->mid);		
		}
		else{
			ret = (*env)->CallLongMethod(env, obj, res->mid);
		}

	}
	else{
		if(res->sm){
			if(debug)
				log("call static object method v : %s\n", res->method_name)
			
			ret = (*env)->CallStaticLongMethodV(env, obj, res->mid, l);
			
			if(debug)
				log("success calling static: %s\n", res->method_name)
		}
		else{
			if(debug)
				log("call object method v : %s mid = 0x%x\n", res->method_name, res->mid)
			
			ret = (*env)->CallLongMethodV(env, obj, res->mid, l);
			
			if(debug)
				log("success calling : %s\n", res->method_name)
		}
	}
	dalvik_postcall(&d, res);
	free(cl);
	free(name);
	free(descriptor);
	free(hash);
	log("----------------------------------------------------\n")
	pthread_mutex_unlock(&(res->mutexh));
	return ret;
}


void* callOrigin(JNIEnv *env, jobject obj, va_list l, struct dalvik_hook_t *res, char* s ){
	jobject str = NULL;
	int j= 0;
	int flag =0;
	char *pointer = s;
	j = is_string_class(pointer);
	int real_args = res->n_iss;//_dvmComputeMethodArgsSize(&d,res->method);
	if(!res->sm){
		real_args--;
	}
	if(j)
		flag = 1;
	//qui posso usare lo shorty
	JValue result;
	switch(*s){
		//method return a class
		case 'L':
			
			if(real_args == 0){
					//method have zero arguments
				if(res->sm){	
					//str = (*env)->CallStaticObjectMethod(env, obj, res->mid);		
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					
					va_list myargs;
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, myargs);
					result.l = (struct Object*) (*env)->NewLocalRef(env, (jobject)result.l);
					log("return a class2: @0x%x\n", result.l)
					return result.l;
				}
				else{
					//str = (*env)->CallObjectMethod(env, obj, res->mid);
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					
					va_list myargs;
					log("chiamo invokemethod con original = %p, obj = %p, result = %p, myargs = %p \n", res->originalMethod, obj, &result, myargs)
					asm volatile("" ::: "memory");
					struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
					log("decodificato = %p\n", nobj)
					log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, myargs);
					result.l = (struct Object*) (*env)->NewLocalRef(env, (jobject)result.l);
					log("return a class2: @0x%x\n", result.l)
					return result.l;
				}
			}
			else{
					//method have args
				if(res->sm){
					if(debug)
						log("call static object method v : %s\n", res->method_name)
					//str = (*env)->CallStaticObjectMethodV(env, obj, res->mid, l);
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, l);
					if(debug)
						log("success calling static: %s\n", res->method_name)
					result.l = (struct Object*) (*env)->NewLocalRef(env, (jobject)result.l);
					log("return a class2: @0x%x\n", result.l)
					return result.l;
				}
				else{
					if(debug)
						log("call object method v : %s mid = 0x%x\n", res->method_name, res->mid)
									
					log("chiamo invokemethod con original = %p, obj = %p, result = %p \n", res->originalMethod, obj, &result )
					asm volatile("" ::: "memory");
					struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
					log("decodificato = %p\n", nobj)
					log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
					//void* gresult = (*env)->NewGlobalRef(env,(jobject)&result);
					jobject* diocane;// = (jobject*) calloc(1, sizeof(jobject));
					struct Object* aa = (struct Object*)(*env)->NewGlobalRef(env,diocane);
					log("global ref = %p\n", aa)
					dump_jni_ref_tables(&d);
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, l);
					return;
					//str = (*env)->CallObjectMethodV(env, obj, res->mid, l);
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					//JValue res;
					//res.l = (JValue) gresult;
					//if(debug)
					//		log("success calling : %s, result name = %s, clazz = %p\n", res->method_name, gresult.l->clazz->descriptor, gresult.l->clazz)
					//result.l = (struct Object*) (*env)->NewGlobalRef(env,result.l);
					//log("return a class2: @0x%x\n", result.l)
					//log("chiamo boxprimitive con result = %p e classobject = %p, primittype = %d\n", result.l, res->originalMethod->clazz, res->originalMethod->clazz->primitiveType)
					//void* aa = d.dvmBoxPrimitive_fnPtr(result,res->originalMethod->clazz);
					
					//memcpy(aa,result.l,sizeof(struct Object*));					
					//log("ritorno res= %p, nobj2 = %p\n", result.l, nobj2)
					//return aa;
				}
			}
			if(flag){
				if(str != NULL){

					char *s = (*env)->GetStringUTFChars(env, result.l, 0);
					if (s != NULL) {
						if(debug)
							log("OTTENUTA STRINGA = %s\n", s)
						(*env)->ReleaseStringUTFChars(env, result.l, s); 
					}
					if(debug)
						log("return FLAG a class: @0x%x\n", str)
					if(debug)
						log("----------------------------------------------------\n")
					return result.l;
				}
				else{
					log("XXX errore str NULL\n")
					if(debug)
						log("----------------------------------------------------\n")
					return result.l;
				}
			}
			else{
				if(debug)
					log("return a class1: @0x%x\n", result.l)
				log("----------------------------------------------------\n")
				result.l = (struct Object*) (*env)->NewLocalRef(env, (jobject)result.l);
				log("return a class2: @0x%x\n", result.l)
				return;
			}
		case 'V':
			if(res->skip){
				if(debug)
					log("VOID :::: SKIPPO IL METODO!!!\n")
				log("----------------------------------------------------\n")
				return (jint)1;
			}
			else{
				if(debug)
					log("VOID obj = 0x%x, mid = 0x%x, l = 0x%x,  realargs = %d, mname = %s\n", obj, res->mid,l, real_args, res->method_name)

				if(strstr(res->method_name, "<init>") != NULL){
					if(debug)
						log("strtstr(%s) STO CHIAMANDO UN COSTRUTTORE\n", res->method_name)					
				}
				if(real_args == 0){
					if(res->sm){
						log("CALLORIGIN STATIC V PROVA PROVA original  = %p, obj = %p\n",res->originalMethod, obj)
						
						
						va_list myargs;
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, myargs);
						//(*env)->CallStaticVoidMethod(env, obj, res->originalMethod);					
						//(*env)->CallStaticVoidMethod(env, obj, res->mid);					

					}
					else{
						if(debug)
							log("chiamo voidmethod\n")
						//(*env)->CallVoidMethod(env, obj, res->mid);
						log("CALLORIGIN V PROVA PROVA %p, name = %s\n",res->originalMethod, res->originalMethod->name)
						log("ORIGINAL insns = %p , nativeFunc = %p, inssize = %d, registersize = %d, jniarginfo = %p, access_flags=%p \n", res->originalMethod->insns, res->originalMethod->nativeFunc, res->originalMethod->insSize,
																	res->originalMethod->registersSize, res->originalMethod->jniArgInfo, res->originalMethod->a)
						log("hooked insns = %p, hooked nativeFunc = %p, inssize=%d, regsize = %d, jniarginfo = %p\n", res->method->insns, res->method->nativeFunc,
																	res->method->insSize, res->method->registersSize, res->method->jniArgInfo)
						log("OLD hooked insns = %p, hooked nativeFunc = %p, inssize=%d, regsize = %d, access_flags = %p\n", res->insns, res->native_func,
											res->iss, res->rss, res->access_flags)
						//_dvmInvokeMethod(&d,thisObject, res->originalMethod, argList, params, returnType, true);
						//(*env)->CallVoidMethod(env, obj, res->originalMethod);
						
						va_list myargs;
						log("chiamo invokemethod con original = %p, obj = %p, result = %p, myargs = %p \n", res->originalMethod, obj, &result, myargs)
						asm volatile("" ::: "memory");
						struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
						log("decodificato = %p\n", nobj)
						log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
						//struct Object* nobj = _dvmDecodeIndirectRef(getSelf(&d), obj); 
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, myargs);
						//_dvmInvokeMethod(&d,obj,res->originalMethod, NULL, NULL, NULL, true);
						/*
						res->method->insSize = res->iss;
						res->method->registersSize = res->rss;
						res->method->outsSize = res->oss;
						res->method->a = res->access_flags;
						res->method->jniArgInfo = 0;
						//res->method->insns = res->insns; 
						*/
						//(*env)->CallVoidMethod(env, obj, (jmethodID) res->originalMethod);
					}
				}
				else{
					if(res->sm){
						log("CALLORIGIN STATIC V PROVA PROVA original =  %p, obj = %p, valist = %p\n",res->originalMethod, obj, l)
						//(*env)->CallStaticVoidMethodV(env, obj, res->originalMethod ,l);
						JValue result;
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, l);
					}
					else{
						if(debug)
							log("chiamo voidmethodV\n")
						//(*env)->CallVoidMethodV(env, obj, res->mid, l);
						log("CALLORIGIN NON STATIC PROVA PROVA original = %p, obj = %p, l =%p\n",res->originalMethod, obj, l)
						

						log("chiamo invokemethod con original = %p, obj = %p, result = %p,  \n", res->originalMethod, obj, &result)
						asm volatile("" ::: "memory");
						struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
						log("decodificato = %p\n", nobj)
						log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
						//struct Object* nobj = _dvmDecodeIndirectRef(getSelf(&d), obj); 
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, l);
						//(*env)->CallVoidMethodV(env, obj, (jmethodID) res->originalMethod, l);
					}
					if(debug)
						log("Chiamato metodo : %s\n", res->method_name)
					log("----------------------------------------------------\n")	
				}
			}
			break;		
		case 'Z':
			log("!!BOOOLEAN___\n") 
			JValue jResult;
			if(res->skip){
				log("!!!!SKIPPO\n")
				if(debug)
					log("----------------------------------------------------\n")
				if(res->skip == 1)
					return (jboolean)1;				
				else return (jboolean)0;
			}
			else{
				if(real_args == 0){	
					if(res->sm){

						//log("CALLORIGIN STATIC Z PROVA PROVA %p, name %s \n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod, res->pOriginalMethodInfo->originalMethodStruct.originalMethod.name)
						//str = (*env)->CallStaticBooleanMethod(env, obj, (jmethodID) &res->pOriginalMethodInfo->originalMethodStruct.originalMethod);
						
						va_list myargs;
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, myargs);
						return result.z;
						if(debug)
							log("----------------------------------------------------\n")					
					}
					else{
						//str = (*env)->CallBooleanMethod(env,obj,res->mid);	
						//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
						
						va_list myargs;
						struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
						log("decodificato = %p\n", nobj)
						log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, myargs);
						return result.z;
					}
					if(debug)
						log("----------------------------------------------------\n")
				}
				else{
					if(res->sm){
						JValue result;
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, l);
						return result.z;
						//log("CALLORIGIN STATIC Z PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)		
						}
					else{
						//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)		
						//str = (*env)->CallBooleanMethodV(env,obj,res->mid,l);
						
						va_list myargs;
						struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
						log("decodificato = %p\n", nobj)
						log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, l);
						return result.z;
					}
					if(debug)
						log("----------------------------------------------------\n")
				}
			}
			return (jboolean)str;
		case 'I':
			if(debug){
				log("XXX2 DENTRO INT METHOD\n")
					//printStackTrace(env);
			}
			if(res->skip){
				if(debug)
					log("XXX2 FINE METODO RITORNO 0!!!!\n")
				if(res->skip == 1)
					return (jint)1;
				else
					return (jint)0;	//to skip checkpermission
			}
			if(real_args == 0){
				log("CALLORIGIN CHIAMO INTMETHOD con obj = %p, res->mid = %p, clname = %s , mthname = %s\n", obj, res->mid, res->clname, res->method_name)
				if(res->sm){
					//str = (*env)->CallStaticIntMethod(env, obj, res->mid);
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)

					
					va_list myargs;
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, myargs);
					return result.i;
				}
				else{
					log("ORIGINCALL CHIAMO N NON STATIC ")
					//str = (*env)->CallIntMethod(env, obj, res->mid);
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					//str = (*env)->CallIntMethod(env, obj, (jmethodID) &res->pOriginalMethodInfo->originalMethodStruct.originalMethod);
					log(" CHIAMATO, ret = %p \n",str)
					
					va_list myargs;
					struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
					log("decodificato = %p\n", nobj)
					log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
					//struct Object* nobj = _dvmDecodeIndirectRef(getSelf(&d), obj); 
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, myargs);
					return result.i;
				}
				dalvik_postcall(&d, res);
				if(debug)
					log("----------------------------------------------------\n")
			}
			else{
				log("CALLORIGIN CHIAMO INT METHODV con obj = %p, res->mid = %p, clname = %s , mthname = %s\n", obj, res->mid, res->clname, res->method_name)
				if(res->sm){					
					//str = (*env)->CallStaticIntMethodV(env, obj, (jmethodID) &res->pOriginalMethodInfo->originalMethodStruct.originalMethod,l);
					//str = (*env)->CallStaticIntMethod(env, obj, res->mid,l);
					
					va_list myargs;
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, l);
					return result.i;
					//str = (*env)->CallStaticIntMethodV(env, obj, res->originalMethod,l);
				}
				else{
					//str = (*env)->CallIntMethodV(env, obj, res->mid,l);
					log("CALLORIGIN PROVA PROVA %p\n",res->originalMethod)
					
					struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
					log("decodificato = %p\n", nobj)
					log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
					//struct Object* nobj = _dvmDecodeIndirectRef(getSelf(&d), obj); 
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, l);
					return result.i;
					//str = (*env)->CallIntMethodV(env, obj, res->originalMethod,l);
				}			
				if(debug)
					log("----------------------------------------------------\n")
			}
			return (jint)str;
		default:
			if(debug)
				log("XXX DEFAULT \n")
			JValue result;
			if(real_args == 0){
				if(debug)
					log("XXX default senza argomenti\n")
				if(res->sm){
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					//str = (*env)->CallStaticIntMethod(env, obj, res->mid);
					va_list myargs;
					
					//struct Object* nobj = _dvmDecodeIndirectRef(getSelf(&d), obj); 
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, myargs);
					
				}
				else{
					//result =(*env)->CallObjectMethod(env, obj, res->mid);
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					va_list myargs;
					
					struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
					log("decodificato = %p\n", nobj)
					log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
					//struct Object* nobj = _dvmDecodeIndirectRef(getSelf(&d), obj); 
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, myargs);
					
				}
				if(debug)
					log("XXX chiamato metodo: %s\n", res->method_name)
			}
			else{
				if(debug)
					log("XXX default con argomenti\n")
				if(res->sm){
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					
					//str = (*env)->CallStaticIntMethod(env, obj, res->mid);
				}
				else{
					//result =(*env)->CallObjectMethodV(env, obj, res->mid, l);
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					
					struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
					log("decodificato = %p\n", nobj)
					log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
					//struct Object* nobj = _dvmDecodeIndirectRef(getSelf(&d), obj); 
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, l);
					
				}
				if(debug)
					log("XXX chiamato metodo: %s\n", res->method_name)
			}
			if(debug)
				log("----------------------------------------------------\n")
			
			return result.l;			
		}
		return -1;
}

void * hookAbilitato(JNIEnv *env, void* obj, va_list l, struct dalvik_hook_t *res, va_list lhook, char* descriptor,char  *s){
	void* ret = NULL;
	int flag = 0;

	int real_args = res->n_iss;//_dvmComputeMethodArgsSize(&d,res->method);
	if(!res->sm){
		real_args--;
	}
	if(debug)
		log("!!!!PRESO MUTEX %s con signature = %s, res = @0x%x, il metodo: @0x%x con mid = 0x%x ha: %d argomenti, skippable = %d, type = %c\n", res->method_name, res->method_sig, res, res->method, res->mid,real_args, res->skip, *s)
	
	if(strlen(res->dex_meth) > 0){
		if(!res->dexAfter)
			load_dex_wrapper(env, obj, res, lhook, descriptor);
		else
			flag = 1;				
	} 
	log("XXX7 HOOK ABILITATO CHIAMO CALLORIGIN\n")
	ret = callOrigin(env, obj, l,res,s);
	if(flag){
		log("XXX7 HOOK ABILITATO CHIAMO DEX WRAPPER\n")
		load_dex_wrapper(env, obj, res, lhook, descriptor);
	}
	//ret = callOrigin2(env,  res, l, obj);
	//dalvik_postcall(&d,res);
	log("XXX7 HOOK ABILITATO ESCO con %p \n", ret)
	return ret;
}
void* hookDisabilitato(JNIEnv *env, jobject obj, va_list l, char*s, struct dalvik_hook_t* res){
	void* ret = NULL;
	int real_args = res->n_iss;//_dvmComputeMethodArgsSize(&d,res->method);
	if(!res->sm){
		real_args--;
	}
	if(debug)
		log("!!!!PRESO MUTEX %s con signature = %s, res = @0x%x, il metodo: @0x%x con mid = 0x%x ha: %d argomenti, skippable = %d, type = %c\n", res->method_name, res->method_sig, res, res->method, res->mid,real_args, res->skip, *s)
	ret = callOrigin(env, obj, l,res,s);
	return ret;
}

void* onetoall(JNIEnv *env, jobject obj, ...){
	//printStackTrace(env);
	log("--------------------- ONETOALL------------obj vale = 0x%x -------------------\n", obj)
	/* PROVA DECODE REF 
	asm volatile("" ::: "memory");
	//jobject gobj = (*env)->NewGlobalRef(env, thiz);
	struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
	log("decodificato = %p\n", nobj)
	log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
	*/

	struct ClassObject* co = _getCallerClass(&d);
	//log("XXX6 GETCALLERCLASS = 0x%x, name = %s \n", co, co->descriptor)
	struct ClassObject* co2 = _getCaller2Class(&d);
	//log("XXX6 GETCALLERCLASS2 = 0x%x \n", co2);
	//log("XXX6 GETCALLERCLASS2 = 0x%x, name = %s \n", co2, co2->descriptor)
	struct ClassObject* co3 = _getCaller3Class(&d);
	//log("XXX6 GETCALLERCLASS3 = 0x%x, name = %s \n", co3, co3->descriptor)
	
	char * cl = (char*)malloc(sizeof(char)*1024);
	char * name = (char*)malloc(sizeof(char)*1024);
	char * descriptor = (char*)malloc(sizeof(char)*1024);
	char * hash = (char*)malloc(sizeof(char)*512);
	memset(cl,0,sizeof(cl));
	memset(name,0,sizeof(name));
	memset(descriptor,0,sizeof(descriptor));
	memset(hash,0,sizeof(hash));
	struct dalvik_hook_t *res;
	
	void *ret = 0;
	
	va_list l, lhook;
	//
	va_start(l,obj);
	va_start(lhook,obj);

	//only for info, try to remove for loop v2
	//get_caller_class(env,obj,cl);
	//metodo chiamante
	get_method(&d,name,descriptor);

	strcpy(hash,name);
	strcat(hash,descriptor);

	//pthread_mutex_lock(&mutex);
	if(debug)
		log("CERCO NELLA LISTA : %s\n", hash)
	res = (struct dalvik_hook_t *) cerca(L, hash);
	//pthread_mutex_lock(&(res->mutexh));
	//pthread_mutex_unlock(&mutex);
	if(!res){
		log("XXX errore RES = 0\n")
	}
	if(is_static(&d, res->method)){
		log("XXX6 METHOD: %s e statico!!\n", res->method_name)
	}
	else{
		log("XXX6 METHOD: %s NON statico!!\n", res->method_name)
	}
	log("ONETOALL %x nome = %s\n", res->originalMethod, res->originalMethod->name)
	memset(cl,0,sizeof(cl));
	strcpy(cl,res->method_sig);
	char *s = parse_signature(cl);
	//dalvik_prepare(&d, res, env);
	int flag = 0;
	if(co){
		if( strstr(co->descriptor, HOOKCLASS) != NULL || strstr(co->descriptor, APICLASS) != NULL ){
			flag = 1;
		} 
		if(co2){
			if( strstr(co2->descriptor, HOOKCLASS) != NULL || strstr(co2->descriptor, APICLASS) != NULL ){
				flag = 1;
			}	 
		}
		if(co3){
			if( strstr(co3->descriptor, HOOKCLASS) != NULL || strstr(co3->descriptor, APICLASS) != NULL ){
				flag = 1;
			}	 
		}
		if(flag){
			log("XXX7 SONO STATO CHIAMATO DAL FRAMEWORK\n")
			ret = callOrigin(env, obj, l,res,s);
			log("XXX7 CHIAMATA DEL FRAMEWORK ret = 0x%x\n",ret)
			//dalvik_postcall(&d,res);
		}
	}
	/**
	if(0){
		;
	}
	*/
	if(flag == 0) {
		/**
		if( strstr(co->descriptor, "Lcom/whatsapp") != NULL ){
			void *target_cls = d.dvmFindLoadedClass_fnPtr(co->descriptor);
			if(target_cls != NULL){
				log("XXX7 TROVATA CLASSE: %s @0x%x\n", co->descriptor, target_cls)
				dalvik_dump_class(&d,co->descriptor);
			}
			log("XXX7 CLASSE WHATSAPP: %s, FIND = 0x%x\n", co->descriptor, target_cls)
		}*/
		if(res->ok){
			log("XXX5 hook abilitato\n")
			ret = hookAbilitato(env, obj, l, res, lhook, descriptor,s);
		}
		else{
			log("XXX5 hook disabilitato\n")
			ret = hookDisabilitato(env, obj,l,s,res );
		}
		if(debug){
			log("XXX uscito metodo: %s con ret = 0x%x\n", res->method_name, ret)
			log("----------------------------------------------------\n")
		}
	}
	free(cl);
	free(name);
	free(descriptor);
	free(hash);
	//pthread_mutex_unlock(&(res->mutexh));
	if(ret != -1){
		return ret;
	}
	return;
}

typedef struct dexth_args{
	JNIEnv* _env;
	struct Method* _original;
	struct Object* _thiz;
	struct dalvik_hook_t*  _res;
	struct Thread* _self;
	JValue* _margs;
}dexth_args;

void* eseguiDex2(JNIEnv*env,struct Method* original, struct Object* thiz, JValue* margs, struct dalvik_hook_t* dh, Thread* self){
//void* eseguiDex2(void* args){
/*
	struct InternalStartArgs* pArgs = (struct InternalStartArgs*) malloc(sizeof(*pArgs));
	memcpy(pArgs, args, sizeof(pArgs));
	log(" controllo pArgs = %p, funcargs = %p \n", pArgs, ((struct InternalStartArgs*)args));
	dexth_args* pDexargs = ((dexth_args*)args);
	//memcpy(pDexargs, pArgs->funcArg, sizeof(dexth_args));
	log(" safety test, original = %p, thiz = %p, dh = %p margs = %p \n", pDexargs->_original, pDexargs->_thiz, pDexargs->_res, pDexargs->_margs)
	JNIEnv *env;
	int status = (*g_JavaVM)->GetEnv(g_JavaVM,(void **) &env, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED)
    {
    	log("ESEGUIDEX2 chiamo attach\n")
        status = (*g_JavaVM)->AttachCurrentThread(g_JavaVM,&env, NULL);
        if (status != JNI_OK)
            log("ESEGUIDEX2 AttachCurrentThread failed %d",status)
        else	
        	log("ESEGUIDEX2 attach success = %d\n", status)
    }
	log("ESEGUIDEX2 preso  myenv = 0x%x, jvm = 0x%x\n", env, g_JavaVM)
	//struct dexth_args* largs = malloc(sizeof(thargs));
	//largs->dexpath = (char*)malloc( );
	//largs->clsname = (char*)malloc( );
	//memcpy(largs, args, sizeof(struct thargs));
	
	struct Object* thiz = pDexargs->_thiz;
	struct Thread* self = pDexargs->_self;
	struct Method* original = pDexargs->_original;
	struct dalvik_hook_t* dh = pDexargs->_res;
	JValue* margs = pDexargs->_margs;
	*/
	
	log(" CONTROLLO thiz = %p, self = %p, original = %p , dh = %p , margs = %p \n", thiz, self, original, dh, margs)
	int real_args = dh->n_iss;//_dvmComputeMethodArgsSize(&d,res->method);
	log("ESEGUIDEX2 metodo %s ha %d argomenti, margs = %p, margs0 = %p\n", original->name, real_args, margs, margs->l)

	JValue result;
	jclass clazz;;
	jobject gja;

	struct ClassObject* pCo;

	struct Object* oo = d.dvmDecodeIndirectRef_fnPtr(self, dh->ref);
	log("ESEGUIDEX2 decodificata ref = %p, risultato = %p \n", dh->ref, oo)

	if(debug)
		log("ESEGUIDEX2 ho un dex method da chiamare = %s!!!\n", dh->dex_meth)
	if(!dh->loaded){
		if(debug)
			log("ESEGUIDEX2 DEVO CARICARE LA CLASSE DAL DEX %s\n", dh->dex_class)
		clazz = (jclass *)dexstuff_defineclass(&d, dh->dex_class, cookie);
		pCo = (struct ClassObject*) _dvmFindSystemClass(&d, DALVIKHOOKCLS_2);
		log("ESEGUIDEX2 trovata dalvikhookcls = %p \n", pCo)
		dh->loaded=1;
		dh->DexHookCls = pCo;
		//change v2
		//mycls = (*env)->FindClass(env, DALVIKHOOKCLS);
		//gclazz = (*env)->NewGlobalRef(env, mycls);
		//dh->DexHookCls = gclazz;
	}
	else{
		log("ESEGUIDEX2 DEX CLASS CACHED!!!\n")
	}
	if(thiz != NULL){
		log("ESEGUIDEX2 DENTRO SET THIZ, cerco in classe = %p \n",dh->DexHookCls)
		struct Method* set_thiz = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(dh->DexHookCls, "setThiz" , "(Ljava/lang/Object;)V");
		log("ESEGUIDEX2 trovato setThiz = %p, chiamo con oo=%p, result = %p, thiz = %p \n", set_thiz, oo, &result, thiz)
		jvalue arg[1];
		arg[0].l = thiz;
		if(set_thiz){
			d.dvmCallMethodA_fnPtr(self,set_thiz,oo,false,&result, arg);
			log("ESEGUIDEX2 chiamato setThiz\n")
		}
	}
	struct Method* initid = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(dh->DexHookCls, "initFunc" , "()V");
	log("ESEGUIDEX2 prima di call initid\n")
	d.dvmCallMethodA_fnPtr(self,initid,oo,false,&result, margs);
	log("ESEGUIDEX2 dopo call initid\n")
	//prova bug SIGBUS
	struct Method* dex_meth = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(dh->DexHookCls, "myexecute" , "([Ljava/lang/Object;)V");
	//struct Method* dex_meth = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(dh->DexHookCls, "myexecute" , "()V");
	log("ESEGUIDEX2 trovato myexecute %p \n", dex_meth)
	char* pshorty = &original->shorty[1];
	struct ClassObject* pDio = _dvmFindSystemClass(&d, "Ljava/lang/reflect/Array;");
	struct ClassObject* pDioObj = _dvmFindSystemClass(&d, "Ljava/lang/Object;");
	log("trovata array class = %p e obj class = %p \n", pDio, pDioObj)
	//dalvik_dump_class(&d, "Ljava/lang/reflect/Array;");
	struct Method* pDioM = d.dvmFindDirectMethodHierByDescriptor_fnPtr(pDio, "createObjectArray" , "(Ljava/lang/Class;I)Ljava/lang/Object;");
	struct Method* pDioM2 = d.dvmFindDirectMethodHierByDescriptor_fnPtr(pDio, "set" , "(Ljava/lang/Object;ILjava/lang/Object;)V");
	struct Method* pDioMint = d.dvmFindDirectMethodHierByDescriptor_fnPtr(pDio, "setInt" , "(Ljava/lang/Object;II)V");

	//struct Object* diocane = d.dvmAllocObject_fnPtr(margs->l->clazz,0);

	log("TROVATO METODO createobjarray = %p, set = %p,  \n", pDioM, pDioM2)
	jvalue args2[2];
	args2[0].l = pDioObj;
	args2[1].i = real_args;
	log("chiamo metodo\n")
	d.dvmCallMethodA_fnPtr(self, pDioM, NULL,false, &result, args2 );
	struct Object* mioArray;
	log("object size = %d\n", oo->clazz->objectSize)
	mioArray = calloc(1,sizeof(struct Object));//d.dvmMalloc_fnPtr(oo->clazz->objectSize,0x01); //ALLOW TRACK
	mioArray = result.l;
	log("chiamato metodo, ottenuto %p \n", mioArray)
	/* PROVA SET ELEMENT
	

	args3[0].l = mioArray;
	args3[1].i = 0;
	args3[2].l = margs->l;
	d.dvmCallMethodA_fnPtr(self, pDioM2, NULL,false, &unused, args3 );
	log("chiamato metodo  set\n")
	*/
	
	//jvalue args3[3];
	//struct ClassObject* objectArrayclass = _dvmFindArrayClass(&d,"[Ljava/lang/Object;", NULL);
	//struct ArrayObject* argsArray = _dvmAllocArrayByClass(&d, objectArrayclass, strlen(original->shorty) - 1, 0x0); 
	//log("ESEGUIDEX2 allocato arrayobject %p\n", argsArray)
	JValue args3[3];
	JValue unused;
	int i = 0;
	for(i=0;i<real_args;i++){
		log("ESEGUIDEX2  DEBUG1 = %c\n", *pshorty)
		switch(*pshorty){
			case 'Z':	
				//log("ESEGUIDEX2 dentro boolean creo oggetto\n")
				//jbobj = (*env)->NewObject(env, jbclazz, idb, margs[i].z);
				//log("ESEGUIDEX2 creato oggetto!!\n")
				//(*env)->SetObjectArrayElement(env, gja, i, jbobj);
				//log("ESEGUIDEX2 inserito oggetto nell'array\n")
				//jaboolean = 1;	
				pshorty++;
				break;
			case 'I':
				log("ESEGUIDEX2 argomento[%d] tipo int = %p\n", i, margs[i].i)
				//jbobj = (*env)->NewObject(env, jiclazz, idi, margs[i].i);
				//(*env)->SetObjectArrayElement(env,gja,i,jbobj);
				//_dvmSetObjectArrayElement(&d,argsArray, i, margs[i].i);
				args3[0].l = mioArray;
				args3[1].i = i;
				args3[2].i = margs[i].i;
				d.dvmCallMethodA_fnPtr(self, pDioMint, NULL,false, &unused, args3 );
				pshorty++;
				break;
			case 'L':
				log("ESEGUIDEX2 argomento[%d] tipo object = %p\n", i, margs[i].l )
				//(*env)->SetObjectArrayElement(env,gja,i,(void*)margs[i].l);
				//_dvmSetObjectArrayElement(&d,argsArray, i, margs[i].l);
				args3[0].l = mioArray;
				args3[1].i = i;
				args3[2].l = margs[i].l;
				d.dvmCallMethodA_fnPtr(self, pDioM2, NULL,false, &unused, args3 );
				pshorty++;
				break;
			default:
				log("ESEGUIDEX2 argomento[%d] di tipo %c \n", i, pshorty)
				break;
		}
	}
	
	log("ESEGUIDEX2 impostato gli argomenti: %d !\n", real_args)
	log("ESEGUIDEX2 chiamo metodo in classe = 0x%x con mid = 0x%x, margs = %p, mioArray = %p \n", dh->ref, dex_meth, margs, mioArray)
	JValue* dexargs;
	dexargs = calloc(1, sizeof(JValue));
	dexargs->l = mioArray;
	d.dvmCallMethodA_fnPtr(self,dex_meth,oo,false,&unused, dexargs );	//change bug SIGBUS
	//free(argsArray);
	//(*env)->DeleteGlobalRef(env, gja);
	//free(dexargs);
	//free(mioArray);
	log("finito eseguidex2\n")
}

static void callOriginalV2(struct Method* original, struct Object* thiz, Thread* self, JValue* pResult, JValue* margs){
	JValue result;
	log("callOriginalV2 CHIAMO DVMCALLMETHODV = %s, self = %p,original = %p, result = %p, myargs = %p\n", original->name, self,original,pResult,margs)	
	d.dvmCallMethodA_fnPtr(self,original,thiz,false,&result, margs);
	// exceptions are thrown to the caller
    if (d.dvmCheckException_fnPtr(self)) {
    	log("ONETOALL2 TROVATA EXCEPTION!!! \n")
        return;
    }   
	struct ClassObject* returnType = d.dvmGetBoxedReturnType_fnPtr(original);
	log("preso returnTYpe = %p, primitive = %d \n", returnType, returnType->primitiveType)
    if (returnType->primitiveType == PRIM_VOID) {
        // ignored
    } else if (result.l == NULL) {
        if (d.dvmIsPrimitiveClass_fnPtr(returnType)) {
            log("null result when primitive expected!!!!!!!!!!!\n");
        }
        pResult->l = NULL;

    } else {
        //if (!d.dvmUnboxPrimitive_fnPtr(result.l, returnType, pResult)) {
        //    log("EXCEPTION!!! %p, %p", result.l->clazz, returnType);
        //}
        if(returnType->primitiveType == PRIM_NOT){
        	log("Tipo NON PRIMITIVO\n")
        	pResult->l = result.l;
        }
        else{
        	log("HO UN TIPO PRIMITIVO\n")
        	switch(returnType->primitiveType){
        		case PRIM_BOOLEAN:
        			pResult->z = result.z;
        			break;
        		case PRIM_INT:
        			log("ritorno un INTERO = %d \n", result.i)
        			pResult->i = result.i;
        			break;
        		case PRIM_CHAR:
        			pResult->c = result.c;
        			break;
        	}

        }        
    }
}


void* onetoall2(u4* args, JValue* pResult, struct Method* method, struct Thread* self){
	//COFFEE_TRY() {
	struct Method* original =(struct Method*) method->insns;
	log("DENTRO ONETOALL2\n")
	log("sono stato chiamato da original name %s, insns = %p, native = %p \n", original->name, original->insns, original->nativeFunc)
	log("sono stato chiamato da pointer name %s, insns = %p, native = %p\n", method->name, method->insns, method->nativeFunc);

	struct ClassObject* co = _getCallerClass(&d);
	log("XXX6 GETCALLERCLASS = 0x%x, name = %s \n", co, co->descriptor)
	struct ClassObject* co2 = _getCaller2Class(&d);
	struct ClassObject* co3 = _getCaller3Class(&d);
	
	bool check = d.dvmIsNativeMethod_fnPtr(original);
	bool check2 = d.dvmIsNativeMethod_fnPtr(method);
	JNIEnv *env = (JNIEnv*) get_jni_for_thread(&d);
	log("ONETOALL2 ORIGINAL METHOD: %s, is native = %d \n",original->name, check)
	log("ONETOALL2 HOOKED METHOD: %s, is native = %d \n",method->name, check2)

	/*
	char* cl= d.dvmMalloc_fnPtr(sizeof(char)*1024, 0x2);
	char* name= d.dvmMalloc_fnPtr(sizeof(char)*1024, 0x2);
	char* descriptor= d.dvmMalloc_fnPtr(sizeof(char)*1024, 0x2);
	char* hash= d.dvmMalloc_fnPtr(sizeof(char)*1024, 0x2);
	*/
	
	char * cl = (char*)malloc(sizeof(char)*1024);
	char * name = (char*)malloc(sizeof(char)*1024);
	char * descriptor = (char*)malloc(sizeof(char)*1024);
	char * hash = (char*)malloc(sizeof(char)*2048);



	if(!cl || !name || !descriptor || !hash ){
		log("MALOCC ERORR!!!\n")
		return;
	}
	
	memset(cl,0,sizeof(cl));
	memset(name,0,sizeof(name));
	memset(descriptor,0,sizeof(descriptor));
	memset(hash,0,sizeof(hash));
	struct dalvik_hook_t *res;

	//only for info, try to remove for loop v2
	//get_caller_class(env,obj,cl);
	//metodo chiamante
	get_method(&d,name,descriptor);

	strcpy(hash,name);
	strcat(hash,descriptor);

	pthread_mutex_lock(&mutex);
	if(debug)
		log("CERCO NELLA LISTA : %s\n", hash)
	res = (struct dalvik_hook_t *) cerca(L, hash);
	pthread_mutex_unlock(&mutex);
	log("preso res = %p, name = %s\n", res, res->clname)
	int len = res->n_iss;
	log("alloco array di %d elementi \n", len)
	//se non funziona un cazzo cambia il valore a jvalue
	
	const char* desc = &original->shorty[1]; //0 is return type
	struct Object* thiz = NULL;
	size_t srcIndex = 0;
	size_t dstIndex = 0;
	if(!is_static(&d, original)){
		log("ONETOALL2 PRENDO THIZ = %p \n", args[0])
		thiz = (struct Object*)args[0];
		srcIndex++;
	}
	JValue* margsCopy;// = margs;
	margsCopy = calloc(len ,sizeof(JValue));
	JValue* margs;
	margs = calloc(len, sizeof(JValue));
	log("ONETOALL2 metodo statico = %d \n", is_static(&d, original))

    /*
	if(strlen(res->dex_meth) > 0){
		log("chiamo eseguidex2 \n");
		dexth_args* thargs = malloc(sizeof( dexth_args));
		log(" creata struttura th = %p \n", thargs)

		thargs->_env = env;
		thargs->_original = original;
		thargs->_thiz = thiz;
		thargs->_res = res;
		thargs->_self = self;
		thargs->_margs =  margs;
		log(" controllo struttura thargs = %p, original = %p, thiz = %p, self = %p, margs = %p \n",thargs, thargs->_original,thargs->_thiz, thargs->_self, thargs->_margs)
		log(" creo il thread\n")
		pthread_t thDex2;
		char* name = "";
		log(" creo il dalvik  thread  con thDex2 = %p, name = %p, eseguidex2 = %p , thargs = %p!!!\n", &thDex2, name, &eseguiDex2, thargs)
		d.dvmCreateInternalThread_fnPtr(&thDex2, name, &eseguiDex2, thargs);
		_dumpAllT(&d);
		log(" CREATO CREATORE \n")
		*/
		/*

		memcpy(thargs->_env, env, sizeof(JNIEnv));
		memcpy(thargs->_original, original, sizeof(struct Method));
		memcpy(thargs->_thiz, thiz, sizeof(struct Object));
		thargs->_res = res;
		thargs->_self = self;
		thargs->_margs =  margs;
		log(" creo il thread\n")
		int rc = pthread_create(&pty_t, NULL, eseguiDex2, thargs);
	    if(rc){
	    	log("XXX3 ERROR PTHREAD CREATE: %d\n", rc)
	    }
	    else{
	    	log("XXX4 CREATO THREAD  = 0x%x\n", rc)
	    }
	    */
	int k =0 ;
	while(*desc != '\0'){
		char descChar = *(desc++);
		/*
		struct ClassObject* pCo = (struct ClassObject*)_dvmFindPrimitiveClass(&d,descChar);
		log("ONETOALL2 preso findprimitiveclass %p \n", pCo)
		if(pCo)
			log("ONETOALL2 primitiveType = %p, desc = %s \n", pCo->primitiveType, pCo->descriptor)
		*/
		//log("inizio switch, shorty = %c, pCo = %p \n", descChar, pCo)
		switch(descChar){
			case 'Z':
			case 'C':
			case 'F':
			case 'B':
			case 'S':
			case 'I':
				margs[k].i = args[srcIndex];
				log("dentro I, %d, srcIndex = %d args = %p \n", margs[k].i, srcIndex, args[srcIndex])
				k++; srcIndex++;
				break;
			case 'D':
			case 'J':
				//da provare
				log("dentro J, k = %d, srcindex = %d, args = %p\n",k,srcIndex, args[srcIndex])
				//margs[k++].j = _dvmGetArgLong(args, srcIndex);
				memcpy(&margs[k++].j, &args[srcIndex], 8);
				srcIndex += 2;
				break;
			case '[':
			case 'L':
				log("dentro L, k = %d, srcindex = %d, args = %p\n",k,srcIndex, args[srcIndex])
				margs[k].l = args[srcIndex];
				k++; srcIndex++;
				break;
			default:
				log("ONETOALL2 DESCCHAR NON RICONOSCIUTO %c\n", descChar)
				srcIndex++;
		}
	}

	memcpy(margsCopy, margs, sizeof(margs));
	log("DEBUG MARGSCOPY, 0 = %p, \n", margsCopy->l)

	callOriginalV2(original, thiz, self, pResult, margs);
	

    int flag =0;
	if(co){
		if( strstr(co->descriptor, HOOKCLASS) != NULL || strstr(co->descriptor, APICLASS) != NULL ){
			flag = 1;
		} 
		if(co2){
			if( strstr(co2->descriptor, HOOKCLASS) != NULL || strstr(co2->descriptor, APICLASS) != NULL ){
				flag = 1;
			}	 
		}
		if(co3){
			if( strstr(co3->descriptor, HOOKCLASS) != NULL || strstr(co3->descriptor, APICLASS) != NULL ){
				flag = 1;
			}	 
		}
		if(flag){
			log("XXX7 SONO STATO CHIAMATO DAL FRAMEWORK\n")
			log("ONETOALL2 FINITO!!!\n")
			goto SAFE;
		}
	}
	//goto SAFE;
	if(strlen(res->dex_meth) > 0){
		log("chiamo eseguidex2 \n");
		if(!res->dexAfter)
			eseguiDex2(env, original, thiz, margsCopy, res, self);
		else
			flag = 1;
	}
/*	
	
	} COFFEE_CATCH() {
		const char* message = coffeecatch_get_message();
		log("**FATAL ERROR: %s\n", message)
	}COFFEE_END();
	*/
SAFE:
	free(cl);
	free(name);
	free(descriptor);
	free(hash);
	free(margsCopy);
	log("ONETOALL2 FINITO!!!\n")
	return;

}
static void _createStruct( JNIEnv* env, jobject thiz, jobject clazz )
{
	pthread_mutex_lock(&mutex);
	if(debug)
		log("------------ CREATE STRUCT!!! --------  clazz = 0x%x, thiz = 0x%x\n", clazz, thiz)
	jclass dalvikCls = (*env)->FindClass(env, DALVIKHOOKCLS);
	log("XXX4 create struct DALVIK HOOK CLASS = 0x%x\n", dalvikCls)
	//dalvik_dump_class(&d, "Lorg/sid/addi/core/manageADDI;");
	jmethodID clname = (*env)->GetMethodID(env,dalvikCls,"get_clname","()Ljava/lang/String;");
	jmethodID method_name = (*env)->GetMethodID(env,dalvikCls,"get_method_name","()Ljava/lang/String;");
	jmethodID method_sig = (*env)->GetMethodID(env,dalvikCls,"get_method_sig","()Ljava/lang/String;");
	jmethodID dexmethod= (*env)->GetMethodID(env,dalvikCls,"get_dex_method","()Ljava/lang/String;");
	jmethodID dexclass= (*env)->GetMethodID(env,dalvikCls,"get_dex_class","()Ljava/lang/String;");
	jmethodID hashid = (*env)->GetMethodID(env,dalvikCls,"get_hashvalue","()Ljava/lang/String;");
	//jmethodID argsid = (*env)->GetMethodID(env,dalvikCls,"get_myargs","()I");
	//jmethodID nsid = (*env)->GetMethodID(env,dalvikCls,"getNs","()I");
	jmethodID sid = (*env)->GetMethodID(env,dalvikCls,"isSkip","()I");

	void * c = (*env)->CallObjectMethod(env,clazz,clname);
	void * c2 = (*env)->CallObjectMethod(env,clazz,method_name);
	void* c3 = (*env)->CallObjectMethod(env,clazz,method_sig);
	void* c4 = (*env)->CallObjectMethod(env,clazz,dexmethod);
	void* c6 = (*env)->CallObjectMethod(env,clazz,dexclass);
	void *c5 = (*env)->CallObjectMethod(env,clazz,hashid);
	//jint args = (*env)->CallIntMethod(env,clazz,argsid);
	//jint ns = (*env)->CallIntMethod(env,clazz,nsid);
	jint flag = (*env)->CallIntMethod(env,clazz,sid);
	
	char *s = (*env)->GetStringUTFChars(env, c , NULL);
	char *s2 = (*env)->GetStringUTFChars(env, c2 , NULL);
	char *s3 = (*env)->GetStringUTFChars(env, c3 , NULL);
	char *s4 = (*env)->GetStringUTFChars(env, c4 , NULL);
	char *s5 = (*env)->GetStringUTFChars(env,c5,NULL); //hashvalue
	char *s6 = (*env)->GetStringUTFChars(env, c6 , NULL);
	struct dalvik_hook_t *dh;
	
	dh = (struct dalvik_hook_t *)malloc(sizeof(struct dalvik_hook_t));
	//dh = d.dvmMalloc_fnPtr(sizeof(struct dalvik_hook_t), 0x0);
	//dh->real_args = args;
	dh->skip = flag;
	dh->loaded = 0;
	dh->ok =1;
	dh->ref = (*env)->NewGlobalRef(env, clazz);
	dh->dexAfter  = 0; //di default gli hook sono before origin

	strcpy(dh->dex_meth,"");
	if(strlen(s4) > 0){
		strcpy(dh->dex_meth,s4);
		strcpy(dh->dex_class,s6);	
	}
	char *pointer;
	pointer = parse_signature(s3);	
	int  check;
	log("CREATE STRUCT PIAZZO  hook \n")
	if(*pointer == 'F'){
		pthread_mutex_unlock(&mutex);
		return;
		//hook a method that return jfloat
		if(!dalvik_hook_setup(dh, s,s2,s3,&jfloat_wrapper))
			log("----------- HOOK SETUP FALLITO ------------- : %s, %s\n", c,c2)
		if(!dalvik_hook(&d,dh, env))
			log("----------- HOOK FALLITO ------------- : %s, %s\n", c,c2)
		log("HOOK PLACED\n")		
	}
	else if(*pointer == 'J'){
		pthread_mutex_unlock(&mutex);
		return;
		if(!dalvik_hook_setup(dh, s,s2,s3,&jlong_wrapper))
			log("----------- HOOK SETUP FALLITO ------------- : %s, %s\n", c,c2)
		if(!dalvik_hook(&d,dh,env))
			log("----------- HOOK FALLITO ------------- : %s, %s\n", c,c2)
		log("HOOK PLACED\n")
	}else if(*pointer == 'V'){
		if(!dalvik_hook_setup(dh, s,s2,s3,&onetoall2))
			log("----------- HOOK SETUP FALLITO ------------- : %s, %s\n", c,c2)
		check =  dalvik_hook(&d, dh,env);
		log("XXX7  check vale = 0x%x, %d\n",check, check)
		if(check == 0x0){
			log("----------- HOOK VOID FALLITO ------------- : %s, %s\n", s,s2)
			free(dh);
			pthread_mutex_unlock(&mutex);
			return;
		}
		else
			log("HOOK VOID PLACED, nome = %s\n", dh->originalMethod->name)			
	}
	else{
		//return;
		if(!dalvik_hook_setup(dh, s,s2,s3,&onetoall2))
			log("----------- HOOK SETUP FALLITO ------------- : %s, %s\n", c,c2)
		check =  dalvik_hook(&d, dh, env);
		log("XXX7  check vale = 0x%x, %d\n",check, check)
		if(check == 0x0){
			log("----------- HOOK FALLITO ------------- : %s, %s\n", s,s2)
			free(dh);
			pthread_mutex_unlock(&mutex);
			return;
		}
		else
			log("HOOK PLACED\n")	
		//log("XXX5 class loader = 0x%x\n", dh->method->clazz->classLoader)
	}
	if(debug)
		log("XXXD inserisco dh = 0x%x 0x%x, %s,%s,%s, hash = %s\n", &dh, dh, s,s2,s3,s5)
	int i = inserisci(&L,dh,s,s2,s3,s5);
	if(debug)
		log("stampo lista: %d, dex_method = %s\n", i, dh->dex_meth)
	pthread_mutex_unlock(&mutex);
}

void* ptyServer(){

	log("XXX5 DENTRO THREADID = %d  PID = %d\n", pthread_self(), getpid())
	JNIEnv *envLocal;
    int status = (*g_JavaVM)->GetEnv(g_JavaVM,(void **) &envLocal, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED)
    {
    	log("XXX5 chiamo attach\n")
        status = (*g_JavaVM)->AttachCurrentThread(g_JavaVM,&envLocal, NULL);
        if (status != JNI_OK)
            log("XXX5 AttachCurrentThread failed %d",status)
        else	
        	log("XXX5 attach success = %d\n", status)
    }
	log("XXX5 preso env  = 0x%x, myenv = 0x%x, jvm = 0x%x\n", envLocal, envLocal, g_JavaVM)


	jclass mycls = (*envLocal)->FindClass(envLocal, "org/sid/addi/core/Session");
	log("XXX5 trovata classe controller = 0x%x\n", mycls)

	jmethodID constructor = (*envLocal)->GetMethodID(envLocal, mycls, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
	//dalvik_dump_class(&d, "Landroid/content/pm/PackageManager;");
	
	char *res = malloc(sizeof(char)*128);
	char *req = malloc(sizeof(char)*128);

	//char* res= d.dvmMalloc_fnPtr(sizeof(char)*128, 0x2);
	//char* req= d.dvmMalloc_fnPtr(sizeof(char)*128, 0x2);

	strcpy(res,"/data/local/tmp/res.fifo");
	strcpy(req,"/data/local/tmp/req.fifo");
	//sprintf(res,"/data/local/tmp/req%d.fifo",getpid());
	//sprintf(req,"/data/local/tmp/res%d.fifo", getpid());
	if (mkfifo(res, 0777) == -1) { 
		if (errno!=EEXIST) {
			log("XXX5 pty error %s\n", strerror(errno))
		}
	}
	if (mkfifo(req, 0777) == -1) { 
		if (errno!=EEXIST) {
			log("XXX5 pty error %s\n", strerror(errno))
		}
	}
	jobject str = (*envLocal)->NewStringUTF(envLocal, req);
	jobject gstr = (*envLocal)->NewGlobalRef(envLocal, str);
	jobject str1 = (*envLocal)->NewStringUTF(envLocal, res);
	jobject gstr1 = (*envLocal)->NewGlobalRef(envLocal, str1);
	log("XXX5 CHIAMO NEW OBJECT\n")
	jobject mobj = (*envLocal)->NewObject(envLocal, mycls, constructor, gstr, gstr1); 
	jmethodID start = (*envLocal)->GetMethodID(envLocal, mycls, "start", "()V");
	log("XXX5 chiamo metodo su obj = 0x%x, start = 0x%x\n", mobj, start)

	(*envLocal)->CallVoidMethod(envLocal, mobj, start, NULL);
	(*g_JavaVM)->DetachCurrentThread(g_JavaVM);
	free(res);
	free(req);
	log("XXX5 ESCO DA THREAD\n")
	sleep(5);
	pthread_exit(1);
}

void* _createPTY(){
	log("XXX4 CREATE PTY\n")
    int rc = pthread_create(&pty_t, NULL, ptyServer, NULL);
    if(rc){
    	log("XXX3 ERROR PTHREAD CREATE: %d\n", rc)
    }
    else{
    	log("XXX4 CREATO THREAD  = 0x%x\n", rc)
    }
}

void * _wrapperSuspendAll(){
	log("XXX5 chiamato wrapper suspend\n")
	_suspendAllT(&d);
	//_dumpAllT(&d);
	log("XXX5 FINE wrapper suspend\n")
}
void * _wrapperResumeAll(){
	log("XXX5 chiamato wrapper resume\n")
	_resumeAllT(&d);
	//_dumpAllT(&d);
	log("XXX5 FINE wrapper resume\n")
}

struct thargs{
	char* dexpath;
	char* clsname;
};

void* _th_wrapper(struct thargs* args){
	log("XXX7 DENTRO THREAD WRAPPER\n")
	JNIEnv *envLocal;
	int status = (*g_JavaVM)->GetEnv(g_JavaVM,(void **) &envLocal, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED)
    {
    	log("XXX5 chiamo attach\n")
        status = (*g_JavaVM)->AttachCurrentThread(g_JavaVM,&envLocal, NULL);
        if (status != JNI_OK)
            log("XXX5 AttachCurrentThread failed %d",status)
        else	
        	log("XXX5 attach success = %d\n", status)
    }
	log("XXX5 preso  myenv = 0x%x, jvm = 0x%x\n", envLocal, g_JavaVM)
	struct thargs* largs = (struct thargs*)malloc(sizeof(struct thargs));
	largs->dexpath = (char*)malloc( (strlen(args->dexpath)+1) * sizeof(char));
	largs->clsname = (char*)malloc( (strlen(args->clsname)+1) * sizeof(char));
	memcpy(largs, args, sizeof(struct thargs));
	log("XXX7 dentro thread wrapper:\n")
	log("ricevuto: %s = %s, %s = %s\n", args->dexpath, largs->dexpath, args->clsname, largs->clsname)
	dalvik_dump_class(&d, "Ljava/lang/VMClassLoader;");
	(*g_JavaVM)->DetachCurrentThread(g_JavaVM);
	free(largs);
	free(args->dexpath);
	free(args->clsname);
	free(args);
	pthread_exit(1);
}

void* _newThread(JNIEnv*env, jobject thiz,  jobject dexpath, jobject clsname){
	log("XXX7 dentro NEW THREAD\n")
	pthread_t tmpThread = NULL;

	struct thargs* args = (struct thargs*)malloc(sizeof(struct thargs));
	char *dex_p = (*env)->GetStringUTFChars(env, dexpath, NULL);
	char *clsn = (*env)->GetStringUTFChars(env, clsname, NULL);	
	args->dexpath = (char*)malloc( (strlen(dex_p)+1) * sizeof(char));
	args->clsname = (char*)malloc( (strlen(clsn)+1) * sizeof(char));
	if (dex_p && clsn ) {
		strcpy(args->dexpath, dex_p);
		strcpy(args->clsname, clsn);
		log("ricevuto: %s = %s, %s = %s\n", args->dexpath, dex_p, args->clsname, clsn)
	}
	int rc = pthread_create(&pty_t, NULL,_th_wrapper, args);
}

void *_loadSystemClass(JNIEnv *env, jobject str){
	void* vmloader = (*env)->FindClass(env, "java/lang/VMClassLoader");
	char *clsname = (*env)->GetStringUTFChars(env, str, NULL);
	jmethodID loadClassid = (*env)->GetStaticMethodID(env, vmloader,"loadClass", "(Ljava/lang/String;Z)Ljava/lang/Class;");
	if(loadClassid){
		jclass res2 = (*env)->CallStaticObjectMethod(env, vmloader, loadClassid, str, 0x0);
		if(res2){
			return res2;
		}
		else
			return NULL;
	}
}

void* startMagic(JNIEnv *env){
	/**
	struct DvmDex* pdex;
	int res = _dvmDexFileOpenFromFd(&d,cookie, pdex);
	log("CI SONO1 %d\n",res)
	*/
	//devo aprire il DEX
	struct DexOrJar* dexfd = dexstuff_loaddex(&d, "/mnt/sdcard/whatsapp.dex");
	if(dexfd){
		log("XXX7 PROVA PROVA dex at = 0x%x, 0x%x, 0x%x, %s\n", dexfd->pDexMemory, dexfd->isDex,dexfd->pRawDexFile,dexfd->fileName)
		struct RawDexFile* p = dexfd->pRawDexFile;
		log("CI SONO1\n")
		if(p){
			log("CI SONO2\n")
			if(p->pDvmDex){
				log("CI SONO3\n")
				struct DvmDex* p2 = p->pDvmDex;
				log("CI SONO4\n")
				if(p2){
					log("CI SONO5\n")
					log("XXX7 p2 = %p, pp = %p, memmap = 0x%x, dexfile = 0x%x\n", p2, p->pDvmDex,  p2->memMap, p2->pDexFile)
					struct MemMapping mm = p2->memMap;
					log("CI SONO6\n")
					log("memmap add = %p,%p length = %d\n", mm.addr, p2->memMap, mm.length)
					u1 *a = mm.addr;
					log("value %p = 0x%02x\n",a+0x28, *a)
					//int res = _sysChangeMapAccess(0x02, 1, 1, NULL);
					u1 new = 0x66;
					//_dvmDexChangeDex1(p2, (u1*) a+0x28, new);
					log("value %p = 0x%02x\n",a, *a)
					log(" dump memmapping ")
					log("  addr = %p , length = %zu, base = %p, baselength = %zu\n", mm.addr, mm.length, mm.baseAddr, mm.baseLength)
					//log("ACCESS CHANGED!!\n")
					struct DexFile* p3 = p2->pDexFile;
					mprotect((void*)mm.addr, mm.addr + mm.length, PROT_READ|PROT_WRITE|PROT_EXEC);
					if(p3){
						struct DexHeader*  p4 = p3->pHeader;
						log("dexfile length = %d \n", p4->fileSize)
					}
				}
			}
		}
		log("CI SONO END\n")
		//log("memmap = 0x%x, dexfile = 0x%x, base =0x%x\n",dexfd->pRawDexFile->pDvmDex->memMap, dexfd->pRawDexFile->pDvmDex->pDexFile,
		//			dexfd->pRawDexFile->pDvmDex->pDexFile->baseAddr)	
	}
	
}

void* createAndInsertDhook(JNIEnv *env, char* clsname, char* methodname, char* methodsig){
	log("DENTRO CREATE AND INSERT DHOOK, clanme = %s, mname = %s, msid = %s\n", clsname, methodname, methodsig)
	pthread_mutex_lock(&mutex);
	char * cl = calloc(1024, sizeof(char));
	char * name = calloc(1024, sizeof(char));
	char * descriptor = calloc(1024,sizeof(char));
	
	memcpy(cl, clsname, sizeof(clsname)*strlen(clsname));
	memcpy(name, methodname, sizeof(methodname)*strlen(methodname));
	memcpy(descriptor, methodsig, sizeof(methodsig)*strlen(methodsig));
	
	log("createAndInsertDhook, copiato cl = %s, name = %s, sign = %s \n ", cl, name, descriptor)
	struct dalvik_hook_t *dh;
	jobject clazz = NULL;
	dh = malloc(sizeof(struct dalvik_hook_t));
	//dh->real_args = args;
	dh->skip = 0;
	dh->loaded = 0;
	dh->ok =1;
	dh->ref = NULL;//(*env)->NewGlobalRef(env, clazz);
	dh->dexAfter  = 0; //di default gli hook sono before origin
	strcpy(dh->dex_meth,"");
	char *pointer;
	pointer = parse_signature(methodsig);	
	int  check;
	if(*pointer == 'F'){
		pthread_mutex_unlock(&mutex);
		free(dh);
		return;
		//hook a method that return jfloat
		if(!dalvik_hook_setup(dh, clsname, methodname,methodsig,jfloat_wrapper))
			log("----------- HOOK SETUP FALLITO ------------- : %s, %s\n", clsname, methodname)
		if(!dalvik_hook(&d,dh,env))
			log("----------- HOOK FALLITO ------------- : %s, %s\n", clsname, methodname)
		log("HOOK PLACED\n")		
	}
	else if(*pointer == 'J'){
		free(dh);
		pthread_mutex_unlock(&mutex);
		return;
		if(!dalvik_hook_setup(dh,clsname, methodname,methodsig,jlong_wrapper))
			log("----------- HOOK SETUP FALLITO ------------- : %s, %s\n",clsname, methodname)
		if(!dalvik_hook(&d,dh,env))
			log("----------- HOOK FALLITO ------------- : %s, %s\n", clsname, methodname)
		log("HOOK PLACED\n")
	}
	else if(*pointer == 'I'){
		free(dh);
		pthread_mutex_unlock(&mutex);
		return 0;
	}
	else if(*pointer == 'V'){
		if(!dalvik_hook_setup(dh, cl, name,descriptor,&onetoall2))
			log("----------- HOOK SETUP FALLITO ------------- : %s, %s\n", clsname, methodname)
		check =  dalvik_hook(&d, dh,env);
		log("XXX7  check vale = 0x%x, %d\n",check, check)
		if(check == 0x0){
			log("----------- HOOK FALLITO ------------- : %s, %s\n",clsname, methodname)
			free(dh);
			pthread_mutex_unlock(&mutex);
			return;
		}
		else
			log("HOOK PLACED\n")	
		//log("XXX5 class loader = 0x%x\n", dh->method->clazz->classLoader)
	}
	else{
		pthread_mutex_unlock(&mutex);
		free(dh);
		return;		
	}
	log("DENTRO CREATE AND INSERT DHOOK22222, clanme = %s, mname = %s, msid = %s\n", cl, name, descriptor)
	char* hash = calloc(2048 , sizeof(char));
	strcpy(hash,cl);
	strcat(hash,name);
	strcat(hash, descriptor);
	log("CREATE AND INSERT DHOOK INSERISCO HASH = %s\n", hash)
	int i = inserisci(&L,dh,cl, name, descriptor, hash);
	if(debug)
		log("stampo lista: %d, dex_method = %s\n", i, dh->dex_meth)
	log("createAndInsertDhook Ho inserito: name = %s, mname = %s, sig = %s \n", dh->clname, dh->method_name, dh->method_sig)
	//free(dh);
	free(hash);
	free(cl);
	free(name);
	free(descriptor);
	pthread_mutex_unlock(&mutex);
}

//str deve essere dot o slash format senza L;
void* _dumpJavaClass(JNIEnv *env, jobject thiz, jobject str, jobject st2){
	//dalvik_dump_class(&d,"Ljavax/crypto/spec/PBEKeySpec;");
	char *clsname = (*env)->GetStringUTFChars(env, str, NULL);
	clsname = dvmDotToSlash(clsname);
	size_t len = strlen(clsname);
	char *clsnew = (char*)malloc(sizeof(char) * (len+3));
	clsnew[0]  = 'L';
	strcpy(clsnew+1, clsname);
	len = strlen(clsname);
	clsnew[len+1] = ';';
	clsnew[len+2] = '\0';
	struct ClassObject* res = (struct ClassObject*) _dvmFindLoadedClass(&d, clsnew);
	if(!res){
		log("XXX7 chiamo loadsystemclass con %s, str = %p\n", clsname, str)
		//res = _loadSystemClass(env, str);
		if(!res){
			;//res = (*env)->FindClass(env,clsname);
		}
	}
	else{
		//dalvik_dump_class(&d,"Ljavax/crypto/spec/PBEKeySpec;");
		log("XXX7 dumpjavaclass res = 0x%x, classloader = 0x%x\n", res, res->classLoader)
		log(" dvmdex = 0x%x\n",  res->pDvmDex)
		log("XXX7 cerco %s\n", clsnew)
		dalvik_dump_class(&d, clsnew);
		//_dvmFindStaticField(res, "z", "[Ljava/lang/String;");

		//res = (*env)->FindClass(env, "com/whatsapp/Voip");
		/**
		void* gres = (*env)->NewGlobalRef(env, res);
		 jfieldID v =  (*env)->GetStaticFieldID(env, gres, "z", "[Ljava/lang/String;");
		jobject vobj = (*env)->GetStaticObjectField(env, gres, v);
		 log("XXX7 preso static field\n" )
		 jmethodID mid = (*env)->GetStaticMethodID(env, gres, "<clinit>", "()V");
		 (*env)->CallStaticObjectMethod(env, gres, mid);
		log(" XXX7 chiamato clinit\n")
		*/
	}
	//diosolo(&d, env);
	free(clsnew);
	(*env)->ReleaseStringUTFChars(env, str, clsname); 
	// /startMagic(env);
}
JNINativeMethod method_table[] = {
    { "createStruct", "(Lorg/sid/addi/core/DalvikHook;)V",
        (void*) _createStruct },
    {"suspendALL","()V",
		(void*)_wrapperSuspendAll},
    {"resumeALL","()V",
		(void*)_wrapperResumeAll},
	{"unhook","(Ljava/lang/String;)V",
		(void*)_unhook},
	{"defineClass","(ILjava/lang/String;)I",
		(void*)_wrapper_dexstuff_defineclass},
	{"loadDex", "(Ljava/lang/String;)I",
		(void*)_wrapper_dexstuff_loaddex},
	{"startNewThread", "(Ljava/lang/String;Ljava/lang/String;)V",
		(void*)_newThread},
	{"dumpJavaClass", "(Ljava/lang/String;Ljava/lang/String;)V",
		(void*)_dumpJavaClass},
	{ NULL, NULL, NULL },
};

jint my_ddi_init(){
	pthread_mutex_init(&mutex, NULL);
	dexstuff_resolv_dvm(&d);
	if(debug)
		log("-------------------DENTRO  DDI INIT ---------------------\n")
	cookie = dexstuff_loaddex(&d, "/data/local/tmp/classes.dex");
	//devo definire tutte le classi di controllo usate dagli hook
	
	
	jclass *clazz55 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/StringHelper", cookie);
	jclass *clazz44 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/LogWrapper", cookie);	
	jclass *clazz22 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/DalvikHook", cookie);
	jclass *clazzDEXHook =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/DEXHook", cookie);
	jclass *clazz59 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/Logs/LoggerConfig", cookie);
	jclass *clazz58 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/Logs/LogTraces", cookie);
	jclass *clazz99 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/Logs/LogToSQLiteHelper", cookie);
	jclass *clazz88 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/Logs/LogToSQLiteClass", cookie);
	jclass *clazz77 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/Logs/LogToSQLite", cookie);
	jclass *clazz57 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/Logs/LogToDB", cookie);
	jclass *clazz56 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/Logs/Logger", cookie);
	jclass *clazz60 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/Common", cookie);
	jclass *clazz61 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/CommandWrapper", cookie);
	jclass *clazz62 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/ArgumentWrapper", cookie);
	jclass *clazz63 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/RequestWrapper", cookie);
	jclass *clazz64 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/Commands", cookie);
	jclass *clazz65 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/Session", cookie);
	jclass *clazz66 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/XML", cookie);
	jclass *clazz5 = (jclass*) dexstuff_defineclass(&d, "org/sid/addi/utils/AppContextConfig", cookie);
	jclass *clazz33 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/manageADDI", cookie);

	// Define user hook
	jclass* networkHook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/NetworkHook", cookie);
	jclass* IPCHook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/IPCHooks", cookie);
	jclass* appC =(jclass*) dexstuff_defineclass(&d, "org/tesi/utils/AppContextConfig", cookie);
	jclass* webViewHook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/WebviewHook", cookie);
	jclass* sqliteHook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/SQLiteHook", cookie);
	jclass* sslhook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/SSLHook", cookie);
	jclass* fshook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/FileSystemHook", cookie);
	jclass* sphook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/SharedPrefHook", cookie);
	jclass* cryptohook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/CryptoHook", cookie);
	jclass* hashhook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/HashHook", cookie);
	jclass* cryptokey =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/HookCryptoKey", cookie);

	jclass *clazz7 =(jclass*) dexstuff_defineclass(&d, "org/tesi/core/DalvikHookImpl", cookie);
	jclass *clazz3 =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/HookList", cookie);
	jclass *clazz4 = (jclass*)dexstuff_defineclass(&d, "org/tesi/core/MyInit", cookie);
	//jclass *clazz5 = (jclass*) dexstuff_defineclass(&d, "org/tesi/utils/AppContextConfig", cookie);
	jclass *clazz6 = (jclass*) dexstuff_defineclass(&d, "org/tesi/utils/sendSMS", cookie);

	
	JNIEnv *env = (JNIEnv*) get_jni_for_thread(&d);
	log("ddi XXX6 init preso env = 0x%x\n", env)

	(*env)->RegisterNatives(env, (*env)->FindClass(env, "org/sid/addi/core/manageADDI"), method_table, sizeof(method_table) / sizeof(method_table[0]));
	if(env){
		handleAllMethodClass(&d, env);
		//dalvik_dump_class(&d, "");
		//_initMemberOffset(env, &d,  getSelf(&d));

		void* c = d.dvmFindLoadedClass_fnPtr("Ljava/net/Socket;");
		log("DIOCANE TROVAATO = %x\n", c)
		dalvik_dump_class(&d,"Ljava/net/Socket;");

		jclass mycls = (*env)->FindClass(env, "org/tesi/core/MyInit");
		log("XXX4 trovata classe myinit = 0x%x\n", mycls)
		jmethodID constructor = (*env)->GetMethodID(env, mycls, "<init>", "()V");
		jmethodID place_hook = (*env)->GetMethodID(env, mycls, "place_hook", "()V");
		jobject obj = (*env)->NewObject(env, mycls, constructor); 
		log("XXX4 creato oggetto della classe MyInit = 0x%x, mid = 0x%x\n", obj, place_hook)
		(*env)->CallVoidMethod(env,obj, place_hook);
	} 
	log("SIZEOF STRUCT = %d \n" , sizeof(L))
	if(debug)
		log("------------------- FINE LOAD DEX INIT ---------------------\n")
	return JNI_OK;
}

void _unhook(JNIEnv *env, jobject thiz, jobject str)
{
	log("XXX5 ------------ UNHOOK SONO STATO CHIAMATO -------- \n")

	char *c = (*env)->GetStringUTFChars(env, str, 0);
	log("XXX5 RICEVUTO = %s\n", c)
	struct dalvik_hook_t *res;
	pthread_mutex_lock(&mutex);
	res = (struct dalvik_hook_t *) cerca(L, c);
	pthread_mutex_unlock(&mutex);
	if(!res){
		log("XXX5 errore RES = 0\n")
		return;
	}
	log("XXX5 trovato hook %s at 0x%x\n", res->clname, res)
	log("XXX5 trovato hook %s\n", res->method_name)
	pthread_mutex_lock(&(res->mutexh));
	dalvik_prepare(&d,res,env);
	res->ok=0;
	pthread_mutex_unlock(&(res->mutexh));
	(*env)->DeleteLocalRef(env, str);
	log("XXX5 FINITO UNHOOK ------------------- \n")    
}

void* setJavaVM(JavaVM* ajvm){
	g_JavaVM = ajvm;
}

void* callOrigin(JNIEnv *env, jobject obj, va_list l, struct dalvik_hook_t *res, char* s ){
	jobject str = NULL;
	int j= 0;
	int flag =0;
	char *pointer = s;
	j = is_string_class(pointer);
	int real_args = res->n_iss;//_dvmComputeMethodArgsSize(&d,res->method);
	if(!res->sm){
		real_args--;
	}
	if(j)
		flag = 1;
	//qui posso usare lo shorty
	JValue result;
	switch(*s){
		//method return a class
		case 'L':
			
			if(real_args == 0){
					//method have zero arguments
				if(res->sm){	
					//str = (*env)->CallStaticObjectMethod(env, obj, res->mid);		
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					
					va_list myargs;
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, myargs);
					result.l = (struct Object*) (*env)->NewLocalRef(env, (jobject)result.l);
					log("return a class2: @0x%x\n", result.l)
					return result.l;
				}
				else{
					//str = (*env)->CallObjectMethod(env, obj, res->mid);
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					
					va_list myargs;
					log("chiamo invokemethod con original = %p, obj = %p, result = %p, myargs = %p \n", res->originalMethod, obj, &result, myargs)
					asm volatile("" ::: "memory");
					struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
					log("decodificato = %p\n", nobj)
					log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, myargs);
					result.l = (struct Object*) (*env)->NewLocalRef(env, (jobject)result.l);
					log("return a class2: @0x%x\n", result.l)
					return result.l;
				}
			}
			else{
					//method have args
				if(res->sm){
					if(debug)
						log("call static object method v : %s\n", res->method_name)
					//str = (*env)->CallStaticObjectMethodV(env, obj, res->mid, l);
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, l);
					if(debug)
						log("success calling static: %s\n", res->method_name)
					result.l = (struct Object*) (*env)->NewLocalRef(env, (jobject)result.l);
					log("return a class2: @0x%x\n", result.l)
					return result.l;
				}
				else{
					if(debug)
						log("call object method v : %s mid = 0x%x\n", res->method_name, res->mid)
									
					log("chiamo invokemethod con original = %p, obj = %p, result = %p \n", res->originalMethod, obj, &result )
					asm volatile("" ::: "memory");
					struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
					log("decodificato = %p\n", nobj)
					log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
					//void* gresult = (*env)->NewGlobalRef(env,(jobject)&result);
					jobject* diocane;// = (jobject*) calloc(1, sizeof(jobject));
					struct Object* aa = (struct Object*)(*env)->NewGlobalRef(env,diocane);
					log("global ref = %p\n", aa)
					dump_jni_ref_tables(&d);
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, l);
					return;
					//str = (*env)->CallObjectMethodV(env, obj, res->mid, l);
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					//JValue res;
					//res.l = (JValue) gresult;
					//if(debug)
					//		log("success calling : %s, result name = %s, clazz = %p\n", res->method_name, gresult.l->clazz->descriptor, gresult.l->clazz)
					//result.l = (struct Object*) (*env)->NewGlobalRef(env,result.l);
					//log("return a class2: @0x%x\n", result.l)
					//log("chiamo boxprimitive con result = %p e classobject = %p, primittype = %d\n", result.l, res->originalMethod->clazz, res->originalMethod->clazz->primitiveType)
					//void* aa = d.dvmBoxPrimitive_fnPtr(result,res->originalMethod->clazz);
					
					//memcpy(aa,result.l,sizeof(struct Object*));					
					//log("ritorno res= %p, nobj2 = %p\n", result.l, nobj2)
					//return aa;
				}
			}
			if(flag){
				if(str != NULL){

					char *s = (*env)->GetStringUTFChars(env, result.l, 0);
					if (s != NULL) {
						if(debug)
							log("OTTENUTA STRINGA = %s\n", s)
						(*env)->ReleaseStringUTFChars(env, result.l, s); 
					}
					if(debug)
						log("return FLAG a class: @0x%x\n", str)
					if(debug)
						log("----------------------------------------------------\n")
					return result.l;
				}
				else{
					log("XXX errore str NULL\n")
					if(debug)
						log("----------------------------------------------------\n")
					return result.l;
				}
			}
			else{
				if(debug)
					log("return a class1: @0x%x\n", result.l)
				log("----------------------------------------------------\n")
				result.l = (struct Object*) (*env)->NewLocalRef(env, (jobject)result.l);
				log("return a class2: @0x%x\n", result.l)
				return;
			}
		case 'V':
			if(res->skip){
				if(debug)
					log("VOID :::: SKIPPO IL METODO!!!\n")
				log("----------------------------------------------------\n")
				return (jint)1;
			}
			else{
				if(debug)
					log("VOID obj = 0x%x, mid = 0x%x, l = 0x%x,  realargs = %d, mname = %s\n", obj, res->mid,l, real_args, res->method_name)

				if(strstr(res->method_name, "<init>") != NULL){
					if(debug)
						log("strtstr(%s) STO CHIAMANDO UN COSTRUTTORE\n", res->method_name)					
				}
				if(real_args == 0){
					if(res->sm){
						log("CALLORIGIN STATIC V PROVA PROVA original  = %p, obj = %p\n",res->originalMethod, obj)
						
						
						va_list myargs;
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, myargs);
						//(*env)->CallStaticVoidMethod(env, obj, res->originalMethod);					
						//(*env)->CallStaticVoidMethod(env, obj, res->mid);					

					}
					else{
						if(debug)
							log("chiamo voidmethod\n")
						//(*env)->CallVoidMethod(env, obj, res->mid);
						log("CALLORIGIN V PROVA PROVA %p, name = %s\n",res->originalMethod, res->originalMethod->name)
						log("ORIGINAL insns = %p , nativeFunc = %p, inssize = %d, registersize = %d, jniarginfo = %p, access_flags=%p \n", res->originalMethod->insns, res->originalMethod->nativeFunc, res->originalMethod->insSize,
																	res->originalMethod->registersSize, res->originalMethod->jniArgInfo, res->originalMethod->a)
						log("hooked insns = %p, hooked nativeFunc = %p, inssize=%d, regsize = %d, jniarginfo = %p\n", res->method->insns, res->method->nativeFunc,
																	res->method->insSize, res->method->registersSize, res->method->jniArgInfo)
						log("OLD hooked insns = %p, hooked nativeFunc = %p, inssize=%d, regsize = %d, access_flags = %p\n", res->insns, res->native_func,
											res->iss, res->rss, res->access_flags)
						//_dvmInvokeMethod(&d,thisObject, res->originalMethod, argList, params, returnType, true);
						//(*env)->CallVoidMethod(env, obj, res->originalMethod);
						
						va_list myargs;
						log("chiamo invokemethod con original = %p, obj = %p, result = %p, myargs = %p \n", res->originalMethod, obj, &result, myargs)
						asm volatile("" ::: "memory");
						struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
						log("decodificato = %p\n", nobj)
						log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
						//struct Object* nobj = _dvmDecodeIndirectRef(getSelf(&d), obj); 
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, myargs);
						//_dvmInvokeMethod(&d,obj,res->originalMethod, NULL, NULL, NULL, true);
						/*
						res->method->insSize = res->iss;
						res->method->registersSize = res->rss;
						res->method->outsSize = res->oss;
						res->method->a = res->access_flags;
						res->method->jniArgInfo = 0;
						//res->method->insns = res->insns; 
						*/
						//(*env)->CallVoidMethod(env, obj, (jmethodID) res->originalMethod);
					}
				}
				else{
					if(res->sm){
						log("CALLORIGIN STATIC V PROVA PROVA original =  %p, obj = %p, valist = %p\n",res->originalMethod, obj, l)
						//(*env)->CallStaticVoidMethodV(env, obj, res->originalMethod ,l);
						JValue result;
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, l);
					}
					else{
						if(debug)
							log("chiamo voidmethodV\n")
						//(*env)->CallVoidMethodV(env, obj, res->mid, l);
						log("CALLORIGIN NON STATIC PROVA PROVA original = %p, obj = %p, l =%p\n",res->originalMethod, obj, l)
						

						log("chiamo invokemethod con original = %p, obj = %p, result = %p,  \n", res->originalMethod, obj, &result)
						asm volatile("" ::: "memory");
						struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
						log("decodificato = %p\n", nobj)
						log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
						//struct Object* nobj = _dvmDecodeIndirectRef(getSelf(&d), obj); 
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, l);
						//(*env)->CallVoidMethodV(env, obj, (jmethodID) res->originalMethod, l);
					}
					if(debug)
						log("Chiamato metodo : %s\n", res->method_name)
					log("----------------------------------------------------\n")	
				}
			}
			break;		
		case 'Z':
			log("!!BOOOLEAN___\n") 
			JValue jResult;
			if(res->skip){
				log("!!!!SKIPPO\n")
				if(debug)
					log("----------------------------------------------------\n")
				if(res->skip == 1)
					return (jboolean)1;				
				else return (jboolean)0;
			}
			else{
				if(real_args == 0){	
					if(res->sm){

						//log("CALLORIGIN STATIC Z PROVA PROVA %p, name %s \n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod, res->pOriginalMethodInfo->originalMethodStruct.originalMethod.name)
						//str = (*env)->CallStaticBooleanMethod(env, obj, (jmethodID) &res->pOriginalMethodInfo->originalMethodStruct.originalMethod);
						
						va_list myargs;
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, myargs);
						return result.z;
						if(debug)
							log("----------------------------------------------------\n")					
					}
					else{
						//str = (*env)->CallBooleanMethod(env,obj,res->mid);	
						//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
						
						va_list myargs;
						struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
						log("decodificato = %p\n", nobj)
						log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, myargs);
						return result.z;
					}
					if(debug)
						log("----------------------------------------------------\n")
				}
				else{
					if(res->sm){
						JValue result;
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, l);
						return result.z;
						//log("CALLORIGIN STATIC Z PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)		
						}
					else{
						//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)		
						//str = (*env)->CallBooleanMethodV(env,obj,res->mid,l);
						
						va_list myargs;
						struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
						log("decodificato = %p\n", nobj)
						log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
						d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, l);
						return result.z;
					}
					if(debug)
						log("----------------------------------------------------\n")
				}
			}
			return (jboolean)str;
		case 'I':
			if(debug){
				log("XXX2 DENTRO INT METHOD\n")
					//printStackTrace(env);
			}
			if(res->skip){
				if(debug)
					log("XXX2 FINE METODO RITORNO 0!!!!\n")
				if(res->skip == 1)
					return (jint)1;
				else
					return (jint)0;	//to skip checkpermission
			}
			if(real_args == 0){
				log("CALLORIGIN CHIAMO INTMETHOD con obj = %p, res->mid = %p, clname = %s , mthname = %s\n", obj, res->mid, res->clname, res->method_name)
				if(res->sm){
					//str = (*env)->CallStaticIntMethod(env, obj, res->mid);
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)

					
					va_list myargs;
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, myargs);
					return result.i;
				}
				else{
					log("ORIGINCALL CHIAMO N NON STATIC ")
					//str = (*env)->CallIntMethod(env, obj, res->mid);
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					//str = (*env)->CallIntMethod(env, obj, (jmethodID) &res->pOriginalMethodInfo->originalMethodStruct.originalMethod);
					log(" CHIAMATO, ret = %p \n",str)
					
					va_list myargs;
					struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
					log("decodificato = %p\n", nobj)
					log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
					//struct Object* nobj = _dvmDecodeIndirectRef(getSelf(&d), obj); 
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, myargs);
					return result.i;
				}
				dalvik_postcall(&d, res);
				if(debug)
					log("----------------------------------------------------\n")
			}
			else{
				log("CALLORIGIN CHIAMO INT METHODV con obj = %p, res->mid = %p, clname = %s , mthname = %s\n", obj, res->mid, res->clname, res->method_name)
				if(res->sm){					
					//str = (*env)->CallStaticIntMethodV(env, obj, (jmethodID) &res->pOriginalMethodInfo->originalMethodStruct.originalMethod,l);
					//str = (*env)->CallStaticIntMethod(env, obj, res->mid,l);
					
					va_list myargs;
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, l);
					return result.i;
					//str = (*env)->CallStaticIntMethodV(env, obj, res->originalMethod,l);
				}
				else{
					//str = (*env)->CallIntMethodV(env, obj, res->mid,l);
					log("CALLORIGIN PROVA PROVA %p\n",res->originalMethod)
					
					struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
					log("decodificato = %p\n", nobj)
					log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
					//struct Object* nobj = _dvmDecodeIndirectRef(getSelf(&d), obj); 
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, l);
					return result.i;
					//str = (*env)->CallIntMethodV(env, obj, res->originalMethod,l);
				}			
				if(debug)
					log("----------------------------------------------------\n")
			}
			return (jint)str;
		default:
			if(debug)
				log("XXX DEFAULT \n")
			JValue result;
			if(real_args == 0){
				if(debug)
					log("XXX default senza argomenti\n")
				if(res->sm){
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					//str = (*env)->CallStaticIntMethod(env, obj, res->mid);
					va_list myargs;
					
					//struct Object* nobj = _dvmDecodeIndirectRef(getSelf(&d), obj); 
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,NULL,true,&result, myargs);
					
				}
				else{
					//result =(*env)->CallObjectMethod(env, obj, res->mid);
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					va_list myargs;
					
					struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
					log("decodificato = %p\n", nobj)
					log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
					//struct Object* nobj = _dvmDecodeIndirectRef(getSelf(&d), obj); 
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, myargs);
					
				}
				if(debug)
					log("XXX chiamato metodo: %s\n", res->method_name)
			}
			else{
				if(debug)
					log("XXX default con argomenti\n")
				if(res->sm){
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					
					//str = (*env)->CallStaticIntMethod(env, obj, res->mid);
				}
				else{
					//result =(*env)->CallObjectMethodV(env, obj, res->mid, l);
					//log("CALLORIGIN PROVA PROVA %p\n",res->pOriginalMethodInfo->originalMethodStruct.originalMethod)
					
					struct Object* nobj = d.dvmDecodeIndirectRef_fnPtr(getSelf(&d), obj); //_dvmDecodeIndirectRef(getSelf(&d), thiz);
					log("decodificato = %p\n", nobj)
					log("accedo decodificato, name = %s, classLoader = %p\n", nobj->clazz->descriptor, nobj->clazz->classLoader)
					//struct Object* nobj = _dvmDecodeIndirectRef(getSelf(&d), obj); 
					d.dvmCallMethodV_fnPtr(getSelf(&d),res->originalMethod,nobj,true,&result, l);
					
				}
				if(debug)
					log("XXX chiamato metodo: %s\n", res->method_name)
			}
			if(debug)
				log("----------------------------------------------------\n")
			
			return result.l;			
		}
		return -1;
}
