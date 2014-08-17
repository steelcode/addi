/*
 *  Collin's Dynamic Dalvik Instrumentation Toolkit for Android
 *  Collin Mulliner <collin[at]mulliner.org>
 *
 *  (c) 2012,2013
 *
 *  Sid's Advanced DDI
 *  (c) 2014
 *
 *  License: LGPL v2.1
 *
 */

#include <string.h>
#include <jni.h>
#include <stdio.h>
#include <fcntl.h>
#include <dlfcn.h>
//#include <pthread.h>

#include "Common.h"
#include "Object.h"
#include "Stack.h"
#include "Thread.h"
#include "dexfilestuff.h"


#ifndef __dexstuff_h__
#define __dexstuff_h__

#define ALLOC_DEFAULT  0x00
#define ALLOC_DONT_TRACK 0x02
/* move between the stack save area and the frame pointer */
#define SAVEAREA_FROM_FP(_fp)   ((StackSaveArea*)(_fp) -1)
#define FP_FROM_SAVEAREA(_save) ((u4*) ((StackSaveArea*)(_save) +1))




typedef void (*DalvikNativeFunc)(const u4* args, jvalue* pResult);

typedef struct DalvikNativeMethod_t {
    const char* name;
    const char* signature;
    DalvikNativeFunc  fnPtr;
} DalvikNativeMethod;






typedef void* (*dvmCreateStringFromCstr_func)(const char* utf8Str, int len, int allocFlags);
typedef void* (*dvmGetSystemClassLoader_func)(void);
typedef void* (*dvmThreadSelf_func)(void);

typedef void* (*dvmIsClassInitialized_func)(void*);
typedef void* (*dvmInitClass_func)(void*);
typedef void* (*dvmFindVirtualMethodHierByDescriptor_func)(void*,const char*, const char*);
typedef void* (*dvmFindDirectMethodByDescriptor_func)(void*,const char*, const char*);
typedef void* (*dvmIsStaticMethod_func)(void*);
typedef void* (*dvmAllocObject_func)(void*, unsigned int);
typedef void* (*dvmCallMethodV_func)(void*,void*,void*,void*,void*,va_list);
typedef void* (*dvmCallMethodA_func)(void*,void*,void*,bool,void*,jvalue*);
typedef void* (*dvmCallMethod_func)(void*,void*,void*,bool,void*,...);
typedef void* (*dvmAddToReferenceTable_func)(void*,void*);
typedef void (*dvmDumpAllClasses_func)(int);
typedef void* (*dvmFindLoadedClass_func)(const char*);

typedef void (*dvmUseJNIBridge_func)(void*, void*);

typedef void* (*dvmDecodeIndirectRef_func)(Thread*,jobject);

typedef void* (*dvmGetCurrentJNIMethod_func)();

typedef void (*dvmLinearSetReadWrite_func)(void*,void*);

typedef void* (*dvmFindInstanceField_func)(void*,const char*,const char*);

typedef void* (*dvmSetNativeFunc_func)(void*,void*, void*);
typedef void (*dvmCallJNIMethod_func)(const u4*, void*, void*, void*);

typedef void (*dvmHashTableLock_func)(void*);
typedef void (*dvmHashTableUnlock_func)(void*);
typedef void (*dvmHashForeach_func)(void*,void*,void*);

typedef void (*dvmDumpClass_func)(void*,void*);

typedef int (*dvmInstanceof_func)(void*,void*);

//XXX
typedef void* (*dvmGetJNIEnvForThreadv_func)();
typedef void* (*dexProtoCopyMethodDescriptor_func)();
typedef void* (*dvmFindDirectMethodHierByDescriptor_func)(void*, const char*,  const char*);
typedef void* (*dvmGetVirtualizedMethod_func)(void*, const struct Method*);
typedef void* (*dvmFindDirectMethod_func)(void*, const char*, void*);
typedef void* (*dvmFindDirectMethodHier_func)(void*, const char*, void*);
typedef void (*dvmDumpJniReferenceTablesv_func)();
typedef void* (*dvmGetCallerFP_func)(void*);
typedef void* (*dvmGetCallerClass_func)(void*);
typedef void* (*dvmGetCaller2Class_func)(void*);
typedef void* (*dvmGetCaller3Class_func)(void*);
typedef void (*dvmSuspendThread_func)(void*);
typedef void (*dvmResumeThread_func)(void *);
typedef void (*dvmSuspendSelf_func)();
typedef void* (*dvmTryLockThreadList_func)();
typedef void (*dvmUnlockThreadList_func)();
typedef void (*dvmSuspendAllThreads_func)(void *);
typedef void (*dvmResumeAllThreads_func)(void *);
typedef void* (*dvmAttachCurrentThread_func)(void *,  void*);
typedef void* (*dvmMterpPrintMethod_func)(void*);
typedef void (*dvmDumpAllThreadsb_func)();
typedef void* (*dvmDexFileOpenFromFd_func)(void*, void**);
typedef void* (*sysChangeMapAccess_func)(void*,void*,void*,void*);
typedef  void* (*dvmDexChangeDex1_func)(void* , void*, void*);
typedef void* (*dvmFindStaticField_func)(void*,void*,void*);
typedef void*(*dvmInvokeMethod_func)(void*,void*,void*,void*,void*,bool);
typedef void* (*dvmCreateCstrFromString_func)(void*);
typedef void (*dvmDumpObject_func)(void*);
typedef void* (*dvmGetStaticFieldObject_func)(void*);
typedef void* (*dvmFindClass_func)(void*,void*);
typedef void (*dvmFreeClassInnards_func)(void*);
typedef void* (*dvmComputeMethodArgsSize_func)(void*);
typedef void* (*dvmDexGetResolvedMethod_func)(void*,void*);
typedef void* (*dexReadAndVerifyClassData_func)(void*,void*);
typedef void*  (*dvmAllocArrayByClass_func)(struct ClassObject*  arrayClass, size_t length, int allocFlags);
typedef void* (*dvmFindArrayClass_func)(char* desc, void* loader);
typedef void* (*dvmBoxPrimitive_func)(JValue, struct ClassObject*);
typedef void* (*dvmFindPrimitiveClass_func)(void*);
typedef void (*dvmReleaseTrackedAlloc_func)(struct Object*, struct Thread*);
typedef void (*dvmWriteBarrierArray_func)(void*, void*, void*);
typedef void (*dvmLogExceptionStackTrace_func)();
typedef bool (*dvmCheckException_func)(void*);
typedef bool (*dvmIsNativeMethod_func)(void*);
typedef void* (*dvmFindSystemClass_func)(void*);
typedef void* (*dvmGetBoxedTypeDescriptor_func)(PrimitiveType);
typedef void* (*dvmGetBoxedReturnType_func)(struct Method*);
typedef void* (*dvmUnboxPrimitive_func)(struct Object*, struct ClassObject*, JValue*);
typedef void* (*dvmIsPrimitiveClass_func)(struct ClassObject*);
typedef int (*dvmConvertPrimitiveValue_func)(PrimitiveType, PrimitiveType, s4*, s4*);
typedef void* (*dvmCreateInternalThread_func)(pthread_t*,char*,InternalThreadStart, void*);
typedef void* (*dvmMalloc_func)(size_t,int);
typedef void* (*dvmLinearAlloc_func)(void*, size_t);
typedef void* (*dexProtoGetParameterCount_func)(void*);
typedef void* (*dvmGetArgLong_func)(void*, void*);
typedef void* (*dvmGetException_func)(void*);
typedef void* (*dvmWrapException_func)(void *);
typedef void* (*dvmThrowFileNotFoundException_func)(void*);
typedef void* (*dvmHumanReadableDescriptor_func)(void*);
typedef void* (*dvmGetMethodThrows_func)(void*);
typedef void* (*dvmClearException_func)(void*);
typedef void* (*dvmThrowClassNotFoundException_func)(void*);
typedef void* (*dvmGetRawDexFileDex_func)(struct RawDexFile*);
typedef void* (*JNI_GetCreatedJavaVMs_func)(void**, void*, void*);
typedef void* (*dvmFillStackTraceArray_func)(void*,void*,void*);
typedef void* (*dvmComputeExactFrameDepth_func)(void*);

typedef void* (*dvmChangeStatus_func)(void*,void*);

//typedef void* (*dvmCreateStringFromCstr_func)(void*);
/**
typedef void (*DalvikNativeFuncToHook)(struct DalvikNativeMethodToHook*, ...);

typedef struct DalvikNativeMethodToHook_t {
    const char* name;
    const char* signature;
    const size_t args_size;
    DalvikNativeFuncToHook  fnPtr;
} DalvikNativeMethodToHook;

struct my_dexstuff_t {
    DalvikNativeMethodToHook* amethod;
};
*/
struct dexstuff_t
{	
	void *dvm_hand;
	
	dvmCreateStringFromCstr_func dvmStringFromCStr_fnPtr;
	dvmGetSystemClassLoader_func dvmGetSystemClassLoader_fnPtr;
	dvmThreadSelf_func dvmThreadSelf_fnPtr;

	dvmIsClassInitialized_func dvmIsClassInitialized_fnPtr;
	dvmInitClass_func dvmInitClass_fnPtr;
	dvmFindVirtualMethodHierByDescriptor_func dvmFindVirtualMethodHierByDescriptor_fnPtr;
	dvmFindDirectMethodByDescriptor_func dvmFindDirectMethodByDescriptor_fnPtr;
	dvmIsStaticMethod_func dvmIsStaticMethod_fnPtr;
	dvmAllocObject_func dvmAllocObject_fnPtr;
	dvmCallMethodV_func dvmCallMethodV_fnPtr;
	dvmCallMethodA_func dvmCallMethodA_fnPtr;
    dvmCallMethod_func dvmCallMethod_fnPtr;
	dvmAddToReferenceTable_func dvmAddToReferenceTable_fnPtr;
	dvmDecodeIndirectRef_func dvmDecodeIndirectRef_fnPtr;
	dvmUseJNIBridge_func dvmUseJNIBridge_fnPtr;
	dvmFindInstanceField_func dvmFindInstanceField_fnPtr;
	dvmFindLoadedClass_func dvmFindLoadedClass_fnPtr;
	dvmDumpAllClasses_func dvmDumpAllClasses_fnPtr;

    //XXX
    dvmGetJNIEnvForThreadv_func dvmGetJNIEnvForThreadv_fnPtr;
    dexProtoCopyMethodDescriptor_func dexProtoCopyMethodDescriptor_fnPtr;
    dvmDumpAllThreadsb_func dvmDumpAllThreadsb_fnPtr;
    dvmFindDirectMethodHierByDescriptor_func dvmFindDirectMethodHierByDescriptor_fnPtr;
    dvmGetVirtualizedMethod_func dvmGetVirtualizedMethod_fnPtr;
    dvmFindDirectMethodHier_func dvmFindDirectMethodHier_fnPtr;
    dvmFindDirectMethod_func dvmFindDirectMethod_fnPtr;
    dvmDumpJniReferenceTablesv_func  dvmDumpJniReferenceTablesv_fnPtr;
    dvmGetCallerClass_func dvmGetCallerClass_fnPtr;
    dvmGetCaller2Class_func dvmGetCaller2Class_fnPtr;
    dvmGetCaller3Class_func dvmGetCaller3Class_fnPtr;
    dvmGetCallerFP_func dvmGetCallerFP_fnPtr;
    dvmSuspendThread_func dvmSuspendThread_fnPtr;
    dvmResumeThread_func  dvmResumeThread_fnPtr;
    dvmSuspendSelf_func dvmSuspendSelf_fnPtr;
    dvmUnlockThreadList_func dvmUnlockThreadList_fnPtr;
    dvmTryLockThreadList_func dvmTryLockThreadList_fnPtr;
    dvmResumeAllThreads_func dvmResumeAllThreads_fnPtr;
    dvmSuspendAllThreads_func dvmSuspendAllThreads_fnPtr;
    dvmAttachCurrentThread_func dvmAttachCurrentThread_fnPtr;
	dvmMterpPrintMethod_func dvmMterpPrintMethod_fnPtr;
    dvmDexFileOpenFromFd_func dvmDexFileOpenFromFd_fnPtr;
    sysChangeMapAccess_func sysChangeMapAccess_fnPtr;
    dvmDexChangeDex1_func  dvmDexChangeDex1_fnPtr;
    dvmFindStaticField_func dvmFindStaticField_fnPtr;
    dvmInvokeMethod_func dvmInvokeMethod_fnPtr;
    dvmCreateCstrFromString_func    dvmCreateCstrFromString_fnPtr;
    dvmDumpObject_func dvmDumpObject_fnPtr;
    dvmGetStaticFieldObject_func dvmGetStaticFieldObject_fnPtr;
    dvmFindClass_func dvmFindClass_fnPtr;
    dvmFreeClassInnards_func  dvmFreeClassInnards_fnPtr;
    dvmComputeMethodArgsSize_func dvmComputeMethodArgsSize_fnPtr;
    dvmDexGetResolvedMethod_func dvmDexGetResolvedMethod_fnPtr;
    dexReadAndVerifyClassData_func dexReadAndVerifyClassData_fnPtr;
    dvmAllocArrayByClass_func dvmAllocArrayByClass_fnPtr;
    dvmFindArrayClass_func dvmFindArrayClass_fnPtr;
    dvmBoxPrimitive_func dvmBoxPrimitive_fnPtr;
    dvmFindPrimitiveClass_func dvmFindPrimitiveClass_fnPtr;
    dvmReleaseTrackedAlloc_func dvmReleaseTrackedAlloc_fnPtr;
    dvmWriteBarrierArray_func dvmWriteBarrierArray_fnPtr;
    dvmLogExceptionStackTrace_func dvmLogExceptionStackTrace_fnPtr;
    dvmCheckException_func dvmCheckException_fnPtr;
    dvmIsNativeMethod_func dvmIsNativeMethod_fnPtr;
    dvmFindSystemClass_func dvmFindSystemClass_fnPtr;
    dvmGetBoxedTypeDescriptor_func dvmGetBoxedTypeDescriptor_fnPtr;
    dvmGetBoxedReturnType_func dvmGetBoxedReturnType_fnPtr;
    dvmUnboxPrimitive_func dvmUnboxPrimitive_fnPtr;
    dvmIsPrimitiveClass_func dvmIsPrimitiveClass_fnPtr;
    dvmConvertPrimitiveValue_func dvmConvertPrimitiveValue_fnPtr;
    dvmCreateInternalThread_func dvmCreateInternalThread_fnPtr;
    dvmMalloc_func dvmMalloc_fnPtr;
    dvmLinearAlloc_func dvmLinearAlloc_fnPtr;
    dexProtoGetParameterCount_func dexProtoGetParameterCount_fnPtr;
    dvmGetArgLong_func dvmGetArgLong_fnPtr;
    dvmGetException_func dvmGetException_fnPtr;
    dvmWrapException_func dvmWrapException_fnPtr;
    dvmThrowFileNotFoundException_func dvmThrowFileNotFoundException_fnPtr;
    dvmHumanReadableDescriptor_func dvmHumanReadableDescriptor_fnPtr;
    dvmGetMethodThrows_func dvmGetMethodThrows_fnPtr;
    dvmClearException_func dvmClearException_fnPtr;
    dvmThrowClassNotFoundException_func dvmThrowClassNotFoundException_fnPtr;
    dvmGetRawDexFileDex_func dvmGetRawDexFileDex_fnPtr;
    //dvmCreateStringFromCstr_func dvmCreateStringFromCstr_fnPtr;
    dvmChangeStatus_func dvmChangeStatus_fnPtr;
    JNI_GetCreatedJavaVMs_func JNI_GetCreatedJavaVMs_fnPtr;
    dvmFillStackTraceArray_func dvmFillStackTraceArray_fnPtr;
    dvmComputeExactFrameDepth_func dvmComputeExactFrameDepth_fnPtr;

	dvmGetCurrentJNIMethod_func dvmGetCurrentJNIMethod_fnPtr;
	dvmLinearSetReadWrite_func dvmLinearSetReadWrite_fnPtr;
	
	dvmSetNativeFunc_func dvmSetNativeFunc_fnPtr;
	dvmCallJNIMethod_func dvmCallJNIMethod_fnPtr;
	
	dvmHashTableLock_func dvmHashTableLock_fnPtr;
	dvmHashTableUnlock_func dvmHashTableUnlock_fnPtr;
	dvmHashForeach_func dvmHashForeach_fnPtr;
	
	dvmDumpClass_func dvmDumpClass_fnPtr;
	dvmInstanceof_func dvmInstanceof_fnPtr;
	
	DalvikNativeMethod *dvm_dalvik_system_DexFile;
	DalvikNativeMethod *dvm_java_lang_Class;
    DalvikNativeMethod *dvm_dalvik_system_VMStack;
    DalvikNativeMethod *dvm_java_lang_VMClassLoader;
		
	struct DvmGlobals *gDvm; // dvm globals !
    struct DvmJniGlobals* gDvmJni;
	
	int done;
};



#endif

void dexstuff_resolv_dvm(struct dexstuff_t *d);
void* dexstuff_loaddex(struct dexstuff_t *d, char *path);
void* dexstuff_defineclass(struct dexstuff_t *d, char *name, int cookie);
Thread* getSelf(struct dexstuff_t *d);
void dalvik_dump_class(struct dexstuff_t *dex, char *clname);
int is_static(struct dexstuff_t *, struct Method *);
void * get_method(struct dexstuff_t *, char* , char * );
void * get_jni_for_thread(struct dexstuff_t *);
