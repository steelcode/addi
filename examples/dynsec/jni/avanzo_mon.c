// patches
static void* sb21_tostring(JNIEnv *env, jobject obj,jobject intent)
{


/*
	log("tostring2\n");
	log("env = 0x%x\n", env);
	log("obj = 0x%x\n", obj);

  if(cont == 0){
    log("provo a caricare un file .dex cont vale: %d\n", cont);
    //provo a caricare un dex
    int val = dexstuff_loaddex(&d, "/data/local/tmp/classes.dex");
    log("caricato cookie =  0x%x\n", val);
    if(!val)
      log("ERRORE CARICAMENTO DEX FILE!!!\n");
    void *clazz = dexstuff_defineclass(&d, "com/tesi/intentsniffer/HookActivity", val);
    log("caricato dex: clazz = 0x%x\n", clazz)
    cont++;
    jclass myhookclass = (*env)->FindClass(env, "com/tesi/intentsniffer/HookActivity");
    jmethodID costruttore = (*env)->GetMethodID(env, myhookclass, "<init>","(Landroid/content/Intent)V;");
   if(costruttore){
    jvalue args[1];
    args[0].l = intent;
    log("CHIAMO NEWOBJECTA\n");
    jobject obj = (*env)->NewObjectA(env, myhookclass, costruttore,args);
    log("mio hook: 0x%x\n", obj);     
    if(!obj)
      log("errore creazione classe myhook!!!\n"); 
   }
   else
    log("costruttore non trovato\n!!!");
    }
    else
      log("CONTATORE DIVERSO DA ZERO!!!\n");
   
    

  jvalue args[1];
  args[0].l = intent;
	dalvik_prepare(&d, &sb20, env);
	void *res = (*env)->CallObjectMethod(env, obj, sb20.mid, args); 
	log("success calling : %s\n", sb20.method_name)
	dalvik_postcall(&d, &sb20);

	char *s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("sb20.toString() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}	

	return res;
	*/
}

static void* sb20_tostring(JNIEnv *env, jobject obj)
{


	log("tostring\n")
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)

	dalvik_prepare(&d, &sb20, env);
	void *res = (*env)->CallObjectMethod(env, obj, sb20.mid); 
	log("success calling : %s\n", sb20.method_name)
	dalvik_postcall(&d, &sb20);

	char *s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("sb20.toString() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}	

	return res;
}

static void sb2_miometodo(JNIEnv *env, jobject obj, jobject intent, jobject bundle){


	log("sb2_start\n")
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
  jvalue args[2];
  args[0].l = intent;
  args[1].l = bundle;
  dalvik_prepare(&d, &sb2, env);
  //void *res = (*env)->CallObjectMethod(env, obj, sb2.mid,args);
  //(*env)->CallObjectMethod(env, obj, sb2.mid,args);
  log("success calling: %s \n", sb2.method_name);
  log("\nYUPPI YUPPI 2\n");
  dalvik_postcall(&d, &sb2);
  //return res;
 
}
static void* sb5_getmethod(JNIEnv *env, jobject obj, jobject str, jobject cls)
{
/*
	log("getmethod\n")
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	log("str = 0x%x\n", str)
	log("cls = 0x%x\n", cls)
*/

	jvalue args[2];
	args[0].l = str;
	args[1].l = cls;
	dalvik_prepare(&d, &sb5, env);
	void *res = (*env)->CallObjectMethodA(env, obj, sb5.mid, args); 
	log("success calling : %s\n", sb5.method_name)
	dalvik_postcall(&d, &sb5);

	if (str) {
		char *s = (*env)->GetStringUTFChars(env, str, 0);
		if (s) {
			log("sb5.getmethod = %s\n", s)
			(*env)->ReleaseStringUTFChars(env, str, s); 
		}
	}

	return res;
}



static void sb1_miometodo(JNIEnv *env, jobject obj, jobject intent){


	log("sb1_MIOMETODO\n")
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	if(myl.cls == 0){
	
		
		
		int cookie = dexstuff_loaddex(&d, "/data/local/tmp/classes.dex");
		log("libsmsdispatch: loaddex res = 0x%x\n", cookie)
		if (!cookie)
			log("libsmsdispatch: make sure /data/dalvik-cache/ is world writable and delete data@local@tmp@ddiclasses.dex\n")
		void *clazz = dexstuff_defineclass(&d, "org/tesi/fakecontext/FakeContext", cookie);
		void *clazz2 = dexstuff_defineclass(&d, "org/tesi/fakecontext/MessangerService", cookie);
		log("libsmsdispatch: clazz = 0x%x, clazz2 = 0x%x\n", clazz,clazz2)
		// call constructor and passin the pdu
		jclass smsd = (*env)->FindClass(env, "org/tesi/fakecontext/FakeContext");
		myl.cls =  (jclass) (*env)->NewGlobalRef(env,smsd);
		log("SMSD: 0x%x, CLS: 0x%x\n", smsd, myl.cls)
		jmethodID constructor = (*env)->GetMethodID(env, smsd, "<init>", "(Landroid/content/Intent;)V");
		jmethodID  start_magic = (*env)->GetMethodID(env,smsd,"start_magic","()V");
		if(!start_magic){
			log("ERRORE START MAGIC!\n")
		}
		myl.eid = start_magic;
		log("MID: 0x%x\n", constructor)
		myl.mid = constructor;
		if (constructor) { 
			log("trovato costruttore!!\n")
			jvalue args[1];
			args[0].l = intent;
			log("libsmsdispatch: old obj = 0x%x\n", obj)
			//XXX
			jobject obj = (*env)->NewObjectA(env, smsd, constructor, args);      
			//jobject obj = (*env)->NewObjectA(env, myl.cls,myl.mid, args);      
			myl.obj =  (*env)->NewGlobalRef(env,obj);
			log("libsmsdispatch: new obj = 0x%x\n", obj)
			
			if (!obj)
				log("libsmsdispatch: failed to create smsdispatch class, FATAL!\n")
		}
		else {
			log("libsmsdispatch: constructor not found!\n")
		}
	}
	log("prima di callvoidmethod: %s, obj: 0x%x!!\n", sb1.method_name, obj);
 	jvalue args[1];
	args[0].l = intent;
	dalvik_prepare(&d, &sb1, env);
	(*env)->CallVoidMethodA(env, obj, sb1.mid, args);
	log("success calling : %s\n\n", sb1.method_name)
	dalvik_postcall(&d, &sb1);
	if(myl.cls){
		log("------------------\n")
		log("DEBUG, cls: 0x%x, mid: 0x%x \n", myl.cls, myl.mid)
		log("------------------\n")
		//jvalue args[1];
		//args[0].l = intent;
		//jobject obj = (*env)->NewObjectA(env, myl.cls, myl.mid, args);
		(*env)->CallVoidMethod(env,myl.obj,myl.eid);
	}
}

static jboolean my_verify(JNIEnv *env, jobject o, jobject b){
	log(" ------------- MY VERIFY ---------------- \n")
	return (jboolean)1;
}


static jboolean my_verify2(JNIEnv *env, jobject o, jobject b, jint x, jint y){
	log(" ------------- MY VERIFY 2 ---------------- \n")
	return (jboolean)1;
}


static void wrapper_1(JNIEnv *env, jobject obj, jobject intent){ //1 argomento
	pthread_mutex_lock(&mutex);
	log("---------------------GET SUBSCRIBER2222!!-------------------------------\n")
	char cl[256];
	char name[256];
	char descriptor[256];
	char hash[256];
	struct dalvik_hook_t *res;
	void *str;
	jboolean i;
	char *s = (char *) malloc(sizeof(char *));

	get_caller_class(env,obj,cl);
	get_method(&d,name,descriptor);

	strncpy(hash,name,sizeof(name));
	strncat(hash,descriptor,sizeof(descriptor));
	log("CERCO NELLA LISTA : %s\n", hash)
	res = (struct dalvik_hook_t *) cerca(L, hash);
	log("trovato = 0x%x\n", res)
	*s = parse_signature(res->method_sig);
	jvalue args[1];
	args[0].l= intent;

 	dalvik_prepare(&d, res, env);
 	log("devo chiamare: %s\n", res->method_name)
	switch(*s){
		
		case 'L':
			log("CLASSE!!\n")
			str = (*env)->CallObjectMethod(env, obj, res->mid, args);
			log("success calling : %s\n", res->method_name)
			break;
		case 'V':
			log("VOID obj = 0x%x, mid = 0x%x, args = 0x%x\n", obj, res->mid, args)
			(*env)->CallVoidMethodA(env, obj, res->mid, args);
			log("success calling : %s\n", res->method_name)
			break;
		case 'Z':
			log("BOOOLEAN\n") //boolean is always true :)
			return (jboolean)1;
			i = (*env)->CallBooleanMethod(env, obj, res->mid, args);
			dalvik_postcall(&d, res);
			break;
		default:
			log("DEFAULT \n")
			str = (*env)->CallObjectMethod(env, obj, res->mid, args);
			log("success calling : %s\n", res->method_name)
			break;
	}
	log("FUORI SWITCH\n")
	dalvik_postcall(&d, res);
	log("----------------------------------------------------\n")
	pthread_mutex_unlock(&mutex);
}

static void my_verify_chain(JNIEnv * env, jobject o, jobject str){
	log("---------------- CHECK SERVER TRUSTED ------------\n")
	return (void)1;
}


static jobject my_get_detailed_state(JNIEnv * env, jobject obj){
	log("---------------- GET DETAILED STATE ------------\n")
	jobject res;

	dalvik_prepare(&d, &sb20, env);
	res = (*env)->CallObjectMethod(env, obj, sb20.mid);
	log("success calling : %s\n", sb20.method_name)
	dalvik_postcall(&d, &sb20);
	return res;
	//return (jobject)1;
}
static void* sb13_my_get_subscriber(JNIEnv *env, jobject obj){
	log("---------------------GET SUBSCRIBER!!-------------------------------\n")
	char cl[256];
	char meth_n[256];
	char shorty_n[256];

	get_caller_class(env,obj,cl);

	
	get_method(&d,meth_n,shorty_n);
	
 	dalvik_prepare(&d, &sb13, env);
	void *str = (*env)->CallObjectMethod(env, obj, sb13.mid);
	log("success calling : %s\n", sb13.method_name)
	dalvik_postcall(&d, &sb13);
	if (str) {
		char *s = (*env)->GetStringUTFChars(env, str, 0);
		if (s) {
			log("SUBSCRIBER = %s\n", s)
			(*env)->ReleaseStringUTFChars(env, str, s); 
		}
	}
	else{
		log("errore str vale: 0x%x\n", str)
	}
	log("----------------------------------------------------\n")
	return str;
}

static jobject my_get_state(JNIEnv * env, jobject o){
	log("---------------- GET STATE ------------\n")
	return (jobject)1;
}

static jobject my_get_device_id(JNIEnv * env, jobject obj){
	log("---------------- GET DEVICE ID ------------\n")
	jobject str;
 	char fakestr[15] = "351895050066172";
 	jstring fake;

 	dalvik_prepare(&d, &sb4, env);
	str = (*env)->CallObjectMethod(env, obj, sb4.mid);
	log("success calling : %s\n", sb4.method_name)
	dalvik_postcall(&d, &sb4);
	if (str) {
		char *s = (*env)->GetStringUTFChars(env, str, 0);
		if (s) {
			log("GETDEVICEID = %s\n", s)
			(*env)->ReleaseStringUTFChars(env, str, s); 
		}
	}

	fake = (*env)->NewStringUTF(env, &fakestr);
	//free(fakestr);
	return fake;
}

static jobject sb6_my_get_line1number(JNIEnv * env, jobject obj)
{
	log("---------------- GET LINE 1 NUMBER ------------\n")
	jobject str;
 	char fakestr[15] = "4141414141";
 	jstring fake;

 	dalvik_prepare(&d, &sb6, env);
	str = (*env)->CallObjectMethod(env, obj, sb6.mid);
	log("success calling : %s\n", sb6.method_name)
	dalvik_postcall(&d, &sb6);
	if (str) {
		char *s = (*env)->GetStringUTFChars(env, str, 0);
		if (s) {
			log("LINE1NUMBER = %s\n", s)
			(*env)->ReleaseStringUTFChars(env, str, s); 
		}
	}

	fake = (*env)->NewStringUTF(env, &fakestr);
	//free(fakestr);
	return fake;
}

static void* sb22_compareto(JNIEnv *env, jobject obj, jobject str)
{
/*
	log("compareto\n")
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	log("str = 0x%x\n", str)
*/

	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb22, env);
	int res = (*env)->CallIntMethodA(env, obj, sb22.mid, args); 
	log("success calling : %s\n", sb22.method_name)
	dalvik_postcall(&d, &sb22);

	printString(env, obj, "sb22 = "); 

	char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		log("sb22.comapreTo() = %d s> %s\n", res, s)
		(*env)->ReleaseStringUTFChars(env, str, s);
	}
	
	return res;
}


static void* sb23_tostring(JNIEnv *env, jobject obj)
{
/*
	log("tostring\n")
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
*/
 	dalvik_prepare(&d, &sb23, env);
	void *res = (*env)->CallObjectMethod(env, obj, sb23.mid); 
	log("success calling : %s\n", sb23.method_name)
	dalvik_postcall(&d, &sb23);

	char *s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("sb23.toString() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}

	return res;
}

static void* sb24_castle(JNIEnv *env, jobject obj, jobject context, jobject arr){
log("----------------  PINNING HELPER ------------------\n")
jvalue args[2];
args[0].l = context;
args[1].l = arr;
dalvik_prepare(&d, &sb24, env);
void *res = (*env)->CallObjectMethodA(env, obj, sb24.mid, args); 
log("success calling : %s\n", sb24.method_name)
dalvik_postcall(&d, &sb24);
return res;

}
static void sb2_onCreate(JNIEnv *env, jobject obj, jobject bundle){
	time_t rawtime;
  	struct tm * timeinfo;

  	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
  	char *mytime = asctime(timeinfo);
	//char *s = (*env)->GetStringUTFChars(env, mytime, 0);
  	jstring jstrBuf = (*env)->NewStringUTF(env, mytime);

	log("-------------------------------------------\n")
	log("DENTRO ON CREATE\n")
	jint pid = getpid();
	log("PRIMA DI GET METHOD\n")
	get_method(&d);
	/**
	if(myl.cls){
		log("DEBUG, PID: %d, TIME: %s, cls: 0x%x, obj: 0x%x, mid: 0x%x, eid: 0x%x \n", pid, mytime, myl.cls, myl.obj,myl.mid, myl.eid)
		jvalue myargs[2];
		myargs[0].l = pid;
		myargs[1].l = jstrBuf;
		(*env)->CallVoidMethodA(env,myl.obj,myl.eid, myargs);
	}
	*/
	jvalue args[1];
	args[0].l = bundle;
	dalvik_prepare(&d, &sb2, env);
	(*env)->CallVoidMethodA(env,obj,sb2.mid,args);
	dalvik_postcall(&d,&sb2);
	log("-------------------------------------------\n")
}

static void* sb3_getString(JNIEnv *env, jobject obj, jobject resolver, jobject str){
	log("------------------getString-------------------------\n")
	printStackTrace(env);	
	get_method(&d);
	jvalue args[2];
	args[0].l = resolver;
	args[1].l = str;
	dalvik_prepare(&d, &sb3, env);
	void *res = (*env)->CallStaticObjectMethodA(env,obj,sb3.mid,args);
	dalvik_postcall(&d,&sb3);
	printString(env,str,"get_string = ");

	char *s = (*env)->GetStringUTFChars(env, res, 0);
	if (s) {
		log("sb3.getString() = %s\n", s)
		(*env)->ReleaseStringUTFChars(env, res, s); 
	}
	log("----------------------------------------------------\n")
	return res;
}

static void* sb13_equalsIgnoreCase(JNIEnv *env, jobject obj, jobject str)
{
/*
	log("env = 0x%x\n", env)
	log("obj = 0x%x\n", obj)
	log("str = 0x%x\n", str)
*/

	jvalue args[1];
	args[0].l = str;
	dalvik_prepare(&d, &sb13, env);
	int res = (*env)->CallBooleanMethodA(env, obj, sb13.mid, args);
	//log("success calling : %s\n", sb13.method_name)
	dalvik_postcall(&d, &sb13);

	printString(env, obj, "sb13 = "); 

	char *s = (*env)->GetStringUTFChars(env, str, 0);
	if (s) {
		log("sb13.equalsIgnoreCase() = %d %s\n", res, s)
		(*env)->ReleaseStringUTFChars(env, str, s); 
	}

	return res;
}

static void* sb22_my_get_simnum(JNIEnv *env, jobject obj){
	log("---------------------SIM NUMBER-------------------------------\n")
	
 	dalvik_prepare(&d, &sb22, env);
	void *str = (*env)->CallObjectMethod(env, obj, sb22.mid);
	log("success calling : %s\n", sb22.method_name)
	dalvik_postcall(&d, &sb22);
	if (str) {
		char *s = (*env)->GetStringUTFChars(env, str, 0);
		if (s) {
			log("SIM NUMBER = %s\n", s)
			(*env)->ReleaseStringUTFChars(env, str, s); 
		}
	}
	else{
		log("errore str vale: 0x%x\n", str)
	}
	log("----------------------------------------------------\n")
	return str;
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