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
 
void* get_caller_class(JNIEnv *env, jobject obj, char *c);
int is_string_class(char *c);
char* parse_signature(char *src);
void parse_descriptor(char *desc, char *res);
void printStackTrace(JNIEnv *env);
void get_info(JNIEnv *env);