/*
 *  Collin's Dynamic Dalvik Instrumentation Toolkit for Android
 *  Collin Mulliner <collin[at]mulliner.org>
 *
 *  (c) 2012,2013
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
#include <stdlib.h>

/* PIPES*/
#include <inttypes.h>
#include <stdint.h>

#include "hook.h"
#include "dexstuff.h"
#include "dalvik_hook.h"
#include "base.h"
#include "lista.h"
 

#undef log

#define log(...) \
        {FILE *fp = fopen("/data/local/tmp/strmon.log", "a+");\
        fprintf(fp, __VA_ARGS__);\
        fclose(fp);}



static struct hook_t eph;
static struct dexstuff_t d;
static struct dalvik_hook_t sb1;
static int cont = 0;
static int debug = 0;



static void my_log(char *msg)
{
	log("%s",msg)
}
static void my_log2(char *msg)
{
	if (debug)
		log("%s",msg)
}


lista L;
pthread_mutex_t mutex;


// helper function
void printString(JNIEnv *env, jobject str, char *l)
{
	char *s = (*env)->GetStringUTFChars(env, str, 0);

	if (s) {
		log("%s%s\n", l, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}
}



void stampa_lista(lista l){
	struct dalvik_hook_t* dd;
	while((l) != NULL){
		dd = (struct dalvik_hook_t *)(l)->dh;
		log("+++++++++++++++++++++++++++++++++++++++++++\n")
		log("clname = %s\n", (l)->clname)
		log("meth_name = %s\n", (l)->meth_name)
		log("meth_sign = %s\n", (l)->meth_sig)
		log("hash = %s\n", (l)->hashvalue)
		log("real num args = %d\n", dd->real_args)
		log("DALVIK HOOK = 0x%x\n", dd->method)
		log("+++++++++++++++++++++++++++++++++++++++++++\n")
		(l) = (l)->next;
	}
}

void printStackTrace(JNIEnv *env) {
char *buf;
log("###################################################################################printStackTrace{\n");
    jclass cls = (*env)->FindClass(env,"java/lang/Exception");
    if (cls != NULL) {
        jmethodID constructor = (*env)->GetMethodID(env,cls, "<init>", "()V");
        if(constructor != NULL) {
            jobject exc = (*env)->NewObject(env,cls, constructor);
            if(exc != NULL) {
                jmethodID printStackTrace = (*env)->GetMethodID(env,cls, "printStackTrace", "()V");
                if(printStackTrace != NULL) {
                	log("AAAAAAAAAAAAAAAAAAAAAAAAAAAA\n")
                    (*env)->CallVoidMethod(env,exc, printStackTrace);
                } else { log("err4"); }
            } else { log("err3"); }
            (*env)->DeleteLocalRef(env,exc);
        } else { log("err2"); }
    } else { log("err1"); }
    /* free the local ref */
    (*env)->DeleteLocalRef(env,cls);
log("###################################################################################printStackTrace}\n");
}


static void* get_caller_class(JNIEnv *env, jobject obj, char *c){

	jclass cls = (*env)->GetObjectClass(env,obj);
	// First get the class object
	jmethodID mid = (*env)->GetMethodID(env,cls, "getClass", "()Ljava/lang/Class;");
	jobject clsObj = (*env)->CallObjectMethod(env,obj, mid);

	// Now get the class object's class descriptor
	cls = (*env)->GetObjectClass(env,clsObj);

	// Find the getName() method on the class object
	mid = (*env)->GetMethodID(env,cls, "getName", "()Ljava/lang/String;");

	// Call the getName() to get a jstring object back
	jstring strObj = (jstring)(*env)->CallObjectMethod(env,clsObj, mid);

	// Now get the c string from the java jstring object
	const char* str2 = (*env)->GetStringUTFChars(env,strObj, NULL);

	// Print the class name
	log("\nCalling class is: %s\n", str2)
	strcpy(c,str2);

	// Release the memory pinned char array
	(*env)->ReleaseStringUTFChars(env,strObj, str2);
}

static int is_string_class(char *c){
	log("IS STRING RICEVUTO = %s\n", c)
	char *s = strstr(c, "String");
	if(s != NULL)
		return 1;
	else
		return 0;
}
static char* parse_signature(char *src){

	char *c = src;
	while(c != NULL && *c != ')')
	{	
		c++;
	}

	c++;	
	return c;
}

static jfloat jfloat_wrapper(JNIEnv *env, jobject obj, ...){

}
static jlong jlong_wrapper(JNIEnv *env, jobject obj, ...){

}
static void* onetoall(JNIEnv *env, jobject obj, ...){
	
	log("--------------------- ONETOALL-------------------------------\n")
	char * cl = (char*)malloc(sizeof(char)*256);
	char * name = (char*)malloc(sizeof(char)*256);
	char * descriptor = (char*)malloc(sizeof(char)*256);
	char * hash = (char*)malloc(sizeof(char)*512);

	memset(cl,0,sizeof(cl));
	memset(cl,0,sizeof(name));
	memset(cl,0,sizeof(descriptor));
	memset(cl,0,sizeof(hash));

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

	strcpy(hash,name);
	strcat(hash,descriptor);
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
	log("!!!!PRESO MUTEX %s con signature = %s, res = @0x%x, il metodo: @0x%x con mid = 0x%x ha: %d argomenti, skippable = %d, type = %c\n", res->method_name, res->method_sig, res, res->method, res->mid,res->real_args, res->skip, *s)
 	dalvik_prepare(&d, res, env);
 	
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
			if(strlen(res->dex_meth) > 0){
				
				log("XXX ho un dex method da chiamare = %s!!!\n", res->dex_meth)
				if(!res->loaded){
					jclass gclazz;
					log("XXX DEVO CARICARE LA CLASSE DAL DEX %s\n", res->dex_class)
					int cookie = dexstuff_loaddex(&d, "/data/local/tmp/classes.dex");
					jclass *clazz = (jclass *)dexstuff_defineclass(&d, res->dex_class, cookie);
					jclass mycls = (*env)->FindClass(env, res->dex_class);
					gclazz = (*env)->NewGlobalRef(env, mycls);
					log("dopo global reference 0x%x\n", gclazz)
					//dalvik_dump_class(&d,res->dex_class);
					
					jmethodID dump_intent = (*env)->GetStaticMethodID(env, gclazz, res->dex_meth, "([Ljava/lang/Object;)V");
					res->loaded=1;
					if(res->real_args == 0){
						
						(*env)->CallStaticVoidMethod(env,gclazz, dump_intent);
						
					}
					else{		
						jobjectArray ja;
						ja = (*env)->NewObjectArray(env, 1, (*env)->FindClass(env, "java/lang/Object"), 0);
						(*env)->SetObjectArrayElement(env,ja,0,va_arg(lhook,jobject *));
						log("XXX chiamo il metodo DEX con ja = 0x%x, clazz = 0x%x, gclazz = 0x%x\n", ja, clazz, gclazz)
						dump_jni_ref_tables(&d);
						(*env)->CallStaticVoidMethod(env,gclazz, dump_intent, ja);	
						
					}
					(*env)->DeleteGlobalRef(env, gclazz);			
				}
				
				
			}
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
				dalvik_postcall(&d, res);
				free(cl);
				free(name);
				free(descriptor);
				free(hash);
				pthread_mutex_unlock(&(res->mutexh));
				log("----------------------------------------------------\n")
				return (jboolean)1;				
			}
			else{
				if(res->real_args == 0){
					
					str = (*env)->CallBooleanMethod(env,obj,res->mid);	
					
					dalvik_postcall(&d, res);
					
					log("----------------------------------------------------\n")
				}
				else{
					
					str = (*env)->CallBooleanMethodV(env,obj,res->mid,l);
					
					dalvik_postcall(&d, res);
					
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
			if(res->real_args == 0){
				
				str = (*env)->CallIntMethod(env, obj, res->mid);
				
				dalvik_postcall(&d, res);
				
				log("----------------------------------------------------\n")
			}
			else{
				
				str = (*env)->CallIntMethodV(env, obj, res->mid,l);
				
				dalvik_postcall(&d, res);
				
				log("----------------------------------------------------\n")
			}
			free(cl);
			free(name);
			free(descriptor);
			free(hash);
			pthread_mutex_unlock(&(res->mutexh));
			return (jint)str;
		default:
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
			log("fine myhook\n")
			free(cl);
			free(name);
			free(descriptor);
			free(hash);
			pthread_mutex_unlock(&(res->mutexh));
			log("----------------------------------------------------\n")
			return result;			
	}

	log("XXX io sono uscito dalla jail: %s\n", res->method_name)
	log("----------------------------------------------------\n")
	return;
}



jstring
Java_org_tesi_fakecontext_LoggerWrapper_stringFromJNI( JNIEnv* env, jobject thiz )
{
	log("------------ SONO STATO CHIAMATO -------- \n")
	//dalvik_prepare(&d,&sb1,env);
	//dalvik_postcall(&d, &sb1);
	//dalvik_hook_setup(&sb1, "Landroid/app/Activity;",  "startActivity",  "(Landroid/content/Intent;)V", 2 ,my_start_activity);
	//dalvik_hook(&d, &sb1);
    return (*env)->NewStringUTF(env, "Hello from JNI !");
}

void
Java_org_tesi_fakecontext_LoggerWrapper_myhook( JNIEnv* env, jobject thiz )
{
	log("------------ SONO STATO CHIAMATO -------- \n")
	//dalvik_prepare(&d,&sb1,env);
	dalvik_postcall(&d, &sb1);
	//dalvik_hook_setup(&sb1, "Landroid/app/Activity;",  "startActivity",  "(Landroid/content/Intent;)V", 2 ,my_start_activity);
	//dalvik_hook(&d, &sb1);
    //return (*env)->NewStringUTF(env, "Hello from JNI !");
}

static void get_info(JNIEnv *env){
	log("------------------GET INFO----------------------\n")

	jclass cls = (*env)->FindClass(env,"android/os/Build");
	jclass cls_ver = (*env)->FindClass(env,"android/os/Build$VERSION");
	jclass cls_sec = (*env)->FindClass(env,"android/provider/Settings$Secure");

	if(!cls_sec){
		log("ERRORE CLS SEC!!\n")
	}
	
	jfieldID fid = (*env)->GetStaticFieldID(env,cls,"MODEL","Ljava/lang/String;");
	jfieldID fingerid = (*env)->GetStaticFieldID(env,cls,"FINGERPRINT","Ljava/lang/String;");
	jfieldID hwid = (*env)->GetStaticFieldID(env,cls,"HARDWARE","Ljava/lang/String;");
	jfieldID bid = (*env)->GetStaticFieldID(env,cls,"BOARD","Ljava/lang/String;");
	jfieldID did = (*env)->GetStaticFieldID(env,cls,"DEVICE","Ljava/lang/String;");
	jfieldID bootid = (*env)->GetStaticFieldID(env,cls,"BOOTLOADER","Ljava/lang/String;");
	jfieldID brandid = (*env)->GetStaticFieldID(env,cls,"BRAND","Ljava/lang/String;");
	jfieldID pid = (*env)->GetStaticFieldID(env,cls,"PRODUCT","Ljava/lang/String;");
	jfieldID man_id = (*env)->GetStaticFieldID(env,cls,"MANUFACTURER","Ljava/lang/String;");
	jfieldID id = (*env)->GetStaticFieldID(env,cls,"ID","Ljava/lang/String;");



	jfieldID aid = (*env)->GetStaticFieldID(env,cls_sec,"ANDROID_ID","Ljava/lang/String;");
	jfieldID rel_id = (*env)->GetStaticFieldID(env,cls_ver,"RELEASE","Ljava/lang/String;");

	void *fake = (*env)->NewStringUTF(env, "LG-E400");
	(*env)->SetStaticObjectField(env,cls,fid,fake);

	fake = (*env)->NewStringUTF(env,"lge/e0_tmo_eur/e0:2.3.6/GRK39F/V10c-MAR-28-2012.2ED8A66C75:user/release-keys");
	(*env)->SetStaticObjectField(env,cls,fingerid,fake);

	fake = (*env)->NewStringUTF(env, "e0");
	(*env)->SetStaticObjectField(env,cls,hwid,fake);

	fake = (*env)->NewStringUTF(env, "e400");
	(*env)->SetStaticObjectField(env,cls,bid,fake);
	(*env)->SetStaticObjectField(env,cls,did,fake);

	fake = (*env)->NewStringUTF(env, "lge");
	(*env)->SetStaticObjectField(env,cls,brandid,fake);

	fake = (*env)->NewStringUTF(env, "e0_tmo_eu");
	(*env)->SetStaticObjectField(env,cls,pid,fake);

	fake = (*env)->NewStringUTF(env, "LGE");
	(*env)->SetStaticObjectField(env,cls,man_id,fake);

	fake = (*env)->NewStringUTF(env, "fc7974371af22c13");
	(*env)->SetStaticObjectField(env,cls_sec,aid,fake);

	void * res = (*env)->GetStaticObjectField(env,cls,fid);

	log("CLAZZ:  0x%x, FID: 0x%x, res: 0x%x\n",cls,fid,res)
	char *s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("DEBUG:::::MODEL = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}
	res = (*env)->GetStaticObjectField(env,cls,fingerid);	
	
	s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("FINGERPRINT = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}
	res = (*env)->GetStaticObjectField(env,cls,hwid);	
	
	s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("HARDWARE() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}
	res = (*env)->GetStaticObjectField(env,cls,bid);
	
	s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("BOARD() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}

	res = (*env)->GetStaticObjectField(env,cls,did);
	
	s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("DEVICE() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}
	res = (*env)->GetStaticObjectField(env,cls,bootid);
	
	s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("BOOTLOADER() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}
	res = (*env)->GetStaticObjectField(env,cls,brandid);
	
	s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("BRAND() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}
	res = (*env)->GetStaticObjectField(env,cls,pid);
	
	s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("PRODUCT() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}
	res = (*env)->GetStaticObjectField(env,cls,man_id);
	
	s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("MANUFACTURER() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}
	res = (*env)->GetStaticObjectField(env,cls,id);
	
	s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("ID() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}

	res = (*env)->GetStaticObjectField(env,cls_ver,rel_id);
	
	s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("RELEASE() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}
	res = (*env)->GetStaticObjectField(env,cls_sec,aid);
	
	s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("ANDROID_ID() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}
	log("-----------------------------------------\n")


}

void
Java_org_tesi_fakecontext_MyInit_createStruct( JNIEnv* env, jobject thiz, jobject clazz )
{
	pthread_mutex_lock(&mutex);
	log("------------ CREATE STRUCT!!! -------- \n")



	

	jclass cls = (*env)->FindClass(env, "org/tesi/fakecontext/DalvikHook");

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
	//log("class: 0x%x, mid: 0x%X, string: 0x%x, list = 0x%x, argsid = 0x%x, jint = %d\n",cls,dexmethod,c, &L, argsid, args)



	char *s = (*env)->GetStringUTFChars(env, c , NULL);
	char *s2 = (*env)->GetStringUTFChars(env, c2 , NULL);
	char *s3 = (*env)->GetStringUTFChars(env, c3 , NULL);
	char *s4 = (*env)->GetStringUTFChars(env, c4 , NULL);
	char *s5 = (*env)->GetStringUTFChars(env,c5,NULL); //hashvalue
	char *s6 = (*env)->GetStringUTFChars(env, c6 , NULL);

	log("chiamato con: %s, %s, %s, %s , %s, args vale = %d, ns = %d\n", s,s2,s3,s4,s5, args, ns)
	
	struct dalvik_hook_t *dh;
	dh = (struct dalvik_hook_t *)malloc(sizeof(struct dalvik_hook_t));
	//struct dalvik_hook_t dh;

	dh->real_args = args;
	dh->skip = flag;
	dh->loaded = 0;
	strcpy(dh->dex_meth,"");
	if(strlen(s4) > 0){
		strcpy(dh->dex_meth,s4);
		strcpy(dh->dex_class,s6);
		log("XXX dh->dex_meth = %s, dex_class = %s\n", dh->dex_meth, dh->dex_class)
	}

	char *pointer;
	pointer = parse_signature(s3);
	log("----- IL METODO HOOKATO RITORNA = %c\n", *pointer)
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
	log("XXXD inserisco dh = 0x%x 0x%x, %s,%s,%s,%s\n", &dh, dh, s,s2,s3,s5)
	int i = inserisci(&L,dh,s,s2,s3,s5);
	log("stampo lista: %d, dex_method = %s\n", i, dh->dex_meth)

	//stampa_lista(L);
	//free(dh);
	pthread_mutex_unlock(&mutex);
}

static void load_dex_init(){
	
	log("-------------------DENTRO LOAD DEX INIT ---------------------\n")
	//dalvik_dump_class(&d,"Lorg/tesi/fakecontext/FakeContext;");
	int cookie = dexstuff_loaddex(&d, "/data/local/tmp/classes.dex");
	jclass *clazz = (jclass*) dexstuff_defineclass(&d, "org/tesi/fakecontext/LoggerWrapper", cookie);
	void *clazz2 = dexstuff_defineclass(&d, "org/tesi/fakecontext/DalvikHook", cookie);
	void *clazz3 = dexstuff_defineclass(&d, "org/tesi/Hooks/HookList", cookie);
	void *clazz4 = dexstuff_defineclass(&d, "org/tesi/fakecontext/MyInit", cookie);
	void *clazz5 = dexstuff_defineclass(&d, "org/tesi/fakecontext/AppContextConfig", cookie);
	//dalvik_dump_class(&d, "Lorg/tesi/fakecontext/DalvikHook;");

	//void *clazz2 = dexstuff_defineclass(&d, "org/tesi/fakecontext/MessangerService", cookie);
	
	//myfind_loaded_class(&d,"Lorg/tesi/fakecontext/FakeContext;", "<init>","(Landroid/content/Intent;)V");
	//dalvik_dump_class(&d,"Lorg/tesi/fakecontext/FakeContext;");
	JNIEnv *env = (JNIEnv*) get_jni_for_thread(&d);

	log("dio porco clazz = 0x%x\n", clazz)
	//jclass gclazz = (*env)->NewGlobalRef(env, clazz);
	//log("dopo newglobalref = 0x%x\n", gclazz)	
	//get_info(env);

	//log("CLAZZ1: 0x%x, CLAZZ2: 0x%x, CLAZZ3 = 0x%x, CLAZZ4 = 0x%x\n", clazz, clazz2, clazz3, clazz4)
	if(env){
		log("DENTRO LOAD DEX INIT, JNIENV = 0x%x\n", env)
		jclass mycls2 = (*env)->FindClass(env, "org/tesi/fakecontext/LoggerWrapper");
		jclass mycls = (*env)->FindClass(env, "org/tesi/fakecontext/MyInit");
		jmethodID constructor = (*env)->GetMethodID(env, mycls, "<init>", "()V");
		jmethodID place_hook = (*env)->GetMethodID(env, mycls, "place_hook", "()V");
		jmethodID  start_magic = (*env)->GetMethodID(env,mycls2,"start_magic","(ILjava/lang/String;)V");
		//log("MID: 0x%x, magic: 0x%x\n", constructor, start_magic)
		jobject obj = (*env)->NewObject(env, mycls, constructor); 
		(*env)->CallVoidMethod(env,obj, place_hook);
		/**
		if(mycls && obj && start_magic){
			log("PORCODIOOOOOOOOOOOOOOOO SALVO UNA GLOBAL REF\n")
			jclass fake = (jclass) (*env)->NewGlobalRef(env,mycls);
			dump_jni_ref_tables(&d);
			//myl.eid = start_magic;
			//myl.mid = constructor;
			//myl.obj = (jobject) (*env)->NewGlobalRef(env,obj);
		}
		*/
		
	} 
	log("------------------- FINE LOAD DEX INIT ---------------------\n")

}



static void* myhook(JNIEnv *env, jobject obj, ...){

log("inizio hook\n")

va_list l;
va_start(l,obj);

dalvik_prepare(&d,&sb1, env);
void * res =(*env)->CallObjectMethodV(env, obj, sb1.mid, l);
//printString(env, str, "argomento = ");
dalvik_postcall(&d,&sb1);
log("fine myhook\n")
return res;

}

void do_patch()
{
  log("do_patch_mon()\n")
  dalvik_hook_setup(&sb1,"Ljava/security/MessageDigest;", "digest", "([B)[B", 2, myhook);
  dalvik_hook(&d,&sb1);

/**
  dalvik_hook_setup(&sb1, "Landroid/app/Activity;",  "startActivity",  "(Landroid/content/Intent;)V", 2 , mystart);
  dalvik_hook(&d, &sb1);

  dalvik_hook_setup(&sb2, "Landroid/app/Activity;",  "onCreate",  "(Landroid/os/Bundle;)V", 2 , sb2_onCreate);
  dalvik_hook(&d, &sb2);

  dalvik_hook_setup(&sb3, "Landroid/provider/Settings$Secure;", "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;", 2, sb3_getString);
  dalvik_hook(&d,&sb3);
  
  dalvik_hook_setup(&sb13, "Ljava/lang/String;", "equalsIgnoreCase", "(Ljava/lang/String;)Z", 2, sb13_equalsIgnoreCase);
  dalvik_hook(&d, &sb13);
  dalvik_hook_setup(&sb23, "Ljava/security/Signature;", "verify", "([B)Z", 2, my_verify );
  dalvik_hook(&d, &sb23);
  dalvik_hook_setup(&sb24, "Lorg/apache/harmony/xnet/provider/jsse/OpenSSLSocketImpl;", "verifyCertificateChain", "([[BLjava/lang/String;)V", 3, my_verify_chain );
  dalvik_hook(&d, &sb24);

  dalvik_hook_setup(&sb23, "Ljava/security/Signature;", "verify", "([B)Z", 2, my_verify );
  dalvik_hook(&d, &sb23);

  dalvik_hook_setup(&sb2, "Ljava/security/Signature;", "verify", "([BII)Z", 4, my_verify2 );
  dalvik_hook(&d, &sb2);

  dalvik_hook_setup(&sb24, "Lorg/apache/harmony/xnet/provider/jsse/OpenSSLSocketImpl;", "verifyCertificateChain", "([[BLjava/lang/String;)V", 3, my_verify_chain );
  dalvik_hook(&d, &sb24);


  dalvik_hook_setup(&sb20, "Landroid/net/NetworkInfo;", "getDetailedState", "()Landroid/net/NetworkInfo$DetailedState;", 1, my_get_detailed_state );
  dalvik_hook(&d, &sb20);

  dalvik_hook_setup(&sb21, "Landroid/net/NetworkInfo;", "getState", "()Landroid/net/NetworkInfo$State;", 1, my_get_state );
  dalvik_hook(&d, &sb21);
  
  dalvik_hook_setup(&sb6, "Landroid/telephony/TelephonyManager;", "getLine1Number", "()Ljava/lang/String;", 1, sb6_my_get_line1number );
  dalvik_hook(&d, &sb6);

  

  dalvik_hook_setup(&sb4, "Landroid/telephony/TelephonyManager;", "getDeviceId", "()Ljava/lang/String;", 1, my_get_device_id );
  dalvik_hook(&d, &sb4);

  dalvik_hook_setup(&sb13, "Landroid/telephony/TelephonyManager;", "getSubscriberId", "()Ljava/lang/String;", 1, sb13_my_get_subscriber );
  dalvik_hook(&d, &sb13);

  dalvik_hook_setup(&sb22, "Landroid/telephony/TelephonyManager;", "getSimSerialNumber", "()Ljava/lang/String;", 1, sb22_my_get_simnum );
  dalvik_hook(&d, &sb22);


  dalvik_hook_setup(&sb22, "Ljava/lang/String;", "compareTo", "(Ljava/lang/String;)I", 2, sb22_compareto);
  dalvik_hook(&d, &sb22);

  dalvik_hook_setup(&sb23, "Ljava/lang/StringBuilder;",  "toString",  "()Ljava/lang/String;", 1, sb23_tostring);
  dalvik_hook(&d, &sb23);




  dalvik_hook_setup(&sb23, "Lorg/bouncycastle/cert/CertUtils;",  "configure",  "(Lorg/bouncycastle/jcajce/provider/config;)V;", 2, sb24_castle);
  dalvik_hook(&d, &sb23);

  
	dalvik_hook_setup(&sb20, "Ljava/lang/StringBuilder;",  "toString",  "()Ljava/lang/String;", 1, sb20_tostring);
	dalvik_hook(&d, &sb20);

	dalvik_hook_setup(&sb5, "Ljava/lang/Class;", "getMethod", "(Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;", 3, sb5_getmethod);
	dalvik_hook(&d, &sb5);
	*/
}


static int my_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	int (*orig_epoll_wait)(int epfd, struct epoll_event *events, int maxevents, int timeout);
	orig_epoll_wait = (void*)eph.orig;
	// remove hook for epoll_wait
	hook_precall(&eph);

	// resolve symbols from DVM
	dexstuff_resolv_dvm(&d);
	load_dex_init();
	
	// insert hooks
	//do_patch();

//   dalvik_dump_class(&d,"Landroid/webkit/WebSettings;");
 //  dalvik_dump_class(&d,"Landroid/webkit/WebView;");
  
	// call original function
	int res = orig_epoll_wait(epfd, events, maxevents, timeout);    
	return res;
}

// set my_init as the entry point
void __attribute__ ((constructor)) my_init(void);

void my_init(void)
{
	log("DDI: started\n");

	pthread_mutex_init(&mutex, NULL);

 	// set to 1 to turn on, this will be noisy
	debug = 1;

 	// set log function for  libbase (very important!)
	set_logfunction(my_log);
	// set log function for libdalvikhook (very important!)
	dalvikhook_set_logfunction(my_log);

    hook(&eph, getpid(), "libc.", "epoll_wait", my_epoll_wait, 0);
}