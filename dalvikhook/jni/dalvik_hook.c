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

#include "dexstuff.h"
#include "dalvik_hook.h"
#include "lista.h"
#include "Misc.h"
#include "log.h"

struct dexstuff_t d;
lista L;
pthread_mutex_t mutex; //list mutex
int debug = 1;
int cookie = NULL;
pthread_t pty_t;
JavaVM* g_JavaVM = NULL;


int dalvik_hook_setup(struct dalvik_hook_t *h, char *cls, char *meth, char *sig, int ns, void *func)
{
	if (!h)
		return 0;

	pthread_mutex_init(&h->mutexh, NULL);
	strcpy(h->clname, cls);
	
	strncpy(h->clnamep, cls+1, strlen(cls)-2);
	h->clnamep[strlen(cls)-2] = '\0';
	log("DALVIK HOOK SETUP len cls ==== %d, cls == %s, clnamep === %s \n", strlen(cls)-2, h->clname, h->clnamep)	
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


void* dalvik_hook(struct dexstuff_t *dex, struct dalvik_hook_t *h)
{
	if (h->debug_me)
		log("dalvik_hook: class %s\n", h->clname)
	
	void *target_cls = dex->dvmFindLoadedClass_fnPtr(h->clname);
	if (h->debug_me)
		log("class = 0x%x\n", target_cls)

	// print class in logcat
	if (h->dump && dex && target_cls)
		dex->dvmDumpClass_fnPtr(target_cls, (void*)1);

	if(h->debug_me){
		log("stampo debug in logcat!!\n");
		dex->dvmDumpClass_fnPtr(target_cls, (void*)1);		
	}
		
	if (!target_cls) {
		if (h->debug_me)
			log("target_cls == 0\n")
		return (void*)0;
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
			log("DALVIK_HOOK nativeFunc 0x%x, dalvin_hook_t: 0x%x\n", h->method->nativeFunc, h->native_func)		
			log("DALVIK_HOOK insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->method->insSize, h->method->registersSize, h->method->outsSize)
		}
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

		return (void*)1;
	}
	else {
		if (h->debug_me)
			log("could NOT patch %s\n", h->method_name)
	}

	return (void*)0;
}

int dalvik_prepare(struct dexstuff_t *dex, struct dalvik_hook_t *h, JNIEnv *env)
{

	// this seems to crash when hooking "constructors"
	//log("DALVIK_HOOK DENTRO DALVIK_PREPARE\n")
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

	dex->dvmUseJNIBridge_fnPtr(h->method, h->native_func);
	
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

int load_dex_wrapper(JNIEnv *env, void *thiz, struct dalvik_hook_t *res, va_list lhook, char*desc){
	jclass gclazz, mycls, *clazz;
	jmethodID dex_meth;

	char *myshorty = (char*) malloc(sizeof(desc));
	parse_descriptor(desc,myshorty);
	log("XXX5 ho trovato: %s\n", myshorty)

	jclass jbclazz = (*env)->FindClass(env, "java/lang/Boolean");
	jmethodID id = (*env)->GetMethodID(env, jbclazz, "<init>", "(Z)V");

	if(debug)
		log("XXX ho un dex method da chiamare = %s!!!\n", res->dex_meth)
	if(!res->loaded){
		if(debug)
			log("XXX DEVO CARICARE LA CLASSE DAL DEX %s\n", res->dex_class)
		clazz = (jclass *)dexstuff_defineclass(&d, res->dex_class, cookie);
		mycls = (*env)->FindClass(env, res->dex_class);
		log("XXX5 trovata class at 0x%x\n", mycls)
		gclazz = (*env)->NewGlobalRef(env, mycls);
		log("XXX5 creata global reference\n")
		res->DexHookCls = gclazz;
		res->loaded=1;
	}
	else{
		log("XXX DEX CLASS CACHED!!!\n")
	}
	//prova salvataggio oggetto this
	if(strcmp(res->dex_meth, "getSharedPref") == 0){
		log("XXX4 CHIAMATO getSharedPref\n")
		jmethodID set_thiz = (*env)->GetStaticMethodID(env, res->DexHookCls, "setThiz", "(Ljava/lang/Object;)V");
		if(set_thiz){
			log("XXX4 TROVATO SET THIZ, lo chiamo\n")
			(*env)->CallStaticVoidMethod(env,res->DexHookCls, set_thiz, thiz);
			log("XXX4 chiamato set thiz\n")
		}
	}
	if(res->real_args == 0){
		if(debug)
			log("XXX chiamo il metodo DEX 0 args con clazz = 0x%x, gclazz = 0x%x\n",  clazz, res->DexHookCls)
		dex_meth = (*env)->GetStaticMethodID(env, res->DexHookCls, res->dex_meth, "()V");
		(*env)->CallStaticVoidMethod(env,res->DexHookCls, dex_meth);	
		if(debug)
			log("XXX method %s chiamato\n", res->method_name)
	}
	else{	
		log("XXX5 il metodo ha argomenti!\n")
	//have args	
		jobjectArray ja;
		jbooleanArray jba;
		

		jboolean jbargs[res->real_args];
		int jaboolean = 0;
		
		ja = (*env)->NewObjectArray(env, res->real_args, (*env)->FindClass(env, "java/lang/Object"), NULL);
		jba = (*env)->NewBooleanArray(env, res->real_args);

		jobject gja = (*env)->NewGlobalRef(env, ja);
		//jobject gjba = (*env)->NewGlobalRef(env, jba);
		
		jobject jbobj;

		//  non viene creato nell ordine correto (Z)V diventa VZ!!
		//char *pshorty  = res->method->shorty;

		int i = 0;

		if(debug)
			log("XXX chiamo il metodo DEX con ja = 0x%x, clazz = 0x%x, gclazz = 0x%x con pshorty = %s\n", gja, clazz, res->DexHookCls, myshorty)
		
		//problemi quando costruisco arg di tipo nativo jboolean, ecc..
		for(i=0;i<res->real_args;i++){
			//log("XXX4  DEBUG1 = %c\n", *myshorty)
			switch(*myshorty){
				case 'Z':	
					log("dentro boolean creo oggetto\n")
					jbobj = (*env)->NewObject(env, jbclazz, id, va_arg(lhook, uint32_t));
					log("creato oggetto!!\n")
					(*env)->SetObjectArrayElement(env, gja, i, jbobj);
					log("inserito oggetto nell'array\n")
					jaboolean = 1;	
					myshorty++;
					break;
				case 'I':
					log("XXX load dex argomento[%d] tipo int\n", i)
					(*env)->SetObjectArrayElement(env,gja,i,va_arg(lhook,jint));
					myshorty++;
					break;
				default:
					//log("XXX4  DEBUG2 = %c\n", *myshorty)
					log("XXX load dex argomento[%d] tipo object\n", i)
					(*env)->SetObjectArrayElement(env,gja,i,va_arg(lhook,jobject *));
					myshorty++;
					break;
			}
			
		}
		log("XXX impostato gli argomenti!\n")
		//dump_jni_ref_tables(&d);
		if(jaboolean){

			dex_meth = (*env)->GetStaticMethodID(env, res->DexHookCls, res->dex_meth, "([Ljava/lang/Object;)V");
	
			(*env)->CallStaticVoidMethod(env,res->DexHookCls, dex_meth, gja);
			//(*env)->CallStaticVoidMethodV(env,res->DexHookCls, dex_meth, glhook);
			log("XXX chiamato metodo\n")
		}
		else{
			dex_meth = (*env)->GetStaticMethodID(env, res->DexHookCls, res->dex_meth, "([Ljava/lang/Object;)V");
			(*env)->CallStaticVoidMethod(env,res->DexHookCls, dex_meth, gja);	
			//(*env)->CallStaticVoidMethodV(env,res->DexHookCls, dex_meth, glhook);	
		}
		(*env)->DeleteGlobalRef(env, gja);
		//(*env)->DeleteGlobalRef(env, gjba);
	}
	//(*env)->DeleteGlobalRef(env, gclazz);
	//(*env)->DeleteLocalRef(env, clazz);			
	log("XXX FINITO USO DEX\n")
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
 void* onetoall(JNIEnv *env, jobject obj, ...){
	
	log("--------------------- ONETOALL-------------------------------\n")
	char * cl = (char*)malloc(sizeof(char)*256);
	char * name = (char*)malloc(sizeof(char)*256);
	char * descriptor = (char*)malloc(sizeof(char)*256);
	char * hash = (char*)malloc(sizeof(char)*512);

	memset(cl,0,sizeof(cl));
	memset(name,0,sizeof(name));
	memset(descriptor,0,sizeof(descriptor));
	memset(hash,0,sizeof(hash));

	struct dalvik_hook_t *res;
	

	jobject str = 0;
	int flag = 0;
	int j=0;
	jfloat fret = 0;
 	jlong lret = 0;
	
	va_list l, lhook;

	va_start(l,obj);
	va_start(lhook,obj);

	get_caller_class(env,obj,cl);
	
	get_method(&d,name,descriptor);
	//dvmDotToSlash(name);

	//printStackTrace(env);
	//hash e': clname+method_name+method_descriptor
	//strcpy(hash,cl);
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

	
	memset(cl,0,sizeof(cl));
	strcpy(cl,res->method_sig);

	char *s = parse_signature(cl);
	//log("dopo parse signature\n");
	char *pointer = s;
	j = is_string_class(pointer);
	if(j)
		flag = 1;

	

	pthread_mutex_lock(&(res->mutexh));
	dalvik_prepare(&d, res, env);
	if(strlen(res->dex_meth) > 0){
		load_dex_wrapper(env, obj, res, lhook, descriptor);				
	} 	

	if(debug)
		log("!!!!PRESO MUTEX %s con signature = %s, res = @0x%x, il metodo: @0x%x con mid = 0x%x ha: %d argomenti, skippable = %d, type = %c\n", res->method_name, res->method_sig, res, res->method, res->mid,res->real_args, res->skip, *s)

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
					dalvik_postcall(&d, res);
					free(cl);
					free(name);
					free(descriptor);
					free(hash);
					pthread_mutex_unlock(&(res->mutexh));
					if(debug)
						log("----------------------------------------------------\n")
					return str;
				}
				else{
					log("XXX errore str NULL\n")
					dalvik_postcall(&d, res);
					free(cl);
					free(name);
					free(descriptor);
					free(hash);
					pthread_mutex_unlock(&(res->mutexh));
					if(debug)
						log("----------------------------------------------------\n")
					return str;
				}
			}
			else{
				if(debug)
					log("return a class: @0x%x\n", str)
				dalvik_postcall(&d, res);
				free(cl);
				free(name);
				free(descriptor);
				free(hash);
				pthread_mutex_unlock(&(res->mutexh));
				log("----------------------------------------------------\n")
				return str;
			}
			
		case 'V':
			if(res->skip){
				if(debug)
					log("VOID :::: SKIPPO IL METODO!!!\n")
				dalvik_postcall(&d, res);	
				free(cl);
				free(name);
				free(descriptor);
				free(hash);
				pthread_mutex_unlock(&(res->mutexh));
				log("----------------------------------------------------\n")
				return (jint)1;
			}
			else{
				if(debug)
					log("VOID obj = 0x%x, mid = 0x%x, l = 0x%x, name= %s, realargs = %d\n", obj, res->mid,l, name, res->real_args)
				
				if(strstr(name, "<init>") != NULL){
					if(debug)
						log("strtstr(%s) STO CHIAMANDO UN COSTRUTTORE: clnamep: %s, name: %s, cls = 0x%x, mid = 0x%x, l = 0x%x\n", name,res->clnamep, res->method_name, res->cls, res->mid, l)					
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
			dalvik_postcall(&d, res);
			free(cl);
			free(name);
			free(descriptor);
			free(hash);
			pthread_mutex_unlock(&(res->mutexh));
			break;		
		case 'Z':
			log("!!BOOOLEAN___\n") //boolean is always true :)
			if(res->skip){
				log("!!!!SKIPPO\n")
				dalvik_postcall(&d, res);
				free(cl);
				free(name);
				free(descriptor);
				free(hash);
				pthread_mutex_unlock(&(res->mutexh));
				if(debug)
					log("----------------------------------------------------\n")
				if(res->skip == 1)
					return (jboolean)1;				
				else return (jboolean)0;
			}
			else{
				if(res->real_args == 0){
					
					str = (*env)->CallBooleanMethod(env,obj,res->mid);	
					
					dalvik_postcall(&d, res);
					if(debug)
						log("----------------------------------------------------\n")
				}
				else{
					
					str = (*env)->CallBooleanMethodV(env,obj,res->mid,l);
					
					dalvik_postcall(&d, res);
					if(debug)
						log("----------------------------------------------------\n")
				}
			}
			free(cl);
			free(name);
			free(descriptor);
			free(hash);		
			pthread_mutex_unlock(&(res->mutexh));
			return (jboolean)str;
		case 'I':
			if(debug){
				log("XXX2 DENTRO INT METHOD\n")
				//printStackTrace(env);
			}
			if(res->skip){
				dalvik_postcall(&d, res);
				free(cl);
				free(name);
				free(descriptor);
				free(hash);
				pthread_mutex_unlock(&(res->mutexh));
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
				
				dalvik_postcall(&d, res);
				if(debug)
					log("----------------------------------------------------\n")
			}
			free(cl);
			free(name);
			free(descriptor);
			free(hash);
			pthread_mutex_unlock(&(res->mutexh));
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
			dalvik_postcall(&d,res);
			free(cl);
			free(name);
			free(descriptor);
			free(hash);
			if(debug)
				log("----------------------------------------------------\n")
			pthread_mutex_unlock(&(res->mutexh));
			return result;			
	}
	if(debug){
		log("XXX io sono uscito dalla jail: %s\n", res->method_name)
		log("----------------------------------------------------\n")
	}
	return;
}
void _createStruct( JNIEnv* env, jobject thiz, jobject clazz )
{
	pthread_mutex_lock(&mutex);
	
	if(debug)
		log("------------ CREATE STRUCT!!! -------- \n")

	
	jclass cls = (*env)->FindClass(env, "org/sid/addi/core/DalvikHook");
	log("XXX4 create struct DALVIK HOOK SID = 0x%x\n", cls)

	jmethodID clname = (*env)->GetMethodID(env,cls,"get_clname","()Ljava/lang/String;");
	jmethodID method_name = (*env)->GetMethodID(env,cls,"get_method_name","()Ljava/lang/String;");
	jmethodID method_sig = (*env)->GetMethodID(env,cls,"get_method_sig","()Ljava/lang/String;");
	jmethodID dexmethod= (*env)->GetMethodID(env,cls,"get_dex_method","()Ljava/lang/String;");
	jmethodID dexclass= (*env)->GetMethodID(env,cls,"get_dex_class","()Ljava/lang/String;");
	jmethodID hashid = (*env)->GetMethodID(env,cls,"get_hashvalue","()Ljava/lang/String;");
	jmethodID argsid = (*env)->GetMethodID(env,cls,"get_myargs","()I");
	jmethodID nsid = (*env)->GetMethodID(env,cls,"getNs","()I");
	jmethodID sid = (*env)->GetMethodID(env,cls,"isSkip","()I");

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
	strcpy(dh->dex_meth,"");
	if(strlen(s4) > 0){
		strcpy(dh->dex_meth,s4);
		strcpy(dh->dex_class,s6);	
	}
	char *pointer;
	pointer = parse_signature(s3);	
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
		if(!dalvik_hook(&d,dh))
			log("----------- HOOK FALLITO ------------- : %s, %s\n", c,c2)
		log("HOOK PLACED\n")		
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
	dalvik_dump_class(&d, "Landroid/content/pm/PackageManager;");
	
	char *res = "/data/local/tmp/req.fifo";
	char *req = "/data/local/tmp/res.fifo";
	jobject str = (*envLocal)->NewStringUTF(envLocal, "/data/local/tmp/req.fifo");
	jobject gstr = (*envLocal)->NewGlobalRef(envLocal, str);
	jobject str1 = (*envLocal)->NewStringUTF(envLocal, "/data/local/tmp/res.fifo");
	jobject gstr1 = (*envLocal)->NewGlobalRef(envLocal, str1);
	log("XXX5 CHIAMO NEW OBJECT\n")
	jobject mobj = (*envLocal)->NewObject(envLocal, mycls, constructor, gstr, gstr1); 
	jmethodID start = (*envLocal)->GetMethodID(envLocal, mycls, "start", "()V");
	log("XXX5 chiamo metodo su obj = 0x%x, start = 0x%x\n", mobj, start)

	(*envLocal)->CallVoidMethod(envLocal, mobj, start, NULL);
	(*g_JavaVM)->DetachCurrentThread(g_JavaVM);
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
	//pthread_exit(1);
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
	_dumpAllT(&d);
}
void * _wrapperResumeAll(){
	log("XXX5 chiamato wrapper resume\n")
	_resumeAllT(&d);
	_dumpAllT(&d);
}
JNINativeMethod method_table[] = {
    { "createStruct", "(Lorg/sid/addi/core/DalvikHook;)V",
        (void*) _createStruct },
    { "createPTY", "()V",
        (void*) _createPTY },
    {"suspendALL","()V",
		(void*)_wrapperSuspendAll},
    {"resumeALL","()V",
		(void*)_wrapperResumeAll},
	{"unhook","(Ljava/lang/String;)V",
		(void*)_unhook}
};

jint my_ddi_init(){
	dexstuff_resolv_dvm(&d);
	dalvik_dump_class(&d, "Landroid/content/pm/IPackageManager$Stub$Proxy;");
	pthread_mutex_init(&mutex, NULL);

	if(debug)
		log("-------------------DENTRO LOAD DEX INIT ---------------------\n")
	cookie = dexstuff_loaddex(&d, "/data/local/tmp/classes.dex");
	//devo definire tutte le classi di controllo usate dagli hook
	jclass *clazz33 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/manageADDI", cookie);
	jclass *clazz55 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/StringHelper", cookie);
	jclass *clazz44 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/LogWrapper", cookie);	
	jclass *clazz22 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/DalvikHook", cookie);
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
	//jclass *clazz67 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/Executor", cookie);
/*
	log("XXX5 vediamo un po cosa carico: xml = 0x%x, common = 0x%x, commandwrapper = 0x%x, argwrapper = 0x%x commands = 0x%x, exec = 0%x\n", clazz66,
				clazz60, clazz61, clazz62, clazz64, clazz67)
*/
	

	//log("!!!XXX5 TROVATO CLASSE SID ADDI = 0x%x, manage = 0x%x\n", clazz22, clazz33)
	jclass *clazz7 =(jclass*) dexstuff_defineclass(&d, "org/tesi/fakecontext/DalvikHookImpl", cookie);
	jclass *clazz3 =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/HookList", cookie);
	jclass *clazz4 = (jclass*)dexstuff_defineclass(&d, "org/tesi/fakecontext/MyInit", cookie);
	jclass *clazz5 = (jclass*) dexstuff_defineclass(&d, "org/tesi/fakecontext/AppContextConfig", cookie);
	jclass *clazz6 = (jclass*) dexstuff_defineclass(&d, "org/tesi/fakecontext/sendSMS", cookie);

	
	JNIEnv *env = (JNIEnv*) get_jni_for_thread(&d);
	log("ddi XXX6 init preso env = 0x%x\n", env)

	
	(*env)->RegisterNatives(env, (*env)->FindClass(env, "org/sid/addi/core/manageADDI"), method_table, sizeof(method_table) / sizeof(method_table[0]));

	if(env){
		jclass mycls = (*env)->FindClass(env, "org/tesi/fakecontext/MyInit");
		log("XXX4 trovata classe myinit = 0x%x\n", mycls)
		jmethodID constructor = (*env)->GetMethodID(env, mycls, "<init>", "()V");
		jmethodID place_hook = (*env)->GetMethodID(env, mycls, "place_hook", "()V");
		jobject obj = (*env)->NewObject(env, mycls, constructor); 
		log("XXX4 creato oggetto della classe MyInit = 0x%x, mid = 0x%x\n", obj, place_hook)
		(*env)->CallVoidMethod(env,obj, place_hook);
		/**
		if(mycls && obj && start_magic){
			jclass fake = (jclass) (*env)->NewGlobalRef(env,mycls);
			dump_jni_ref_tables(&d);
			//myl.eid = start_magic;
			//myl.mid = constructor;
			//myl.obj = (jobject) (*env)->NewGlobalRef(env,obj);
		}
		*/
	} 

	if(debug)
		log("------------------- FINE LOAD DEX INIT ---------------------\n")
	return JNI_OK;
}

void _unhook(JNIEnv *env, jobject thiz, jobject str)
{
	log("XXX5 ------------ UNHOOK SONO STATO CHIAMATO -------- \n")
	//dalvik_prepare(&d,&sb1, env);
	

	//char *c = "Landroid/app/SharedPreferencesImpl;getString(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;";
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
	pthread_mutex_unlock(&(res->mutexh));
	(*env)->DeleteLocalRef(env, str);
	log("XXX5 FINITO UNHOOK ------------------- \n")
	//dalvik_postcall(&d, &sb1);
	//dalvik_hook_setup(&sb1, "Landroid/app/Activity;",  "startActivity",  "(Landroid/content/Intent;)V", 2 ,my_start_activity);
	//dalvik_hook(&d, &sb1);
    //return (*env)->NewStringUTF(env, "Hello from JNI !");
    
}

void* setJavaVM(JavaVM* ajvm){
	g_JavaVM = ajvm;
}
