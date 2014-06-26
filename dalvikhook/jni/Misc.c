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
#include <string.h>

#include "log.h"
#include "Misc.h"

/*
 * Return a newly-allocated string in which all occurrences of '.' have
 * been changed to '/'.  If we find a '/' in the original string, NULL
 * is returned to avoid ambiguity.
 */
char* dvmDotToSlash(char* str)
{
    char* newStr = strdup(str);
    char* cp = newStr;

    if (newStr == NULL)
        return NULL;

    while (*cp != '\0') {
        if (*cp == '/') {
            return NULL;
        }
        if (*cp == '.')
            *cp = '/';
        cp++;
    }

    return newStr;
}

void* get_caller_class(JNIEnv *env, jobject obj, char *c){

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
	log("XXX3 Calling class is: %s\n", str2)
	strcpy(c,str2);

	// Release the memory pinned char array
	(*env)->ReleaseStringUTFChars(env,strObj, str2);
}

int is_string_class(char *c){

	char *s = strstr(c, "String");
	if(s != NULL)
		return 1;
	else
		return 0;
}
char* parse_signature(char *src){

	char *c = src;
	while(c != NULL && *c != ')')
	{	
		c++;
	}
	c++;	
	return c;
}

void parse_descriptor(char *desc, char *res){
	log("XXX5 ricevuto: %s\n", desc)
	int i =0;
	desc++;
	while(*desc != ')'){
		switch(*desc){
			case 'L':
				res[i] = *desc;
				while(*desc != ';'){
					desc++;
				}
				desc++; i++;
				break;
			default:
				res[i] = *desc;
				desc++;i++;
				break;
		}
	}
	res[i] = '\0';
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

void get_info(JNIEnv *env){
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