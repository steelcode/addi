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
#include "dexstuff.h"
#include "dalvik_hook.h"
#include "lista.h"
#include "Misc.h"
#include "log.h"


#define APICLASS "Lorg/sid"
#define HOOKCLASS "Lorg/tesi"
#define DALVIKHOOKCLS "org/sid/addi/core/DalvikHook"


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

int dalvik_hook_setup(struct dalvik_hook_t *h, char *cls, char *meth, char *sig, int ns, void *func)
{
	if (!h)
		return 0;

	pthread_mutex_init(&h->mutexh, NULL);
	strcpy(h->clname, cls);
	
	strncpy(h->clnamep, cls+1, strlen(cls)-2);
	h->clnamep[strlen(cls)-2] = '\0';

	strcpy(h->method_name, meth);
	strcpy(h->method_sig, sig);
	h->n_iss = ns;
	h->n_rss = ns;
	h->n_oss = 0;
	h->native_func = func;

	h->sm = 0; // set by hand if needed

	h->af = 0x0100; // native, modify by hand if needed
	
	h->resolvm = 0; // don't resolve method on-the-fly, change by hand if needed

	h->debug_me = 0;
	h->dump = 0;
	return 1;
}


int dalvik_hook(struct dexstuff_t *dex, struct dalvik_hook_t *h)
{
	log("DENTRO DALVIK HOOK MAIN\n")
	if (h->debug_me)
		log("dalvik_hook: class %s\n", h->clname)
	void *target_cls = dex->dvmFindLoadedClass_fnPtr(h->clname);
	if (h->debug_me)
		log("class = 0x%x\n", target_cls)

	if (!target_cls) {
		if (h->debug_me)
			log("target_cls == 0\n")
		log("XXX5  ESCO DA DALVIKHOOK CON 0\n")
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
		if(h->debug_me)
			log("calling dvmFindDirectMethodByDescriptor_fnPtr\n");
		h->method = dex->dvmFindDirectMethodByDescriptor_fnPtr(target_cls, h->method_name, h->method_sig);
	}
	// constrcutor workaround, see "dalvik_prepare" below
	if (!h->resolvm) {
		h->cls = target_cls;
		h->mid = (void*)h->method;
	}
	log("XXXD: h->method vale: 0x%x, 0x%08x\n", h->method, h->mid)
	if (h->debug_me)
		log("%s(%s) = 0x%x\n", h->method_name, h->method_sig, h->method)

	if (h->method) {
		h->sm = is_static(dex, h->method);
		h->insns = h->method->insns;

		if (h->debug_me) {
			log("DALVIK_HOOK nativeFunc 0x%x, dalvin_hook_t: 0x%x\n", h->method->nativeFunc, h->native_func)		
			log("DALVIK_HOOK insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->method->insSize, h->method->registersSize, h->method->outsSize)
		}
		h->iss = h->method->insSize;
		h->rss = h->method->registersSize;
		h->oss = h->method->outsSize;
	
		h->method->insSize = h->n_iss;
		h->method->registersSize = h->n_rss;
		h->method->outsSize = h->n_oss;

		if (h->debug_me) {
			log("shorty %s\n", h->method->shorty)
			log("name %s\n", h->method->name)
			log("arginfo %x\n", h->method->jniArgInfo)
		}
		h->method->jniArgInfo = 0x80000000; // <--- also important

		h->access_flags = h->method->a;
		h->method->a = h->method->a | h->af; // make method native
		if (h->debug_me)
			log("access %x\n", h->method->a)
	
		dex->dvmUseJNIBridge_fnPtr(h->method, h->native_func);		
		if (h->debug_me)
			log("patched %s to: 0x%x\n", h->method_name, h->native_func)
		if (h->debug_me) {
			log("DALVIK_HOOK nativeFunc 0x%x, dalvin_hook_t: 0x%x\n", h->method->nativeFunc, h->native_func)		
			log("DALVIK_HOOK insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->method->insSize, h->method->registersSize, h->method->outsSize)
		}
		log("XXX5  ESCO DA DALVIKHOOK CON 1\n")
		return 1;
	}
	else {
		if (h->debug_me)
			log("could NOT patch %s\n", h->method_name)
	}
	log("XXX5  ESCO DA DALVIKHOOK CON 0\n")
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
	h->method->insSize = h->iss;
	h->method->registersSize = h->rss;
	h->method->outsSize = h->oss;
	h->method->a = h->access_flags;
	h->method->jniArgInfo = 0;
	h->method->insns = h->insns; 
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
	dex->dvmUseJNIBridge_fnPtr(h->method, h->native_func);
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

//sid 
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
	if(res->real_args == 0){
		if(debug)
			log("XXX5 chiamo il metodo DEX 0 args con clazz = 0x%x, gclazz = 0x%x\n",  clazz, res->DexHookCls)
		log("XXX5  prima  di getmethod\n")
		jmethodID initid = (*env)->GetMethodID(env, res->DexHookCls, "initFunc", "()V");
		log("XXX5 prima di call initid\n")
		(*env)->CallVoidMethod(env,res->ref, initid);
		dex_meth = (*env)->GetMethodID(env, res->DexHookCls, "myexecute", "()V");
		(*env)->CallVoidMethod(env,res->ref, dex_meth, NULL);	
		if(debug)
			log("XXX5 method %s chiamato\n", res->method_name)
	}
	else{	
		log("XXX5 il metodo ha argomenti!\n")
		jobjectArray ja;
		jbooleanArray jba;
		jboolean jbargs[res->real_args];
		int jaboolean = 0;
		
		ja = (*env)->NewObjectArray(env, res->real_args, (*env)->FindClass(env, "java/lang/Object"), NULL);
		jba = (*env)->NewBooleanArray(env, res->real_args);

		jobject gja = (*env)->NewGlobalRef(env, ja);
		
		jobject jbobj;
		char *pshorty  = res->method->shorty;
		pshorty++; //[0] is return  type
		int i = 0;

		if(debug)
			log("XXX5 chiamo il metodo DEX con ja = 0x%x, clazz = 0x%x, gclazz = 0x%x con pshorty = %s\n", gja, clazz, res->DexHookCls, pshorty)
		
		//problemi quando costruisco arg di tipo nativo jboolean, ecc..
		for(i=0;i<res->real_args;i++){
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
	char * cl = (char*)malloc(sizeof(char)*256);
	char * name = (char*)malloc(sizeof(char)*256);
	char * descriptor = (char*)malloc(sizeof(char)*256);
	char * hash = (char*)malloc(sizeof(char)*512);

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
	dalvik_prepare(&d, res, env);
	jfloat ret;
	if(res->real_args == 0){
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
	char * cl = (char*)malloc(sizeof(char)*256);
	char * name = (char*)malloc(sizeof(char)*256);
	char * descriptor = (char*)malloc(sizeof(char)*256);
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
	dalvik_prepare(&d, res, env);
	jlong ret;
	if(res->real_args == 0){
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
	jobject str = 0;
	int j= 0;
	int flag =0;
	char *pointer = s;
	j = is_string_class(pointer);
	if(j)
		flag = 1;
	//qui posso usare lo shorty
	switch(*s){
		//method return a class
		case 'L':
		if(res->real_args == 0){
				//method have zero arguments
			if(res->sm){	
				str = (*env)->CallStaticObjectMethod(env, obj, res->mid);		
			}
			else{
				str = (*env)->CallObjectMethod(env, obj, res->mid);
			}
		}
		else{
				//method have args
			if(res->sm){
				if(debug)
					log("call static object method v : %s\n", res->method_name)

				str = (*env)->CallStaticObjectMethodV(env, obj, res->mid, l);

				if(debug)
					log("success calling static: %s\n", res->method_name)
			}
			else{
				if(debug)
					log("call object method v : %s mid = 0x%x\n", res->method_name, res->mid)

				str = (*env)->CallObjectMethodV(env, obj, res->mid, l);

				if(debug)
					log("success calling : %s\n", res->method_name)
			}
		}
		if(flag){
			if(str != NULL){

				char *s = (*env)->GetStringUTFChars(env, str, 0);
				if (s != NULL) {
					if(debug)
						log("OTTENUTA STRINGA = %s\n", s)
					(*env)->ReleaseStringUTFChars(env, str, s); 
				}
				if(debug)
					log("return a class: @0x%x\n", str)
				if(debug)
					log("----------------------------------------------------\n")
				return str;
			}
			else{
				log("XXX errore str NULL\n")
				if(debug)
					log("----------------------------------------------------\n")
				return str;
			}
		}
		else{
			if(debug)
				log("return a class: @0x%x\n", str)
			log("----------------------------------------------------\n")
			return str;
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
				log("VOID obj = 0x%x, mid = 0x%x, l = 0x%x,  realargs = %d, mname = %s\n", obj, res->mid,l, res->real_args, res->method_name)

			if(strstr(res->method_name, "<init>") != NULL){
				if(debug)
					log("strtstr(%s) STO CHIAMANDO UN COSTRUTTORE\n", res->method_name)					
			}
			if(res->real_args == 0){
				if(debug)
					log("chiamo voidmethod\n")
				(*env)->CallVoidMethod(env, obj, res->mid);
			}
			else{
				if(debug)
					log("chiamo voidmethodV\n")
				(*env)->CallVoidMethodV(env, obj, res->mid, l);
				if(debug)
					log("Chiamato metodo : %s\n", res->method_name)
			}
		}
		break;		
		case 'Z':
		log("!!BOOOLEAN___\n") 
		if(res->skip){
			log("!!!!SKIPPO\n")
			if(debug)
				log("----------------------------------------------------\n")
			if(res->skip == 1)
				return (jboolean)1;				
			else return (jboolean)0;
		}
		else{
			if(res->real_args == 0){	
				str = (*env)->CallBooleanMethod(env,obj,res->mid);	
				if(debug)
					log("----------------------------------------------------\n")
			}
			else{				
				str = (*env)->CallBooleanMethodV(env,obj,res->mid,l);
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
			if(res->real_args == 0){
				
				str = (*env)->CallIntMethod(env, obj, res->mid);
				
				dalvik_postcall(&d, res);
				if(debug)
					log("----------------------------------------------------\n")
			}
			else{
				
				str = (*env)->CallIntMethodV(env, obj, res->mid,l);
				
				if(debug)
					log("----------------------------------------------------\n")
			}
			return (jint)str;
			default:
			if(debug)
				log("XXX DEFAULT \n")
			void * result;
			if(res->real_args == 0){
				if(debug)
					log("XXX default senza argomenti\n")
				
				result =(*env)->CallObjectMethod(env, obj, res->mid);
				if(debug)
					log("XXX chiamato metodo: %s\n", res->method_name)
			}
			else{
				if(debug)
					log("XXX default con argomenti\n")
				
				result =(*env)->CallObjectMethodV(env, obj, res->mid, l);
				if(debug)
					log("XXX chiamato metodo: %s\n", res->method_name)
			}
			if(debug)
				log("----------------------------------------------------\n")
			
			return result;			
		}
		return -1;
}
void*  callOrigin2(JNIEnv  *env, struct dalvik_hook_t* res, va_list l, void* obj){
	log("XXX5 dentro CALL ORIGIN 2 obj = 0x%x\n", obj)
    JValue result;
    jobject gthiz = (*env)->NewGlobalRef(env, obj);
    if(!is_static(&d, res->method)){
    	//thiz = args[0];
    	//thiz = (struct Object*) va_arg(l, struct Object);
    	log("XXX5  metodo: %s non statico\n", res->method_name)
    }
    log("XXX5 chiamo callmethodv \n")
    _dvmCallMethodV(&d, getSelf(&d), res->method, (struct Object*)obj, false, &result, l);
    log("XXX5 finito callmethod v: 0x%x\n", result.l)

    return (jobject) result.l;
    //ClassObject* returnType = dvmGetBoxedReturnType(res->method);

}
void * hookAbilitato(JNIEnv *env, void* obj, va_list l, struct dalvik_hook_t *res, va_list lhook, char* descriptor,char  *s){
	void* ret = NULL;
	int flag = 0;
	if(debug)
		log("!!!!PRESO MUTEX %s con signature = %s, res = @0x%x, il metodo: @0x%x con mid = 0x%x ha: %d argomenti, skippable = %d, type = %c\n", res->method_name, res->method_sig, res, res->method, res->mid,res->real_args, res->skip, *s)
	
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
	dalvik_postcall(&d,res);
	log("XXX7 HOOK ABILITATO ESCO\n")
	return ret;
}
void* hookDisabilitato(JNIEnv *env, jobject obj, va_list l, char*s, struct dalvik_hook_t* res){
	void* ret = NULL;
	if(debug)
		log("!!!!PRESO MUTEX %s con signature = %s, res = @0x%x, il metodo: @0x%x con mid = 0x%x ha: %d argomenti, skippable = %d, type = %c\n", res->method_name, res->method_sig, res, res->method, res->mid,res->real_args, res->skip, *s)
	ret = callOrigin(env, obj, l,res,s);
	return ret;
}

void* onetoall(JNIEnv *env, jobject obj, ...){
	//printStackTrace(env);
	log("--------------------- ONETOALL------------obj vale = 0x%x -------------------\n", obj)
	
	struct ClassObject* co = _getCallerClass(&d);
	//log("XXX6 GETCALLERCLASS = 0x%x, name = %s \n", co, co->descriptor)
	struct ClassObject* co2 = _getCaller2Class(&d);
	//log("XXX6 GETCALLERCLASS2 = 0x%x \n", co2);
	//log("XXX6 GETCALLERCLASS2 = 0x%x, name = %s \n", co2, co2->descriptor)
	struct ClassObject* co3 = _getCaller3Class(&d);
	//log("XXX6 GETCALLERCLASS3 = 0x%x, name = %s \n", co3, co3->descriptor)
	
	char * cl = (char*)malloc(sizeof(char)*256);
	char * name = (char*)malloc(sizeof(char)*256);
	char * descriptor = (char*)malloc(sizeof(char)*256);
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

	//only for info
	get_caller_class(env,obj,cl);
	//metodo chiamante
	get_method(&d,name,descriptor);

	strcpy(hash,name);
	strcat(hash,descriptor);
	if(debug)
		log("CERCO NELLA LISTA : %s\n", hash)

	pthread_mutex_lock(&mutex);
	res = (struct dalvik_hook_t *) cerca(L, hash);
	pthread_mutex_lock(&(res->mutexh));
	pthread_mutex_unlock(&mutex);
	if(!res){
		log("XXX errore RES = 0\n")
	}
	if(is_static(&d, res->method)){
		log("XXX6 METHOD: %s e statico!!\n", res->method_name)
	}
	else{
		log("XXX6 METHOD: %s NON statico!!\n", res->method_name)
	}
	memset(cl,0,sizeof(cl));
	strcpy(cl,res->method_sig);
	char *s = parse_signature(cl);
	dalvik_prepare(&d, res, env);
	//TODO: check against null pointer!!
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
			dalvik_postcall(&d,res);
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
	pthread_mutex_unlock(&(res->mutexh));
	if(ret != -1){
		return ret;
	}
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
	jmethodID argsid = (*env)->GetMethodID(env,dalvikCls,"get_myargs","()I");
	jmethodID nsid = (*env)->GetMethodID(env,dalvikCls,"getNs","()I");
	jmethodID sid = (*env)->GetMethodID(env,dalvikCls,"isSkip","()I");

	void * c = (*env)->CallObjectMethod(env,clazz,clname);
	void * c2 = (*env)->CallObjectMethod(env,clazz,method_name);
	void* c3 = (*env)->CallObjectMethod(env,clazz,method_sig);
	void* c4 = (*env)->CallObjectMethod(env,clazz,dexmethod);
	void* c6 = (*env)->CallObjectMethod(env,clazz,dexclass);
	void *c5 = (*env)->CallObjectMethod(env,clazz,hashid);
	jint args = (*env)->CallIntMethod(env,clazz,argsid);
	jint ns = (*env)->CallIntMethod(env,clazz,nsid);
	jint flag = (*env)->CallIntMethod(env,clazz,sid);
	
	char *s = (*env)->GetStringUTFChars(env, c , NULL);
	char *s2 = (*env)->GetStringUTFChars(env, c2 , NULL);
	char *s3 = (*env)->GetStringUTFChars(env, c3 , NULL);
	char *s4 = (*env)->GetStringUTFChars(env, c4 , NULL);
	char *s5 = (*env)->GetStringUTFChars(env,c5,NULL); //hashvalue
	char *s6 = (*env)->GetStringUTFChars(env, c6 , NULL);
	struct dalvik_hook_t *dh;
	dh = (struct dalvik_hook_t *)malloc(sizeof(struct dalvik_hook_t));
	dh->real_args = args;
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
	if(*pointer == 'F'){
		//hook a method that return jfloat
		if(!dalvik_hook_setup(dh, s,s2,s3,ns,jfloat_wrapper))
			log("----------- HOOK SETUP FALLITO ------------- : %s, %s\n", c,c2)
		if(!dalvik_hook(&d,dh))
			log("----------- HOOK FALLITO ------------- : %s, %s\n", c,c2)
		log("HOOK PLACED\n")		
	}
	else if(*pointer == 'J'){
		if(!dalvik_hook_setup(dh, s,s2,s3,ns,jlong_wrapper))
			log("----------- HOOK SETUP FALLITO ------------- : %s, %s\n", c,c2)
		if(!dalvik_hook(&d,dh))
			log("----------- HOOK FALLITO ------------- : %s, %s\n", c,c2)
		log("HOOK PLACED\n")
	}
	else{
		if(!dalvik_hook_setup(dh, s,s2,s3,ns,onetoall))
			log("----------- HOOK SETUP FALLITO ------------- : %s, %s\n", c,c2)
		check =  dalvik_hook(&d, dh);
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
	/**
	char *fifoname = "/data/local/tmp/fortune.fifo";
    int fifo_server, fifo_client;
    char line[80];
    int pid = 0, rc, nread;
    char input[28];
    int cont = 0;
	if (mkfifo(fifoname, 0777) == -1) { 
		if (errno!=EEXIST) {
			log("XXX5 pty error %s\n", strerror(errno))
		}
		log("XXX5 ERRORE FIFO = %s\n", strerror(errno))
	}
	fifo_server = open(fifoname, O_RDONLY);
	if (open(fifoname, O_WRONLY) < 0) {
		log("XXX5 pty error %s\n", strerror(errno))
	}
	log("XXX5 CREATO FIFO!!!!!\n")	
	while(1){
		log("XXX5 INIZIO  LOOP\n")
		log("XXX5 chiamato suspend all\n")
		nread = read(fifo_server, line, 79); 
		log("XXX5 LETTO DA FIFO\n")
		if((cont % 2) == 0){
		log("XXX5 chiamo unhook, cont = %d\n",cont)
		//int ret = _tryLockThreadList(&d);
		//log("XXX4 PRIMA DI SUSPEND THREAD ritornato + %d\n", ret)
		void * niente = getSelf(&d);
		log("XXX5 preso self vale = 0x%x\n", niente)
		niente = getSelf(&d);
		log("XXX5 preso self vale = 0x%x\n", niente)
		niente = getSelf(&d);
		log("XXX5 preso self vale = 0x%x\n", niente)
		log("XXX5 chiamo suspend all\n")
		_suspendAllT(&d);
		log("XXX5 chiamato suspend all\n")
		//_dumpAllT(&d);
		//_resumeAllT(&d);
		//callSuspendThread(&d);
		//_suspendSelf(&d); <---- sospende native
		//_unlockThreadList(&d);
		//log("XXX4 DOPO SUSPEND THREAD!!!\n")
		}
		else{
		log("XXX5 chiamo resume all, cont =  %d\n", cont)
		//_tryLockThreadList(&d);
		_resumeAllT(&d);
		//_dumpAllT(&d);
		//_resumeThread(&d);
		//_unlockThreadList(&d);
		}
		line[nread] = 0;
		fifo_client = open(line, O_WRONLY);
		nread = write(fifo_client,"ciao dal server\n",strlen("ciao dal server\n"));
		close(fifo_client);
		cont++;
	}
	*/
	/**
	JNIEnv* env = (JNIEnv*) get_jni_for_thread(&d);
	log("XXX4 preso env\n")
	//int gotVM = (*env)->GetJavaVM(env,&jvm);
   //log("PRESO VM %d\n", gotVM);

	//int status = (*jvm)->AttachCurrentThread(env, &env,NULL);
	//if (status != JNI_OK)
   // 	log("AttachCurrentThread failed %d\n",status);
	_unhook(env);
	log("XXX4 THREAD CHIAMATO UNHOOK\n")
	*/
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
	struct thargs* largs = (struct thargs*)malloc(sizeof(struct thargs));
	largs->dexpath = (char*)malloc( (strlen(args->dexpath)+1) * sizeof(char));
	largs->clsname = (char*)malloc( (strlen(args->clsname)+1) * sizeof(char));
	memcpy(largs, args, sizeof(struct thargs));
	log("XXX7 dentro thread wrapper:\n")
	log("ricevuto: %s = %s, %s = %s\n", args->dexpath, largs->dexpath, args->clsname, largs->clsname)
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
void*_dumpJavaClass(JNIEnv *env, jobject thiz, jobject str){
	char *clsname = (*env)->GetStringUTFChars(env, str, NULL);
	log("XXX7 provo a caricare = %s\n", clsname)
	_loadClass(&d, str);

	dalvik_dump_class(&d,clsname);
	(*env)->ReleaseStringUTFChars(env, str, clsname); 
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
	{"dumpJavaClass", "(Ljava/lang/String;)V",
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
	
	jclass *clazz33 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/manageADDI", cookie);
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

	jclass *clazz7 =(jclass*) dexstuff_defineclass(&d, "org/tesi/core/DalvikHookImpl", cookie);
	jclass *clazz3 =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/HookList", cookie);
	jclass *clazz4 = (jclass*)dexstuff_defineclass(&d, "org/tesi/core/MyInit", cookie);
	//jclass *clazz5 = (jclass*) dexstuff_defineclass(&d, "org/tesi/utils/AppContextConfig", cookie);
	jclass *clazz6 = (jclass*) dexstuff_defineclass(&d, "org/tesi/utils/sendSMS", cookie);

	
	JNIEnv *env = (JNIEnv*) get_jni_for_thread(&d);
	log("ddi XXX6 init preso env = 0x%x\n", env)
	//jbclazz = (*env)->FindClass(env, "java/lang/Boolean");
	//jiclazz = (*env)->FindClass(env, "java/lang/Integer");
	//dalvikCls = (*env)->FindClass(env, DALVIKHOOKCLS);

	(*env)->RegisterNatives(env, (*env)->FindClass(env, "org/sid/addi/core/manageADDI"), method_table, sizeof(method_table) / sizeof(method_table[0]));

	if(env){
		jclass mycls = (*env)->FindClass(env, "org/tesi/core/MyInit");
		log("XXX4 trovata classe myinit = 0x%x\n", mycls)
		jmethodID constructor = (*env)->GetMethodID(env, mycls, "<init>", "()V");
		jmethodID place_hook = (*env)->GetMethodID(env, mycls, "place_hook", "()V");
		jobject obj = (*env)->NewObject(env, mycls, constructor); 
		log("XXX4 creato oggetto della classe MyInit = 0x%x, mid = 0x%x\n", obj, place_hook)
		(*env)->CallVoidMethod(env,obj, place_hook);
	} 

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
