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
#define BUFFER 4096
#define FAIL -1
#define SUCCESS 0


static struct hook_t eph;
static struct dexstuff_t d;

static int cont = 0;
static int debug;

static char *inputBuffer[BUFFER];

static void my_log(char *msg)
{
	log("%s",msg)
}
static void my_log2(char *msg)
{
	if (debug)
		log("%s",msg)
}


static struct dalvik_hook_t sb5;
static struct dalvik_hook_t sb20;
static struct dalvik_hook_t sb21;
static struct dalvik_hook_t sb22;
static struct dalvik_hook_t sb23;
static struct dalvik_hook_t sb24;
static struct dalvik_hook_t sb1;
static struct dalvik_hook_t sb2;
static struct dalvik_hook_t sb3;
static struct dalvik_hook_t sb4;
static struct dalvik_hook_t sb6;
static struct dalvik_hook_t sb13;
static struct logger_hook myl;

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
	strcpy(c,str2,sizeof(str2));

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
static void* onetoall(JNIEnv *env, jobject obj, ...){
	pthread_mutex_lock(&mutex);
	log("--------------------- ONETOALL-------------------------------\n")
	char cl[256];
	char name[256];
	char descriptor[256];
	char hash[256];
	struct dalvik_hook_t *res;
	void *str;
	int flag = 0;
	int skip = 0;
	int i = 0;
	int j=0;
	


	char *p = (char*)malloc(sizeof(char));
	jobjectArray ja;	
	va_list l;
	jint ns;
	jobject op;

	va_start(l,obj);

	get_caller_class(env,obj,cl);
	get_method(&d,name,descriptor);

	strncpy(hash,name,sizeof(name));
	strncat(hash,descriptor,sizeof(descriptor));
	log("CERCO NELLA LISTA : %s\n", hash)
	
	res = (struct dalvik_hook_t *) cerca(L, hash);
	strcpy(p,res->method_sig);

	char *s = parse_signature(p);

	char *pointer = s;
	j = is_string_class(pointer);


	if(j)
		flag = 1;

	ns = res->real_args;
	skip = res->skip;


	/**
	jvalue args[ns];
	
	

	//problemi con malloc
	ja = (*env)->NewObjectArray(env,ns, (*env)->FindClass(env,"java/lang/Object"), NULL);

	log("prima del for\n")

	for(i=0;i<ns;i++){
		(*env)->SetObjectArrayElement(env, ja, i, va_arg(l , jobject *)); //problemi con argomenti native type
		op = (*env)->GetObjectArrayElement(env, ja, i);
		args[i].l = op;
	}
	*/

	log("!!!!trovato = 0x%x, il metodo: @0x%x ha: %d argomenti, skippable = %d, type = %c\n", res, res->method, ns, skip, *s)

 	dalvik_prepare(&d, res, env);
 	
	switch(*s){
		//method return a class
		case 'L':
			if(ns == 0){
				//method have zero arguments
				if(res->sm){
					str = (*env)->CallStaticObjectMethod(env, obj, res->mid);	
				}

			}
			if(res->sm){
				str = (*env)->CallStaticObjectMethodV(env, obj, res->mid, l);
				log("success calling static: %s\n", res->method_name)
			}
			else{
				log("CLASSE yeah!!\n")
				str = (*env)->CallObjectMethodV(env, obj, res->mid, l);
				log("success calling : %s\n", res->method_name)
			}
			break;
		case 'V':
			if(skip){
				log("VOID :::: SKIPPO IL METODO!!!\n")
				dalvik_postcall(&d, res);	
				pthread_mutex_unlock(&mutex);
				log("RILASCIATO LOCK!\n")
				log("----------------------------------------------------\n")
				return (void*)1;
			}

			log("VOID obj = 0x%x, mid = 0x%x, l = 0x%x, name= %s\n", obj, res->mid,l, name)	
			if(strstr(name, "<init>") != NULL){

				log("STO CHIAMANDO UN COSTRUTTORE: clnamep: %s, name: %s, cls = 0x%x, findcls = 0x%x, mid = 0x%x, l = 0x%x\n", res->clnamep, res->method_name, res->cls, (*env)->FindClass(env, res->clnamep),res->mid, l)
			
				//log("call non virtual\n")
				//(*env)->CallNonvirtualVoidMethodV(env,obj,res->cls, res->mid, l);
				//(*env)->CallVoidMethodV(env, obj, res->mid, l);
			}
			else{
				log("STO CHIAMANDO : %s\n", name)
				(*env)->CallVoidMethodV(env, obj, res->mid, l);
			}
			log("ESCO success calling : %s\n", res->method_name)
			dalvik_postcall(&d, res);
			pthread_mutex_unlock(&mutex);
			log("RILASCIATO LOCK!\n")
			log("----------------------------------------------------\n")			
			return (void*)0;			
		case 'Z':
			log("!!BOOOLEAN___\n") //boolean is always true :)
			if(skip){
				dalvik_postcall(&d, res);
				pthread_mutex_unlock(&mutex);
				log("RILASCIATO LOCK!\n")
				log("----------------------------------------------------\n")
				return (jboolean)1;				
			}
			else{
				str = (*env)->CallBooleanMethodV(env,obj,res->mid,l);
				dalvik_postcall(&d, res);
				pthread_mutex_unlock(&mutex);
				log("RILASCIATO LOCK!\n")
				log("----------------------------------------------------\n")
				return str;
			}		
		default:
			log("DEFAULT \n")
			if(res->sm){
				str = (*env)->CallStaticObjectMethodV(env, obj, res->mid, l);
				log("success calling static: %s\n", res->method_name)
			}
			else{
				log("DEFAULT CLASSE yeah!!\n")
				str = (*env)->CallObjectMethodV(env, obj, res->mid, l);
				log("success calling : %s\n", res->method_name)
			}
			break;			
	}

	dalvik_postcall(&d, res);
	
	if (flag) {
		log("FLAG TRUE\n")
		if(str){
			char *s = (*env)->GetStringUTFChars(env, str, 0);
			if (s) {
				log("OTTENUTA STRINGA = %s\n", s)
				(*env)->ReleaseStringUTFChars(env, str, s); 
			}
			
			log("RILASCIATO LOCK!\n")
			log("----------------------------------------------------\n")
			pthread_mutex_unlock(&mutex);
			return str;
		}	
	}
	else{
		log("----------------------- FLAG FALSE-----------------------------\n")
		log("RILASCIATO LOCK!\n")
		pthread_mutex_unlock(&mutex);
		log("----------------------------------------------------\n")
		return str;
	}	
	pthread_mutex_unlock(&mutex);
	log("RILASCIATO LOCK!\n")
	log("----------------------------------------------------\n")
}


static void* wrapper_0(JNIEnv *env, jobject obj){ //0 argomenti
	pthread_mutex_lock(&mutex);
	log("--------------------- WRAPPER 0!!-------------------------------\n")
	char cl[256];
	char name[256];
	char descriptor[256];
	char hash[256];
	struct dalvik_hook_t *res;
	void* str;
	char *p = (char *)malloc(sizeof(char *));
	int j;
	int flag = 0;

	//get_caller_class(env,obj,cl);
	get_method(&d,name,descriptor);

	strncpy(hash,name,sizeof(name));
	strncat(hash,descriptor,sizeof(descriptor));
	log("CERCO NELLA LISTA : %s\n", hash)
	
	res = (struct dalvik_hook_t *) cerca(L, hash);
	strcpy(p,res->method_sig);

	char *s = parse_signature(p);
	char *pointer = s;
	j = is_string_class(pointer);

	if(j)
		flag = 1;

	dalvik_prepare(&d, res, env);
	log("trovato = 0x%x, il metodo: @0x%x,  type = %c\n", res, res->method,   *s)

	if( *s == 'L'){
		log("CLASSE!!\n")
		str = (*env)->CallObjectMethod(env, obj, res->mid); //XXX non mi ritorna il valore
		if(!str)
			log("PORCODIO NIENTE VALORE DI RITORNO\n")
		log("success calling : %s, at = 0x%x, mid = 0x%x\n", res->method_name, obj, res->mid)
	}
	else if (*s == 'V'){
		log("VOID\n")
		(*env)->CallVoidMethod(env, obj, res->mid);
		log("success calling : %s\n", res->method_name)
		dalvik_postcall(&d, res);
		pthread_mutex_unlock(&mutex);
		log("RILASCIATO LOCK!\n")
		return 0;
	}
	else if(*s == 'Z'){
		log("BOOOLEAN\n") //boolean is always true :)
		dalvik_postcall(&d, res);
		pthread_mutex_unlock(&mutex);
		log("RILASCIATO LOCK!\n")
		return 1;
	}
	else{
		log("default \n")
		(*env)->CallVoidMethod(env, obj, res->mid);
		log("success calling : %s\n", res->method_name)
		dalvik_postcall(&d, res);
		pthread_mutex_unlock(&mutex);
		log("RILASCIATO LOCK!\n")
		return (void*)0;

	}

	dalvik_postcall(&d, res);

	if (flag) {
		log("FLAG TRUE\n")
		if(str){
			char *s = (*env)->GetStringUTFChars(env, str, 0);
			if (s) {
				log("OTTENUTA STRINGA = %s\n", s)
				(*env)->ReleaseStringUTFChars(env, str, s); 
			}
			
			pthread_mutex_unlock(&mutex);
			log("RILASCIATO LOCK!\n")
			log("----------------------------------------------------\n")
			return str;
		}	
		else{
			log("------------------- STR NULL END---------------------------------\n")
			pthread_mutex_unlock(&mutex);
			log("RILASCIATO LOCK!\n")
			log("----------------------------------------------------\n")
			return str;
		}
	}
	else{
		
		pthread_mutex_unlock(&mutex);
		log("RILASCIATO LOCK!\n")
		log("----------------------------------------------------\n")
	}	
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



	struct dalvik_hook_t *dh;

	jclass cls = (*env)->FindClass(env, "org/tesi/fakecontext/DalvikHook");

	jmethodID clname = (*env)->GetMethodID(env,cls,"get_clname","()Ljava/lang/String;");
	jmethodID method_name = (*env)->GetMethodID(env,cls,"get_method_name","()Ljava/lang/String;");
	jmethodID method_sig = (*env)->GetMethodID(env,cls,"get_method_sig","()Ljava/lang/String;");
	jmethodID dexpath= (*env)->GetMethodID(env,cls,"get_dexpath","()Ljava/lang/String;");
	jmethodID hashid = (*env)->GetMethodID(env,cls,"get_hashvalue","()Ljava/lang/String;");
	jmethodID argsid = (*env)->GetMethodID(env,cls,"get_myargs","()I");
	jmethodID nsid = (*env)->GetMethodID(env,cls,"getNs","()I");
	jmethodID sid = (*env)->GetMethodID(env,cls,"isSkip","()I");

	void * c = (*env)->CallObjectMethod(env,clazz,clname);
	void * c2 = (*env)->CallObjectMethod(env,clazz,method_name);
	void* c3 = (*env)->CallObjectMethod(env,clazz,method_sig);
	void* c4 = (*env)->CallObjectMethod(env,clazz,dexpath);
	void *c5 = (*env)->CallObjectMethod(env,clazz,hashid);
	jint args = (*env)->CallIntMethod(env,clazz,argsid);
	jint ns = (*env)->CallIntMethod(env,clazz,nsid);
	jint flag = (*env)->CallIntMethod(env,clazz,sid);
	log("class: 0x%x, mid: 0x%X, string: 0x%x, list = 0x%x, argsid = 0x%x, jint = %d\n",cls,dexpath,c, &L, argsid, args)



	char *s = (*env)->GetStringUTFChars(env, c , NULL);
	char *s2 = (*env)->GetStringUTFChars(env, c2 , NULL);
	char *s3 = (*env)->GetStringUTFChars(env, c3 , NULL);
	char *s4 = (*env)->GetStringUTFChars(env, c4 , NULL);
	char *s5 = (*env)->GetStringUTFChars(env,c5,NULL);

	
	dh = (struct dalvik_hook_t *)malloc(sizeof(struct dalvik_hook_t));
	dh->real_args = args;
	dh->skip = flag;

	log("chiamato con: %s, %s, %s, %s , %s, args vale = %d, ns = %d\n", s,s2,s3,s4,s5, args, ns)
	int i = inserisci(&L,dh,s,s2,s3,s5);
	switch(args){
		case 0:
			log("CHIAMO WRAPPER 00000000000000000000000000000000\n")
			if(!dalvik_hook_setup(dh, s,s2,s3,ns,wrapper_0))
				log("----------- HOOK SETUP FALLITO ------------- : %s, %s\n", c,c2)
			if(!dalvik_hook(&d,dh))
				log("----------- HOOK FALLITO ------------- : %s, %s\n", c,c2)
			log("HOOK PLACED0\n")
			break;
		default:
			log("DEFAULT!!!!!!!!!\n")
			if(!dalvik_hook_setup(dh,s,s2,s3,ns,onetoall))
				log("----------- HOOK SETUP FALLITO ------------- : %s, %s\n", c,c2)
			if(!dalvik_hook(&d, dh))
				log("----------- HOOK FALLITO ------------- : %s, %s\n", c,c2)
			log("HOOK PLACED1\n")
			break;
	}
	log("stampo lista: %d\n", i)
	//stampa_lista(L);
	pthread_mutex_unlock(&mutex);
}

static void load_dex_init(){
	
	log("-------------------DENTRO LOAD DEX INIT ---------------------\n")
	//dalvik_dump_class(&d,"Lorg/tesi/fakecontext/FakeContext;");
	int cookie = dexstuff_loaddex(&d, "/data/local/tmp/classes.dex");
	void *clazz = dexstuff_defineclass(&d, "org/tesi/fakecontext/LoggerWrapper", cookie);
	void *clazz2 = dexstuff_defineclass(&d, "org/tesi/fakecontext/DalvikHook", cookie);
	void *clazz3 = dexstuff_defineclass(&d, "org/tesi/Hooks/HookList", cookie);
	void *clazz4 = dexstuff_defineclass(&d, "org/tesi/fakecontext/MyInit", cookie);
	//dalvik_dump_class(&d, "Lorg/tesi/fakecontext/DalvikHook;");

	//void *clazz2 = dexstuff_defineclass(&d, "org/tesi/fakecontext/MessangerService", cookie);
	
	//myfind_loaded_class(&d,"Lorg/tesi/fakecontext/FakeContext;", "<init>","(Landroid/content/Intent;)V");
	//dalvik_dump_class(&d,"Lorg/tesi/fakecontext/FakeContext;");
	JNIEnv *env = (JNIEnv*) get_jni_for_thread(&d);
	get_info(env);
	log("CLAZZ1: 0x%x, CLAZZ2: 0x%x, CLAZZ3 = 0x%x, CLAZZ4 = 0x%x\n", clazz, clazz2, clazz3, clazz4)
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
		log("DEBUGGGGG: new obj = 0x%x, cls= 0x%x, mid = 0x%x\n", obj, mycls, start_magic)
		if(mycls && obj && start_magic){
			myl.cls = (jclass) (*env)->NewGlobalRef(env,mycls);
			myl.eid = start_magic;
			myl.mid = constructor;
			myl.obj = (jobject) (*env)->NewGlobalRef(env,obj);
		}
	} 
	log("------------------- FINE LOAD DEX INIT ---------------------\n")

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

static void mystart(JNIEnv *env, jobject thiz, jobject intent){
	log("DENTRO START ACTIVITY\n")
	jvalue args[1];
	args[0].l = intent;
	dalvik_prepare(&d, &sb1, env);
	(*env)->CallVoidMethodA(env, thiz, sb1.mid, args);
	dalvik_postcall(&d, &sb1);
	log("FINE START ACTIVITY\n")
}

static void* myset(JNIEnv *env,jobject thiz, ...){

	log("DENTRO MY SET!!\n")
	va_list l;
	jboolean op;
	jobjectArray ja;
	va_start(l,thiz);
	/**
	jvalue args[2];
	int i;
	
	ja = (*env)->NewObjectArray(env,1, (*env)->FindClass(env,"java/lang/Object"), 0);
	log("DIOPORCOOO ::: 0x%x, 0x%x , 0x%x\n", ja, o1, o2)
	log("1\n")
	(*env)->SetObjectArrayElement(env, ja, 0,thiz);
	log("2\n")
	args[0].l = (*env)->GetObjectArrayElement(env, ja, 0);

	log("prima del for\n")
	
	args[1].l = o2;
	*/
	/**
	for(i=0;i<2;i++){
		log("1\n")
		(*env)->SetObjectArrayElement(env, ja, i, va_arg(l , void *)); //problemi con argomenti native type
		
		op = (jboolean)(*env)->GetObjectArrayElement(env, ja, i);
		args[i].l = op;
	}
	*/
	log("dopo for\n")
	dalvik_prepare(&d,&sb1,env);
	void *res = (*env)->CallBooleanMethodV (env,thiz, sb1.mid, l);
	dalvik_postcall(&d,&sb1);
	log("------------------- FINE LOAD DEX INIT ---------------------\n")
	return res;

}
void do_patch()
{
  log("do_patch_mon()\n")
  dalvik_hook_setup(&sb1,"Ljava/io/File;", "setWritable", "(ZZ)Z", 3, myset);
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

	

	//dalvik_dump_class(&d,"");
	// insert hooks
	//do_patch();
	
	// call dump class (demo)

   //dalvik_dump_class(&d,"Landroid/database/sqlite/SQLiteDatabase;");
  
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
	debug = 0;
	/**
	myl.mid = 0;
	myl.cls = 0;
	myl.obj = 0;
	*/
 	// set log function for  libbase (very important!)
	set_logfunction(my_log);
	// set log function for libdalvikhook (very important!)
	dalvikhook_set_logfunction(my_log);

    hook(&eph, getpid(), "libc.", "epoll_wait", my_epoll_wait, 0);
}