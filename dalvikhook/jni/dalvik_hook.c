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

//lib per intercettare i crash nativi
#include "coffeecatch.h"


#define APICLASS "Lorg/sid"
#define HOOKCLASS "Lorg/tesi"
#define APICLASS2 "org.sid"
#define HOOKCLASS2 "org.tesi"
#define DALVIKHOOKCLS "org/sid/addi/core/DalvikHook" //used by env->findclass()
#define DALVIKHOOKCLS_2 "Lorg/sid/addi/core/DalvikHook;"
#define LOADER"/data/local/tmp/loader.dex"
#define APIFILE "/data/local/tmp/apiclasses.dex"


struct dexstuff_t d;
lista L; //dalvikhook list
pthread_mutex_t mutex; //list mutex
static int debug = 1;
static int cookie = NULL; 
pthread_t pty_t;
char working_dir[256] = {0};
static JavaVM* g_JavaVM = NULL;
static int cookie2 = NULL;
struct ClassObject* dalvikHookCls=NULL;
jclass stackelem = NULL;
jclass th = NULL;

/**
	Inizializza la struttura dalvik_hook_t con i dati relativi al metodo da hookare
*/
//int dalvik_hook_setup(struct dalvik_hook_t *h, char *cls, char *meth, char *sig, int ns, void *func)
int dalvik_hook_setup(struct dalvik_hook_t *h, char *cls, char *meth, char *sig,  void *func)
{
	if (!h)
		return EXIT_FAILURE;

	pthread_mutex_init(&h->mutexh, NULL);
	strcpy(h->clname, cls);
	
	strncpy(h->clnamep, cls+1, strlen(cls)-2);
	h->clnamep[strlen(cls)-2] = '\0';

	strcpy(h->method_name, meth);
	strcpy(h->method_sig, sig);
	h->n_iss = 0; //ns
	h->n_rss = 0; //ns
	h->n_oss = 0;
	h->native_func = func;

	h->sm = 0; // set by hand if needed

	h->af = 0x0100; // native, modify by hand if needed
	
	h->resolvm = 0; // don't resolve method on-the-fly, change by hand if needed

	h->debug_me = 1;
	h->dump = 0;
	return EXIT_SUCCESS;
}

/**
	Imposta l'hook del metodo
*/
int dalvik_hook(struct dexstuff_t *dex, struct dalvik_hook_t *h)
{
	jthrowable exc;
	//cerco se la class è in memoria
	struct ClassObject* target_cls = dex->dvmFindLoadedClass_fnPtr(h->clname); //return classobject*
	// in caso negativo fail
	if (!target_cls) {
		if (h->debug_me)		
			log("ERROR non ho trovato la classe: %s\n", h->clname)
		return EXIT_FAILURE;
	}
	if(h->debug_me){
		log("DENTRO DALVIK HOOK MAIN, cerco classe = %s\n",h->clname)
		log("dalvik_hook: class %s\n", h->clname)
		log("class = 0x%x, classLoaader = %p\n", target_cls, target_cls->classLoader)
	}	
	// print class in logcat
	if (h->dump && dex && target_cls)
		dex->dvmDumpClass_fnPtr(target_cls, (void*)1);
	//cerco il metodo da hookare tra i  metodi non-diretti
	h->method = dex->dvmFindVirtualMethodHierByDescriptor_fnPtr(target_cls, h->method_name, h->method_sig);
	if (h->method == 0) {
		if(h->debug_me)
			log("calling dvmFindDirectMethodByDescriptor_fnPtr\n");
		//cerco il metodo target tra i metodi diretti
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
		if (h->debug_me)
			log("DALVIK HOOK COPIO METHOD %p %s\n", h->method, h->method_name)
		//copio il metodo originale
		struct Method* cippa = (struct Method*) dex->dvmLinearAlloc_fnPtr(((struct ClassObject*)target_cls)->classLoader,sizeof(struct Method));//(struct Method*)calloc(1,sizeof(struct Method));
		memcpy( cippa, h->method, sizeof(struct Method));
		//aggiungo il metodo appena copiato nella struttura dalvil_hook_t
		h->originalMethod = cippa;
		//impostato a 1 se il metodo target e' statico
		h->sm = is_static(dex, h->method);
		//bytecode del metodo
		h->insns = h->method->insns;
		if (h->debug_me) {
			log("DALVIK_HOOK h->method->nativeFunc 0x%x, h->nativefunc: 0x%x\n", h->method->nativeFunc, h->native_func)		
			log("DALVIK_HOOK insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->method->insSize, h->method->registersSize, h->method->outsSize)
		}
		//imposto valori di grandezza registri, argomenti in ingresso, etc..
		h->iss = h->method->insSize;
		h->rss = h->method->registersSize;
		h->oss = h->method->outsSize;
		
		//uso una funzione dalvik per calcolare la grandezza  degli argomenti del metodo
		int res = dex->dvmComputeMethodArgsSize_fnPtr(h->method);

		if (h->debug_me) {
			log("shorty %s\n", h->method->shorty)
			log("name %s\n", h->method->name)
			log("arginfo %x\n", h->method->jniArgInfo)
			log("#args = %d\n", res)
		}

		h->method->a = h->method->a | h->af; // make method native
		if (h->debug_me)
			log("access %x\n", h->method->a)
		if (h->debug_me) {
			log("DALVIK_HOOK nativeFunc %p, dalvin_hook_t: %p, method->insns = %p\n", h->method->nativeFunc, h->native_func, h->method->insns)		
			log("DALVIK_HOOK insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->method->insSize, h->method->registersSize, h->method->outsSize)
			log("DALVIK HOOK ORIGINAL nativeFunc %p,  method->insns = %p\n", h->originalMethod->nativeFunc, h->originalMethod->insns)		
			log("DALVIK HOOK ORIGINAL insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->originalMethod->insSize, h->originalMethod->registersSize, h->originalMethod->outsSize)
		}
		//imposto la funzione nativa
		h->method->nativeFunc = h->native_func;
		h->method->insns = (const u2*)  cippa;
		h->method->registersSize = cippa->insSize;
		h->method->outsSize = 0;
		h->n_iss  = res;

		if (h->debug_me){
			log("METODO CON %d ARGOMENTI \n", res)
			log("DALVIK_HOOK nativeFunc %p, dalvin_hook_t: %p, method->insns = %p\n", h->method->nativeFunc, h->native_func, h->method->insns)		
			log("DALVIK_HOOK insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->method->insSize, h->method->registersSize, h->method->outsSize)
			log("DALVIK HOOK ORIGINAL nativeFunc %p,  method->insns = %p\n", h->originalMethod->nativeFunc, h->originalMethod->insns)		
			log("DALVIK HOOK ORIGINAL insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->originalMethod->insSize, h->originalMethod->registersSize, h->originalMethod->outsSize)
			log("patched %s to: 0x%x\n", h->method_name, h->native_func)
			log("DALVIK_HOOK nativeFunc 0x%x, dalvin_hook_t: 0x%x  method->insns = %p\n", h->method->nativeFunc, h->native_func, h->method->insns)		
			log("DALVIK_HOOK insSize = 0x%x  registersSize = 0x%x  outsSize = 0x%x\n", h->method->insSize, h->method->registersSize, h->method->outsSize)
		}		
	}
	else {
		if (h->debug_me)
			log("could NOT patch %s\n", h->method_name)
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


/*
 * Return a new Object[] array with the contents of "args".  We determine
 * the number and types of values in "args" based on the method signature.
 * Primitive types are boxed.
 *
 * Returns NULL if the method takes no arguments.
 *
 * The caller must call dvmReleaseTrackedAlloc() on the return value.
 *
 * On failure, returns with an appropriate exception raised.
 */
static struct ArrayObject* boxMethodArgs(struct Method* method, const u4* args)
{
    const char* desc = &method->shorty[1]; // [0] is the return type.

    /* count args */
    size_t argCount = d.dexProtoGetParameterCount_fnPtr(&method->prototype);
    ClassObject* coa = d.dvmFindSystemClass_fnPtr("[Ljava/lang/Object;");
    //(ClassObject*) ((struct DvmGlobals)d.gDvm).classJavaLangObjectArray;
    /* allocate storage */
    struct ArrayObject* argArray = d.dvmAllocArrayByClass_fnPtr( coa,
        argCount, ALLOC_DEFAULT);
    if (argArray == NULL)
        return NULL;
    struct Object** argObjects = (struct Object**)(void*)argArray->contents;

    /*
     * Fill in the array.
     */

    size_t srcIndex = 0;
    size_t dstIndex = 0;
    while (*desc != '\0') {
        char descChar = *(desc++);
        JValue value;

        switch (descChar) {
        case 'Z':
        case 'C':
        case 'F':
        case 'B':
        case 'S':
        case 'I':
            value.i = args[srcIndex++];
            argObjects[dstIndex] = (struct Object*) d.dvmBoxPrimitive_fnPtr(value,
                d.dvmFindPrimitiveClass_fnPtr(descChar));
            /* argObjects is tracked, don't need to hold this too */
            d.dvmReleaseTrackedAlloc_fnPtr(argObjects[dstIndex], NULL);
            dstIndex++;
            break;
        case 'D':
        case 'J':
            value.j = d.dvmGetArgLong_fnPtr(args, srcIndex);
            srcIndex += 2;
            argObjects[dstIndex] = (struct Object*) d.dvmBoxPrimitive_fnPtr(value,
                d.dvmFindPrimitiveClass_fnPtr(descChar));
            d.dvmReleaseTrackedAlloc_fnPtr(argObjects[dstIndex], NULL);
            dstIndex++;
            break;
        case '[':
        case 'L':
            argObjects[dstIndex++] = (Object*) args[srcIndex++];
            break;
        }
    }

    return argArray;
}


//static void __attribute__ ((constructor)) dalvikhook_my_init(void);

static char logfile[256] = "/data/local/tmp/dynsec";

void logmsgtofile(char *msg)
{
	/*
    int fp = open(logfile, O_WRONLY|O_APPEND);
    write(fp, msg, strlen(msg));
    close(fp);
    */
    FILE* fp = fopen(logfile, "a+");
    fprintf(fp,"%s",msg);
    fclose(fp);
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




/**
	Funzione che esegue il codice di controllo associato all'hook

	TODO: 
	-segare via margs
	-gestire exception

	@env
	@original: puntatore all'oggetto che rappresenta il metodo originale (senza hook)
	@thiz: eventuale puntatore al this
	@margs
	@dh
	@self
	@oldargs
*/
struct Object* eseguiDex2(JNIEnv*env,struct Method* original, struct Object* thiz, JValue* margs, struct dalvik_hook_t* dh, Thread* self, u4* oldargs){
	//return;
	if(thiz!=NULL){
		oldargs++;
	}
	int real_args = dh->n_iss;//_dvmComputeMethodArgsSize(&d,res->method);
	JValue result;
	jclass clazz;;
	struct ClassObject* pCo;
	//costruisco oggetto [Ljava/lang/Object con gli argomenti oldargs
	struct ArrayObject* pAo = boxMethodArgs(original, oldargs);
	//prendo il riferimento all'oggetto DalvikHook contenuto nella struttura degli hook
	struct Object* oo = d.dvmDecodeIndirectRef_fnPtr(self, dh->ref);
	if(debug){
		log(" CONTROLLO thiz = %p, self = %p, original = %p , dh = %p , margs = %p \n", thiz, self, original, dh, margs)
		log("ESEGUIDEX2 metodo %s ha %d argomenti, margs = %p, margs0 = %p\n", original->name, real_args, margs, margs->l)
		log("ESEGUIDEX2 decodificata ref = %p, risultato = %p \n", dh->ref, oo)
		log("ESEGUIDEX2 ho un dex method da chiamare = %s!!!\n", dh->dex_meth)
	}
	//se esiste il this per il metodo lo prendo e lo salvo
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
	// inizializzo l'oggetto DalvikHook con i valori  dell'hook
	struct Method* initid = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(dh->DexHookCls, "initFunc" , "()V");
	d.dvmCallMethodA_fnPtr(self,initid,oo,false,&result, NULL);

	//cerco l'entry point del codice DEX
	struct Method* dex_meth = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(dh->DexHookCls, "myexecute" , "([Ljava/lang/Object;)V");
	//struct Method* dex_meth = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(dh->DexHookCls, "myexecute" , "()V");
	if(debug)
		log("ESEGUIDEX2 trovato myexecute %p \n", dex_meth)

	JValue* dexargs;
	dexargs = calloc(1, sizeof(JValue));
	//dexargs->l = mioArray;
	dexargs->l = pAo;
	JValue unused;
	//chiamo l'entry point del DEX, "myexecute"
	d.dvmCallMethodA_fnPtr(self,dex_meth,oo,false,&unused, dexargs );	
	if(unused.l != NULL){
		if(debug)
			log("XXX8 unusedvale: %p \n", unused.l)
		return unused.l;
	}
	else{
		log("!!!!!!!!!!!!!!unused.l NULLLL\n")
	}
EXIT:
	d.dvmReleaseTrackedAlloc_fnPtr(pAo, self);
	free(dexargs);

}

void ArrayObjectHelper(void* aobj){
	log("DENTRO ARRAY OBEJCT HELPER\n")
	struct ArrayObject* pAo = aobj;
	int i = 0;
	char* c = NULL;
	log("array lungo: %d\n", pAo->length)
	struct Object** argsArray = (struct Object**)(void*)pAo->contents;
	for(i=0; i<pAo->length; i++){
		log("pAo[%d]: %s\n",i, argsArray[i]->clazz->descriptor)
		c = d.dvmCreateCstrFromString_fnPtr((struct StringObject*)argsArray[i]);
		if(c != NULL){
			log("HELPER OTTENUTO: %s\n", c)
		}
	}
}

/**
	Funzione che esegue la chiamata al metodo originale

	TODO:
	-gestire exception


	@original: pointer al metodo originale
	@thiz
	@self
	@pResult
	@margs
*/
static int callOriginalV2(struct Method* original, struct Object* thiz, Thread* self, JValue* pResult, JValue* margs){
	JValue result;
	void *old = d.dvmChangeStatus_fnPtr(self, THREAD_RUNNING);
	log("callOriginalV2 CHIAMO DVMCALLMETHODA = %s, self = %p,original = %p, result = %p, myargs = %p\n", original->name, self,original->insns,pResult,margs)
	log("original souldtrace: %d\n", original->shouldTrace)	
	d.dvmCallMethodA_fnPtr(self,original,thiz,false,&result, margs);
	// exceptions are thrown to the caller with a return!
    if (d.dvmCheckException_fnPtr(self)) {
    	log("ERROR callOriginalV2 TROVATA EXCEPTION!!! \n")
    	Object* excep = d.dvmGetException_fnPtr(self);
    	if(excep != NULL){
    		log("XXX8 dio cane presa exception %p desc: %s\n", excep, excep->clazz->descriptor)
    		/**
    		struct ArrayObject* pAo = NULL;//d.dvmGetMethodThrows_fnPtr(original); //getmethodthrows se ne va a male
    		log("XXX8 dopo dvmgetmethodthrows\n")
    		if( pAo != NULL){
    			log("XXX8 ARRAYOBJECT EXCEPTION NOT NULL! \n")
				log("diosolo arrayobj length = %d\n", pAo->length)
				struct Object** argsArray = (struct Object**)(void*)pAo->contents;
				size_t i = 0;
				char* c= NULL;
			    for (i = 0; i < pAo->length; ++i) {
			        //log("pos[%d] = %s \n",i,argsArray[i]->clazz->descriptor);
			        //log("diosolo provo a ottenere char*\n")
			        c = d.dvmCreateCstrFromString_fnPtr((struct StringObject*)argsArray[i]);
			        log("diosolo [%d] preso char = %s\n", i, c)
			    }
    		}
    		else{
    			log("XXX8 ARRAYOBJECT EXCEPTION NULL! \n")
    		}
    		//void* str = d.dvmHumanReadableDescriptor_fnPtr(excep->clazz->descriptor);
    		//log("XXX8 exception name: %s\n", (char*)str);
    		
	    	//d.dvmWrapException_fnPtr("Ljava/lang/Exception;");
	    	//log("XXX8 dopo wrap exception \n")
	    	*/
	    	if( strcmp(excep->clazz->descriptor, "Ljava/lang/ClassNotFoundException;")){
	    		log("XXX8 lancio class not found exception\n")
	    		d.dvmThrowClassNotFoundException_fnPtr("Class Not found!!");
	    	}else{
	    		;
	    	}
	    	//d.dvmThrowFileNotFoundException_fnPtr("PORCODIOOOOOO");
	    	d.dvmChangeStatus_fnPtr(self, old);
	        return -1;
        }

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
    d.dvmChangeStatus_fnPtr(self, old);
}


int checkCaller(JNIEnv *env){

	
	jmethodID toS = (*env)->GetMethodID(env, stackelem, "toString", "()Ljava/lang/String;");
	
	//log("trovata clase thread: %p\n", th)
	jmethodID currid = (*env)->GetStaticMethodID(env, th, "currentThread", "()Ljava/lang/Thread;");
	//log("trovato mid: %p\n", currid)
	jobject cth = (*env)->CallStaticObjectMethod(env, th, currid);
	//log("trovata static m: %p\n", cth)
	jmethodID getST = (*env)->GetMethodID(env, th, "getStackTrace", "()[Ljava/lang/StackTraceElement;");
	//log("trovata m2: %p\n", getST)
	jobject arr = (*env)->CallObjectMethod(env, cth, getST);
	jsize len = (*env)->GetArrayLength(env, arr);
	int i = 0;
	//log("trovato array: %p, di len: %d\n", arr, len)
	jobject tmp;
	
	for(i=0; i < len; i++){
		tmp = (*env)->GetObjectArrayElement(env,arr,i);
		//log("preso elem[%d]: %p\n", i, tmp)
		jstring str = (*env)->CallObjectMethod(env, tmp, toS);
		//log("chiamo printstring con: %p\n", str)
		const char *buf = (*env)->GetStringUTFChars(env, str, 0);
		//log("preso buf: %s\n",buf)
		if(strstr(buf, HOOKCLASS2) != NULL || strstr(buf, APICLASS2) != NULL ){
			//log("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA CHIAMATO DA DIOOO\n")
			return 1;
		}
		(*env)->ReleaseStringUTFChars(env, str, buf);
		(*env)->DeleteLocalRef(env,tmp);
		(*env)->DeleteLocalRef(env,str);
		//printString(env, "valore: ", str);
	}
	return 0;
}
/*
	Funzione ausiliaria per copiare i dex tra le diverse istanze
*/
char* copyDexs(){
	log("dentro copy dex\n")
	FILE *exein, *exeout;

	exein = fopen(APIFILE, "rb");
	if (exein == NULL) {
	    /* handle error */
	    //perror("file open for reading");
		log("file open for reading\n");
		exit(EXIT_FAILURE);
	}
	char *fout = malloc(sizeof(char)*128);
	sprintf(fout, "%s/classes.dex", working_dir);
	exeout = fopen(fout, "wb");
	if (exeout == NULL) {
	    /* handle error */
	    //perror("file open for writing");
		log("file open for writing");
		exit(EXIT_FAILURE);
	}
	size_t n, m;
	unsigned char buff[8192];
	do {
		n = fread(buff, 1, sizeof buff, exein);
		if (n) m = fwrite(buff, 1, n, exeout);
		else   m = 0;
	} while ((n > 0) && (n == m));
	if (m) perror("copy");

	if (fclose(exeout)) perror("close output file");
	if (fclose(exein)) perror("close input file");
	return fout;
}
/**
	Entry point di tutti i metodi hookati.
	http://androidxref.com/4.4.2_r2/xref/dalvik/vm/Jni.cpp#dvmCallJNIMethod
*/
void* onetoall2(u4* args, JValue* pResult, struct Method* method, struct Thread* self){
	//COFFEE_TRY() {

	struct dalvik_hook_t *res;
	jthrowable exc;
	//recupero il metodo originale
	struct Method* original =(struct Method*) method->insns;
	//ottengo JNIEnv*
	JNIEnv *env = (JNIEnv*) get_jni_for_thread(&d);

	if(debug){
		log("DENTRO ONETOALL2\n")
		log("sono stato chiamato da original name %s, insns = %p, native = %p \n", original->name, original->insns, original->nativeFunc)
		log("sono stato chiamato da hooked name %s, insns = %p, native = %p\n", method->name, method->insns, method->nativeFunc);
		log("self vale: %p, env: %p\n", self,env)
	}

	//cambio lo status del Thread e salvo il vecchio
	void *old = d.dvmChangeStatus_fnPtr(self, THREAD_NATIVE);
	log("old status: %d\n", old)

/*	Per ora ho un metodo migliore, checkCaller()

	//controllo lo stack per ottenere i chiamanti del metodo
	struct ClassObject* co = _getCallerClass(&d);
	struct ClassObject* co2 = _getCaller2Class(&d);
	struct ClassObject* co3 = _getCaller3Class(&d);
	if(debug){
		if(co)
			log("XXX6 GETCALLERCLASS = 0x%x, name = %s \n", co, co->descriptor)
		if(co2)
			log("XXX6 GETCALLERCLASS = 0x%x, name = %s \n", co2, co2->descriptor)
		if(co3)
			log("XXX6 GETCALLERCLASS = 0x%x, name = %s \n", co3, co3->descriptor)
	}

	//debug code
	bool check = d.dvmIsNativeMethod_fnPtr(original);
	bool check2 = d.dvmIsNativeMethod_fnPtr(method);
	if(debug){
		log("ONETOALL2 ORIGINAL METHOD: %s, is native = %d \n",original->name, check)
		log("ONETOALL2 HOOKED METHOD: %s, is native = %d \n",method->name, check2)
	}
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
	
	//ottengo il metodo chiamato e creo l'hash da cercare nella lista
	get_method(&d,name,descriptor);
	strcpy(hash,name);
	strcat(hash,descriptor);

	//certo nella lista codivisa l'hash del metodo hookato
	pthread_mutex_lock(&mutex);
	if(debug)
		log("CERCO NELLA LISTA : %s\n", hash)
	res = (struct dalvik_hook_t *) cerca(L, hash);
	pthread_mutex_unlock(&mutex);
	if(debug)
		log("preso res = %p, name = %s\n", res, res->clname)
	int len = res->n_iss;
	
	//descriptor del metodo senza il return type
	const char* desc = &original->shorty[1]; //0 is return type
	struct Object* thiz = NULL;
	size_t srcIndex = 0;
	size_t dstIndex = 0;

	//se il metodo non e' statico recupero il thiz
	if(!is_static(&d, original)){
		if(debug)
			log("ONETOALL2 PRENDO THIZ = %p \n", args[0])
		thiz = (struct Object*)args[0];
		srcIndex++;
	}

	JValue* margs;
	margs = calloc(len, sizeof(JValue));
	if(debug)
		log("ONETOALL2 metodo statico = %d \n", is_static(&d, original))

	int k =0 ;
	//loop sugli argomenti
	while(*desc != '\0'){
		char descChar = *(desc++);
		switch(descChar){
			case 'Z':
			case 'C':
			case 'F':
			case 'B':
			case 'S':
			case 'I':
				margs[k].i = args[srcIndex];
				if(debug)
					log("dentro I, %d, %d,srcIndex = %d args = %p \n", margs[k].i, args[srcIndex], srcIndex, args[srcIndex])
				k++; srcIndex++;
				break;
			case 'D':
			case 'J':
				if(debug)
					log("dentro J, k = %d, srcindex = %d, args = %p\n",k,srcIndex, args[srcIndex])
				memcpy(&margs[k++].j, &args[srcIndex], 8);
				srcIndex += 2;
				break;
			case '[':
			case 'L':
				if(debug)
					log("dentro L, k = %d, srcindex = %d, args = %p\n",k,srcIndex, args[srcIndex])
				margs[k].l = args[srcIndex];
				k++; srcIndex++;
				break;
			default:
				log("ONETOALL2 DESCCHAR NON RICONOSCIUTO %c\n", descChar)
				srcIndex++;
		}
	}
    int flag = checkCaller(env);
	if(flag){
		log("XXX7 SONO STATO CHIAMATO DAL FRAMEWORK\n")
		callOriginalV2(original, thiz, self, pResult, margs);
		log("ONETOALL2 FROM FRAMEWORK FINITO!!!\n")
		goto SAFE;
	}
    /*
    //se il metodo e' stato chiamato dal Framework devo eseguire la chiamata originale e terminare
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
			callOriginalV2(original, thiz, self, pResult, margs);
			log("ONETOALL2 FROM FRAMEWORK FINITO!!!\n")
			goto SAFE;
		}
	}
	*/
	struct Object* tmp = NULL;
	//switch sui diversi tipi di Hook
	switch(res->type){
		case NORMAL_HOOK: 
			//prima eseguo il DEX
			if(strlen(res->dex_meth) > 0){
				d.dvmChangeStatus_fnPtr(self, THREAD_RUNNING);
				eseguiDex2(env, original, thiz, margs, res, self, args);
				d.dvmChangeStatus_fnPtr(self, THREAD_NATIVE);
			}
			break;
		case TRUE_HOOK:
			pResult->z = true;
			goto SAFE;
		case FALSE_HOOK:
			pResult->z = false;
			goto SAFE;
		case NOX_HOOK:
			if(strlen(res->dex_meth) > 0){
				d.dvmChangeStatus_fnPtr(self, THREAD_RUNNING);
				tmp = eseguiDex2(env, original, thiz, margs, res, self, args);
				d.dvmChangeStatus_fnPtr(self, THREAD_NATIVE);
			}
			if(tmp != NULL){
				log("XXX8 valore di ritorno DEX non NULLO: %p\n", tmp)
				pResult->l = tmp;
				goto SAFE;
			}
			else{
				log("XXX8 valore di ritorno DEX NULLO\n")
				goto SAFE;
			}
			break;
		case AFTER_HOOK:
			/*
			exc = (*env)->ExceptionOccurred(env);
	  		if (exc) {
	  			log("ERROR ONETOALL2  CATTURA EXCEPTION\n")
	  			(*env)->ExceptionDescribe(env);
	  			(*env)->ExceptionClear(env);
	  		}
	  		**/
	  		log("AFTER HOOK, original meth = %x, hooked meth = %x \n", original->insns, method->insns)
			if(callOriginalV2(original, thiz, self, pResult, margs) == -1){
				log("ORIGINAL CALL FALLITA\n")
				goto SAFE;
			}
			d.dvmChangeStatus_fnPtr(self, THREAD_RUNNING);
			eseguiDex2(env, original, thiz, margs, res, self, args);
			d.dvmChangeStatus_fnPtr(self, THREAD_NATIVE);
			goto SAFE;	
		case UNUSED_HOOK:
			//hook disabilitato
			break;
		default:
			log("XXX9 ERRORE\n")
			//non dovremmo mai arrivare qui...
			break;
	}
	//chiamata del metodo originale
	callOriginalV2(original, thiz, self, pResult, margs);

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
	free(margs);

	//ripristino il vecchio status del thread
	d.dvmChangeStatus_fnPtr(self, old);
	log("ONETOALL2 FINITO!!!\n")
	return;
}

/* 
	Metodi ausiliari per le chiamate ai metodi Java
*/

void* callStringObjectMethod(Thread* self, Method* m, Object* oo){
	char *c = NULL;
	JValue result;
	void* sobj = d.dvmCallMethodA_fnPtr(self, m, oo, true, &result, NULL );
	if(result.l){
		c = d.dvmCreateCstrFromString_fnPtr( (struct StringObject*) result.l);
	}
	return c;
}
int callIntMethod(Thread* self, Method* m, Object* oo){
	JValue result;
	void* sobj = d.dvmCallMethodA_fnPtr(self, m, oo, false, &result, NULL );
	return result.i;
}

/**
	Funzione richiamata da Java e utilizzata per impostare gli hook
*/
static int _createStruct( JNIEnv* env, jobject thiz, jobject clazz )
{
	pthread_mutex_lock(&mutex);
	struct Thread* self = getSelf(&d);
	struct Object* oo = d.dvmDecodeIndirectRef_fnPtr(self, clazz);
	struct dalvik_hook_t *dh;
	//char **methods = (char *[]){"get_clname:()Ljava/lang/String;", "get_method_name:()Ljava/lang/String;","get_method_sig:()Ljava/lang/String;"};
	char methods[][512] = {"get_clname:()Ljava/lang/String;", "get_method_name:()Ljava/lang/String;","get_method_sig:()Ljava/lang/String;",
								"get_dex_method:()Ljava/lang/String;", "get_dex_class:()Ljava/lang/String;","get_hashvalue:()Ljava/lang/String;",
								"getType:()I", "isSkip:()I"};
	size_t ii = 0;
	char **tokens;
	char *token;

	char values[8][512] = {};

	char *dex_hash;
	char *dex_mname;
	char* dex_cls;
	char *dex_clname;
	char *dex_msig;
	char *dex_meth;
	int dex_type;
	int dex_isSkip;

	Method* clname ;
	Method* mname ;
	Method* msig ;
	Method* dexcls;
	Method* dexm ;
	Method* dexhash ;
	Method* dext;
	Method* dexskip;

	if(debug)
		log("------------ CREATE STRUCT!!! --------  clazz = 0x%x, thiz = 0x%x\n", clazz, thiz)
	
	if(dalvikHookCls){
		 clname = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(dalvikHookCls, "get_clname","()Ljava/lang/String;");
		 mname = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(dalvikHookCls, "get_method_name","()Ljava/lang/String;");
		 msig = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(dalvikHookCls, "get_method_sig","()Ljava/lang/String;");
		 dexcls = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(dalvikHookCls, "get_dex_class","()Ljava/lang/String;");
		 dexm = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(dalvikHookCls, "get_dex_method","()Ljava/lang/String;");
		 dexhash = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(dalvikHookCls, "get_hashvalue","()Ljava/lang/String;");
		 dext = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(dalvikHookCls, "getType","()I");
		 dexskip = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(dalvikHookCls, "isSkip","()I");


		if(clname){
			dex_clname = callStringObjectMethod(self, clname, oo);
			dex_mname = callStringObjectMethod(self, mname, oo);
			dex_msig = callStringObjectMethod(self, msig, oo);
			dex_cls = callStringObjectMethod(self, dexcls, oo);
			dex_meth = callStringObjectMethod(self, dexm, oo);
			dex_hash = callStringObjectMethod(self, dexhash, oo);
			dex_type = callIntMethod(self, dext, oo);
			dex_isSkip = callIntMethod(self, dexskip, oo);
		}
		
		

	}


	dh = (struct dalvik_hook_t *)malloc(sizeof(struct dalvik_hook_t));
	if(!dh){
		log("ERROR malloc dalvik_hook_t!!\n");
		return EXIT_FAILURE;
	}
	dh->type = dex_type;
	dh->loaded = 0;
	dh->ref = (*env)->NewGlobalRef(env, clazz);
	dh->DexHookCls = dalvikHookCls;
	strcpy(dh->dex_meth,"");
	if(strlen(dex_meth) > 0){
		strcpy(dh->dex_meth,dex_meth);
		strcpy(dh->dex_class,dex_cls);	
	}
	char *pointer;
	pointer = parse_signature(dex_msig);	
	int  check;
	if(*pointer == 'F'){
		pthread_mutex_unlock(&mutex);
		return  EXIT_FAILURE;
		//hook a method that return jfloat
	
	}
	else if(*pointer == 'J'){
		pthread_mutex_unlock(&mutex);
		return EXIT_FAILURE;

	}else if(*pointer == 'V'){
		if(dalvik_hook_setup(dh, dex_clname,dex_mname,dex_msig,&onetoall2))
			log("----------- HOOK SETUP FALLITO ------------- : %s, %s\n", clname,mname)
		check =  dalvik_hook(&d, dh);
		log("XXX7  check vale = 0x%x, %d\n",check, check)
		if(check == 0x1){
			log("----------- HOOK VOID FALLITO ------------- : %s, %s\n", dex_clname,dex_mname)
			free(dh);
			pthread_mutex_unlock(&mutex);
			return EXIT_FAILURE;
		}
		else
			log("HOOK VOID PLACED, nome = %s\n", dh->originalMethod->name)			
	}
	else{
		if(dalvik_hook_setup(dh, dex_clname,dex_mname,dex_msig,&onetoall2))
			log("----------- HOOK SETUP FALLITO ------------- : %s, %s\n", clname,mname)
		check =  dalvik_hook(&d, dh);
		log("XXX7  check vale = 0x%x, %d\n",check, check)
		if(check == 0x1){
			log("----------- HOOK FALLITO ------------- : %s, %s\n", dex_clname,dex_mname)
			free(dh);
			pthread_mutex_unlock(&mutex);
			return EXIT_FAILURE;
		}
		else
			log("HOOK PLACED\n")	
		//log("XXX5 class loader = 0x%x\n", dh->method->clazz->classLoader)
	}
	if(debug)
		log("XXXD inserisco dh = 0x%x 0x%x, %s,%s,%s, hash = %s\n", &dh, dh, dex_clname,dex_mname,dex_msig,dex_hash)
	int i = inserisci(&L,dh,dex_clname,dex_mname,dex_msig,dex_hash);
	if(debug)
		log("stampo lista: %d, dex_method = %s\n", i, dh->dex_meth)
	pthread_mutex_unlock(&mutex);
	return EXIT_SUCCESS;
}

/**
	Funzione che crea il thread per mettersi in asccolta sulla PIPE e ricevere i comandi
*/

void* ptyServer(){
	//cazzi amari

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

	//XXX
	jclass mycls2 = (*envLocal)->FindClass(envLocal, "org/sid/addi/core/Session");
	log("XXX5 trovata classe controller = 0x%x\n", mycls2)
	jmethodID constructor2 = (*envLocal)->GetMethodID(envLocal, mycls2, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
	
	
	void *mycls = _dvmFindLoadedClass(&d, "Lorg/sid/addi/core/Session;");
	dalvik_dump_class(&d, "L*/org/sid/addi/core/Session;");
	void *constructor = d.dvmFindDirectMethodHierByDescriptor_fnPtr(mycls, "<init>" , "(Ljava/lang/String;Ljava/lang/String;)V");
	void *start = d.dvmFindVirtualMethodHierByDescriptor_fnPtr(mycls, "start" , "()V");
	log("XXX9 trovato session: %p, <init>: %p, start: %p\n", mycls, constructor,start)
	
	//strcpy(res,"/data/local/tmp/res.fifo");
	//strcpy(req,"/data/local/tmp/req.fifo");
	char *res = malloc(sizeof(char)*256);
	char *req = malloc(sizeof(char)*256);
	sprintf(res,"%s/res.fifo",working_dir);
	sprintf(req,"%s/req.fifo", working_dir);
	log("XXX9 res: %s, req: %s\n", res,req)
	if (mkfifo(res, 0777) == -1) { 
		if (errno!=EEXIST) {
			log("XXX5 pty error %s\n", strerror(errno))
			return;
		}
	}
	if (mkfifo(req, 0777) == -1) { 
		if (errno!=EEXIST) {
			log("XXX5 pty error %s\n", strerror(errno))
			return;
		}
	}
	
	//struct StringObject* jres = d.dvmCreateStringFromCstr_fnPtr(res);
	void* jres  = d.dvmStringFromCStr_fnPtr(res, strlen(res), ALLOC_DEFAULT);
	d.dvmReleaseTrackedAlloc_fnPtr(jres,getSelf(&d));
	//jobject gstr = (*envLocal)->NewGlobalRef(envLocal, jres);	
	void* jreq = d.dvmStringFromCStr_fnPtr(req, strlen(req), ALLOC_DEFAULT);
	d.dvmReleaseTrackedAlloc_fnPtr(jreq,getSelf(&d));
	//jobject gstr1 = (*envLocal)->NewGlobalRef(envLocal, jreq);
	
	//struct StringObject* jreq = d.dvmCreateStringFromCstr_fnPtr(req);
	log("XXX9 creati string obj, jres: %p, jreq: %p\n", jres, jreq)
	jvalue args[2];
	args[0].l = jres;
	args[1].l = jreq;
	JValue result;
	log("xxx9 chiamo metodo\n")
	void *instance = d.dvmAllocObject_fnPtr(mycls,ALLOC_DEFAULT);
	d.dvmReleaseTrackedAlloc_fnPtr(instance,getSelf(&d));
	log("xxx9 allocato object: %p\n", instance)
	d.dvmCallMethodA_fnPtr(getSelf(&d),constructor, instance, false, &result, args);
	log("xxx9 chiamo start\n")
	d.dvmCallMethodA_fnPtr(getSelf(&d),start, instance, false, &result, NULL);
	
	/*
	jobject str = (*envLocal)->NewStringUTF(envLocal, req);
	jobject gstr = (*envLocal)->NewGlobalRef(envLocal, str);
	jobject str1 = (*envLocal)->NewStringUTF(envLocal, res);
	jobject gstr1 = (*envLocal)->NewGlobalRef(envLocal, str1);
	log("XXX5 CHIAMO NEW OBJECT\n")
	jobject mobj = (*envLocal)->NewObject(envLocal, mycls, constructor, gstr, gstr1); 
	jmethodID start = (*envLocal)->GetMethodID(envLocal, mycls, "start", "()V");
	log("XXX5 chiamo metodo su obj = 0x%x, start = 0x%x\n", mobj, start)
	(*envLocal)->CallVoidMethod(envLocal, mobj, start, NULL);
	*/




	(*g_JavaVM)->DetachCurrentThread(g_JavaVM);
	free(res);
	free(req);
	log("XXX5 ESCO DA THREAD\n")
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
	//dalvik_dump_class(&d, "Ljava/lang/VMClassLoader;");
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


/**
	Funzione di prova per manipolare il bytecode a runtime
*/
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

/**
	Funzione per aggiungere hook a runtime

	TODO:
	aggiungere HookType tra gli argomenti
*/
int createAndInsertDhook(JNIEnv *env, char* clsname, char* methodname, char* methodsig){
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
	dh->loaded = 0;
	dh->ok =1;
	dh->ref = NULL;//(*env)->NewGlobalRef(env, clazz);
	dh->type = NORMAL_HOOK;
	strcpy(dh->dex_meth,"");
	char *pointer;
	pointer = parse_signature(methodsig);	
	int  check;
	if(*pointer == 'F'){
		pthread_mutex_unlock(&mutex);
		free(dh);
		return EXIT_FAILURE;
		//hook a method that return jfloat
	}
	else if(*pointer == 'J'){
		free(dh);
		pthread_mutex_unlock(&mutex);
		return EXIT_FAILURE;
	}
	else if(*pointer == 'I'){
		free(dh);
		pthread_mutex_unlock(&mutex);
		return 0;
	}
	else if(*pointer == 'V'){
		if(dalvik_hook_setup(dh, cl, name,descriptor,&onetoall2))
			log("----------- HOOK SETUP FALLITO ------------- : %s, %s\n", clsname, methodname)
		check =  dalvik_hook(&d, dh);
		log("XXX7  check vale = 0x%x, %d\n",check, check)
		if(check == 0x1){
			log("----------- HOOK FALLITO ------------- : %s, %s\n",clsname, methodname)
			free(dh);
			pthread_mutex_unlock(&mutex);
			return EXIT_FAILURE;
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
	free(hash);
	free(cl);
	free(name);
	free(descriptor);
	pthread_mutex_unlock(&mutex);
	return EXIT_SUCCESS;
}


/**
	Wrapper di metodi JNI
*/
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
		log("XXX8 CLASSE NON TROVATA chiamo loadsystemclass con %s, str = %p\n", clsname, str)
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

static void _diosoloW(JNIEnv* env, jobject thiz){
	diosolo(&d,env);
}
/**
	Registro i metodi nativi che sono usati dal mondo  Java
*/
JNINativeMethod method_table[] = {
    { "createStruct", "(Lorg/sid/addi/core/DalvikHook;)I",
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
	{"diosoloW", "()V",
		(void*)_diosoloW},
	{ NULL, NULL, NULL },
};

/* 
OLD STUFF

static void _register_api_native(JNIEnv *env, jclass mycls, jclass test){
	log("dentro register api, args: %p %p\n", mycls, test)
	(*env)->RegisterNatives(env, mycls, method_table, sizeof(method_table) / sizeof(method_table[0]));
	log("fine register api\n")
}


JNINativeMethod bootloader_table[] = {
    { "register_api_native", "(Ljava/lang/Class;)V",
        (void*) _register_api_native },
	{ NULL, NULL, NULL },
};
*/

/* 
	Funzioni ausiliarie per creare l'ambiente di esecuzione del framework
*/
int processIsZygote(pid_t p){
	char fname[256];
	sprintf(fname,"/proc/%d/cmdline",p);
	int fp = open(fname,O_RDONLY);
	if(fp<0){
		return 0;
	}
	read(fp, fname,sizeof(fname));
	close(fp);
	if(strstr(fname,"zygote")){
		return 1;
	}
	else
		return 0;
}

void createWorkingDir(pid_t p){
	struct stat st = {0};
	if(processIsZygote(p)){
		sprintf(working_dir, "/data/local/tmp/dynsec/zygote");
		if (stat(working_dir, &st) == -1) {
		   	mkdir(working_dir, S_IRWXU | S_IRWXG | S_IRWXO);
	   		//chmod(working_dir, j);
		}
		sprintf(logfile,"%s", working_dir);
		strcat(logfile,"/dynsec.log");
	}
	else{
		sprintf(working_dir,"/data/local/tmp/dynsec/%d", p);
		if (stat(working_dir, &st) == -1) {
		   	mkdir(working_dir, S_IRWXU | S_IRWXG | S_IRWXO);
	   		//chmod(working_dir, j);
		}
		sprintf(logfile,"%s", working_dir);
		strcat(logfile,"/dynsec.log");
	}
}

/**
	Funzione richiamata quando viene caricata la libreria dinamica in memoria
	Carica le classi Java che sono utilizzate dal framework
	e richiama la funzione del mondo Java che si occupa di impostare gli hook

	usando i metodi nativi per caricare le classi devo rispettare l'ordine, prima le superclassi poi le subclass...
	http://androidxref.com/4.4.2_r2/xref/dalvik/vm/oo/Class.cpp#dvmLinkClass
*/
jint my_ddi_init(){
	pthread_mutex_init(&mutex, NULL);	
	createWorkingDir(getpid());
	char *dexfile2 = copyDexs();

	if(debug)
		log("-------------------DENTRO  DDI INIT ---------------------\n")
	dexstuff_resolv_dvm(&d);

	//int cookie2 = dexstuff_loaddex(&d, LOADER);
	cookie = dexstuff_loaddex(&d, APIFILE);
	int cookieHooks = dexstuff_loaddex(&d, "/data/local/tmp/classes.dex");
	log("cookie: %p, cookie2: %p\n", cookie, cookieHooks)

	/* new way
	//XXX
	struct ClassObject* pCo = dexstuff_defineclass(&d, "org/tesi/bootloader/BootLoader", cookie2);	
	struct ClassObject* pCo2 = dexstuff_defineclass(&d, "org/sid/addi/core/HookT", cookie);	
	jclass *clazzHookType =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/HookT$HookType", cookie);
	jclass *clazz55 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/StringHelper", cookie);
	jclass *clazz44 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/LogWrapper", cookie);	
	jclass *clazz22 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/DalvikHook", cookie);
	jclass *clazzDEXHook =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/DEXHook", cookie);
	jclass *clazz33 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/manageADDI", cookie);
	jclass *clazz60 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/Common", cookie);
	jclass *clazz61 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/CommandWrapper", cookie);
	jclass *clazz62 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/ArgumentWrapper", cookie);
	jclass *clazz63 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/RequestWrapper", cookie);
	jclass *clazz64 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/Commands", cookie);
	jclass *clazz65 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/Session", cookie);
	jclass *clazz66 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/XML", cookie);
	jclass *clazz333 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/manageADDI", cookie);

	jclass *clazz59 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/Logs/LoggerConfig", cookie);
	jclass *clazz58 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/Logs/LogTraces", cookie);
	jclass *clazz99 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/Logs/LogToSQLiteHelper", cookie);
	jclass *clazz88 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/Logs/LogToSQLiteClass", cookie);
	jclass *clazz77 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/Logs/LogToSQLite", cookie);
	jclass *clazz57 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/Logs/LogToDB", cookie);
	jclass *clazz56 =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/Logs/Logger", cookie);
	jclass *clazz5 = (jclass*) dexstuff_defineclass(&d, "org/sid/addi/utils/AppContextConfig", cookie);
	*/
	struct ClassObject* pCo = dexstuff_defineclass(&d, "org/sid/addi/core/HookT", cookie);	
	jclass *clazzHookType =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/HookT$HookType", cookie);
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
	jclass* billingHook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/BillingHook", cookieHooks);
	jclass* LogHook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/OnlyLogHook", cookieHooks);
	jclass* antiEmuHook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/AntiEmulatorDetectionHook", cookieHooks);
	jclass* networkHook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/NetworkHook", cookieHooks);
	jclass* IPCHook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/IPCHooks", cookieHooks);
	jclass* appC =(jclass*) dexstuff_defineclass(&d, "org/tesi/utils/AppContextConfig", cookieHooks);
	jclass* webViewHook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/WebviewHook", cookieHooks);
	jclass* sqliteHook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/SQLiteHook", cookieHooks);
	jclass* sslhook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/SSLHook", cookieHooks);
	jclass* fshook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/FileSystemHook", cookieHooks);
	jclass* dummyhook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/DummyHook", cookieHooks);
	jclass* sphook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/SharedPrefHook", cookieHooks);
	jclass* cryptohook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/CryptoHook", cookieHooks);
	jclass* hashhook =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/HashHook", cookieHooks);
	jclass* cryptokey =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/HookCryptoKey", cookieHooks);
	jclass *clazz7 =(jclass*) dexstuff_defineclass(&d, "org/tesi/core/DalvikHookImpl", cookieHooks);
	jclass *clazz3 =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/HookList", cookieHooks);
	jclass *testt =(jclass*) dexstuff_defineclass(&d, "org/tesi/Hooks/FuckMe", cookieHooks);
	jclass *clazz4 = (jclass*)dexstuff_defineclass(&d, "org/tesi/core/MyInit", cookieHooks);
	jclass *clazz6 = (jclass*) dexstuff_defineclass(&d, "org/tesi/utils/sendSMS", cookieHooks);


	//jclass *sessionClazz =(jclass*) dexstuff_defineclass(&d, "org/sid/addi/core/Session", cookie);
	
	JNIEnv *env = (JNIEnv*) get_jni_for_thread(&d);
	
	if(debug)
		log("ddi XXX6 init preso env = 0x%x\n", env)	
	if(env){		
		/* new way
		struct stat st = {0};
		if (stat("/data/local/tmp/odex", &st) == -1) {
	    	mkdir("/data/local/tmp/odex", S_IRWXU | S_IRWXG | S_IRWXO);
	    	log("CREATA DIR ODEX\n")
		}
		
		dalvik_dump_class(&d, "Lorg/tesi/bootloader/BootLoader;");
		
		jclass mycls = (*env)->FindClass(env, "org/tesi/bootloader/BootLoader");
		
		jmethodID entrypoint = (*env)->GetStaticMethodID(env, mycls, "_entrypoint", "()V");
		(*env)->CallStaticVoidMethod(env,mycls,entrypoint);

		dalvik_dump_class(&d, "Lorg/sid/addi/core/manageADDI;");
		dalvik_dump_class(&d, "Lorg/sid/addi/core/DalvikHook;");
		dalvik_dump_class(&d, "Lorg/tesi/core/DalvikHookImpl;");
		(*env)->RegisterNatives(env, (*env)->FindClass(env, "org/sid/addi/core/manageADDI"), method_table, sizeof(method_table) / sizeof(method_table[0]));
		dalvikHookCls = (struct ClassObject*) _dvmFindLoadedClass(&d, DALVIKHOOKCLS_2);
		log("trovata dalvikhookcls: %p\n", dalvikHookCls)		
		jmethodID place_hook = (*env)->GetStaticMethodID(env, mycls, "_placehook", "()V");
		(*env)->CallStaticVoidMethod(env,mycls,place_hook);
		*/
		//shared by threads
		stackelem = (*env)->NewGlobalRef(env, (*env)->FindClass(env,"java/lang/StackTraceElement"));
		th = (*env)->NewGlobalRef(env,(*env)->FindClass(env, "java/lang/Thread"));
		log("classe stackelemen : %p\n", stackelem)
		dalvikHookCls = (struct ClassObject*) _dvmFindLoadedClass(&d, DALVIKHOOKCLS_2);
		(*env)->RegisterNatives(env, (*env)->FindClass(env, "org/sid/addi/core/manageADDI"), method_table, sizeof(method_table) / sizeof(method_table[0]));
		jclass mycls = (*env)->FindClass(env, "org/tesi/core/MyInit");
		if(debug)
			log("XXX4 trovata classe myinit = 0x%x, dalvikhookcls: %p\n", mycls, dalvikHookCls)
		//jmethodID constructor = (*env)->GetStaticMethodID(env, mycls, "<init>", "()V");
		jmethodID place_hook = (*env)->GetStaticMethodID(env, mycls, "place_hook", "()V");
		//jobject obj = (*env)->NewObject(env, mycls, constructor);
		(*env)->CallStaticVoidMethod(env,mycls,place_hook);
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
	res->type = UNUSED_HOOK;
	//dalvik_prepare(&d,res,env);
	//res->ok=0;
	
	(*env)->DeleteLocalRef(env, str);
	log("XXX5 FINITO UNHOOK ------------------- \n")    
}

void* setJavaVM(JavaVM* ajvm){
	g_JavaVM = ajvm;
}
