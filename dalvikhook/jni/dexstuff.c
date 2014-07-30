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
#include <dlfcn.h>
#include <stdarg.h>

#include "dexstuff.h"
#include "Globals.h"
#include "log.h"


static void* tself;
size_t arrayContentsOffset = 0;

static void* mydlsym(void *hand, const char *name)
{
	void* ret = dlsym(hand, name);
	log("%s = 0x%x\n", name, ret)
	return ret;
}

/**
	Salvik nella struttura dexstuff_t i riferimenti ai metodi della libdvm.so
*/
void dexstuff_resolv_dvm(struct dexstuff_t *d)
{
	d->dvm_hand = dlopen("libdvm.so", RTLD_NOW);
	log("dvm_hand = 0x%x\n", (unsigned int)d->dvm_hand)
	
	if (d->dvm_hand) {
		d->dvm_dalvik_system_DexFile = (DalvikNativeMethod*) mydlsym(d->dvm_hand, "dvm_dalvik_system_DexFile");
		d->dvm_dalvik_system_VMStack = (DalvikNativeMethod*) mydlsym(d->dvm_hand, "dvm_dalvik_system_VMStack");
		d->dvm_java_lang_VMClassLoader = (DalvikNativeMethod*) mydlsym(d->dvm_hand, "dvm_java_lang_VMClassLoader");
		d->dvm_java_lang_Class = (DalvikNativeMethod*) mydlsym(d->dvm_hand, "dvm_java_lang_Class");	
		d->dvmThreadSelf_fnPtr = mydlsym(d->dvm_hand, "_Z13dvmThreadSelfv");
		if (!d->dvmThreadSelf_fnPtr)
			d->dvmThreadSelf_fnPtr = mydlsym(d->dvm_hand, "dvmThreadSelf");
		
		d->dvmGetJNIEnvForThreadv_fnPtr = mydlsym(d->dvm_hand, "_Z21dvmGetJNIEnvForThreadv");
		d->dexProtoCopyMethodDescriptor_fnPtr = mydlsym(d->dvm_hand,"_Z28dexProtoCopyMethodDescriptorPK8DexProto");
		d->dvmDumpAllThreadsb_fnPtr = mydlsym(d->dvm_hand,"_Z17dvmDumpAllThreadsb");
		d->dvmStringFromCStr_fnPtr = mydlsym(d->dvm_hand, "_Z32dvmCreateStringFromCstrAndLengthPKcj");
		d->dvmGetSystemClassLoader_fnPtr = mydlsym(d->dvm_hand, "_Z23dvmGetSystemClassLoaderv");
		d->dvmIsClassInitialized_fnPtr = mydlsym(d->dvm_hand, "_Z21dvmIsClassInitializedPK11ClassObject");
		d->dvmInitClass_fnPtr = mydlsym(d->dvm_hand, "dvmInitClass");
		d->dvmFindVirtualMethodHierByDescriptor_fnPtr = mydlsym(d->dvm_hand, "_Z36dvmFindVirtualMethodHierByDescriptorPK11ClassObjectPKcS3_");
		if (!d->dvmFindVirtualMethodHierByDescriptor_fnPtr)
			d->dvmFindVirtualMethodHierByDescriptor_fnPtr = mydlsym(d->dvm_hand, "dvmFindVirtualMethodHierByDescriptor");
		d->dvmFindDirectMethodByDescriptor_fnPtr = mydlsym(d->dvm_hand, "_Z31dvmFindDirectMethodByDescriptorPK11ClassObjectPKcS3_");
		if (!d->dvmFindDirectMethodByDescriptor_fnPtr)
			d->dvmFindDirectMethodByDescriptor_fnPtr = mydlsym(d->dvm_hand, "dvmFindDirectMethodByDescriptor");

		//XXX
		d->dvmFindDirectMethodHierByDescriptor_fnPtr = mydlsym(d->dvm_hand, "_Z35dvmFindDirectMethodHierByDescriptorPK11ClassObjectPKcS3_");
		d->dvmGetVirtualizedMethod_fnPtr = mydlsym(d->dvm_hand, "_Z23dvmGetVirtualizedMethodPK11ClassObjectPK6Method");
		d->dvmFindDirectMethod_fnPtr = mydlsym(d->dvm_hand,"_Z19dvmFindDirectMethodPK11ClassObjectPKcPK8DexProto");
		d->dvmFindDirectMethodHier_fnPtr = mydlsym(d->dvm_hand, "_Z23dvmFindDirectMethodHierPK11ClassObjectPKcPK8DexProto");
		d->dvmDumpJniReferenceTablesv_fnPtr = mydlsym(d->dvm_hand, "_Z25dvmDumpJniReferenceTablesv");
		d->dvmGetCallerFP_fnPtr = mydlsym(d->dvm_hand, "_Z14dvmGetCallerFPPKv");
		d->dvmGetCallerClass_fnPtr = mydlsym(d->dvm_hand, "_Z17dvmGetCallerClassPKv");
		d->dvmGetCaller2Class_fnPtr = mydlsym(d->dvm_hand, "_Z18dvmGetCaller2ClassPKv");
		d->dvmGetCaller3Class_fnPtr = mydlsym(d->dvm_hand, "_Z18dvmGetCaller3ClassPKv");
		d->dvmSuspendThread_fnPtr = mydlsym(d->dvm_hand, "_Z16dvmSuspendThreadP6Thread");
		d->dvmSuspendSelf_fnPtr = mydlsym(d->dvm_hand, "_Z14dvmSuspendSelfb");
		d->dvmResumeThread_fnPtr = mydlsym(d->dvm_hand, "_Z15dvmResumeThreadP6Thread");
		d->dvmTryLockThreadList_fnPtr = mydlsym(d->dvm_hand, "_Z20dvmTryLockThreadListv");
		d->dvmUnlockThreadList_fnPtr = mydlsym(d->dvm_hand, "_Z19dvmUnlockThreadListv");
		d->dvmSuspendAllThreads_fnPtr = mydlsym(d->dvm_hand, "_Z20dvmSuspendAllThreads12SuspendCause");
		d->dvmResumeAllThreads_fnPtr = mydlsym(d->dvm_hand, "_Z19dvmResumeAllThreads12SuspendCause");
		d->dvmAttachCurrentThread_fnPtr = mydlsym(d->dvm_hand, "_Z22dvmAttachCurrentThreadPK16JavaVMAttachArgsb");
		d->dvmMterpPrintMethod_fnPtr = mydlsym(d->dvm_hand, "dvmMterpPrintMethod");		
		d->dvmIsStaticMethod_fnPtr = mydlsym(d->dvm_hand, "_Z17dvmIsStaticMethodPK6Method");
		d->dvmAllocObject_fnPtr = mydlsym(d->dvm_hand, "dvmAllocObject");
		d->dvmCallMethodV_fnPtr = mydlsym(d->dvm_hand, "_Z14dvmCallMethodVP6ThreadPK6MethodP6ObjectbP6JValueSt9__va_list");
		d->dvmCallMethodA_fnPtr = mydlsym(d->dvm_hand, "_Z14dvmCallMethodAP6ThreadPK6MethodP6ObjectbP6JValuePK6jvalue");
		d->dvmCallMethod_fnPtr = mydlsym(d->dvm_hand, "_Z13dvmCallMethodP6ThreadPK6MethodP6ObjectP6JValuez");
		d->dvmAddToReferenceTable_fnPtr = mydlsym(d->dvm_hand, "_Z22dvmAddToReferenceTableP14ReferenceTableP6Object");
		d->dvmSetNativeFunc_fnPtr = mydlsym(d->dvm_hand, "_Z16dvmSetNativeFuncP6MethodPFvPKjP6JValuePKS_P6ThreadEPKt");
		d->dvmUseJNIBridge_fnPtr = mydlsym(d->dvm_hand, "_Z15dvmUseJNIBridgeP6MethodPv");
		d->dvmDexFileOpenFromFd_fnPtr = mydlsym(d->dvm_hand, "_Z20dvmDexFileOpenFromFdiPP6DvmDex");
		d->sysChangeMapAccess_fnPtr = mydlsym(d->dvm_hand, "_Z18sysChangeMapAccessPvjiP10MemMapping");
		d->dvmDexChangeDex1_fnPtr = mydlsym(d->dvm_hand, "_Z16dvmDexChangeDex1P6DvmDexPhh");
		d->dvmFindStaticField_fnPtr = mydlsym(d->dvm_hand, "_Z18dvmFindStaticFieldPK11ClassObjectPKcS3_");
		d->dvmInvokeMethod_fnPtr = mydlsym(d->dvm_hand, "_Z15dvmInvokeMethodP6ObjectPK6MethodP11ArrayObjectS5_P11ClassObjectb");
		d->dvmCreateCstrFromString_fnPtr = mydlsym(d->dvm_hand, "_Z23dvmCreateCstrFromStringPK12StringObject");
		d->dvmDumpObject_fnPtr = mydlsym(d->dvm_hand, "_Z13dvmDumpObjectPK6Object");
		d->dvmGetStaticFieldObject_fnPtr = mydlsym(d->dvm_hand, "_Z23dvmGetStaticFieldObjectPK11StaticField");
		d->dvmFindClass_fnPtr = mydlsym(d->dvm_hand, "_Z12dvmFindClassPKcP6Object");
		d->dvmFreeClassInnards_fnPtr = mydlsym(d->dvm_hand, "_Z19dvmFreeClassInnardsP11ClassObject");
		d->dvmComputeMethodArgsSize_fnPtr = mydlsym(d->dvm_hand, "_Z24dvmComputeMethodArgsSizePK6Method");
		d->dvmDexGetResolvedMethod_fnPtr = mydlsym(d->dvm_hand, "_Z23dvmDexGetResolvedMethodPK6DvmDexj");
		d->dexReadAndVerifyClassData_fnPtr = mydlsym(d->dvm_hand, "_Z25dexReadAndVerifyClassDataPPKhS0_");
		d->dvmAllocArrayByClass_fnPtr = mydlsym(d->dvm_hand, "dvmAllocArrayByClass");
		d->dvmFindArrayClass_fnPtr = mydlsym(d->dvm_hand, "_Z17dvmFindArrayClassPKcP6Object");
		d->dvmBoxPrimitive_fnPtr  = mydlsym(d->dvm_hand, "_Z15dvmBoxPrimitive6JValueP11ClassObject");
		d->dvmFindPrimitiveClass_fnPtr = mydlsym(d->dvm_hand, "_Z21dvmFindPrimitiveClassc");
		d->dvmWriteBarrierArray_fnPtr = mydlsym(d->dvm_hand, "_Z20dvmWriteBarrierArrayPK11ArrayObjectjj");
		d->dvmLogExceptionStackTrace_fnPtr = mydlsym(d->dvm_hand, "_Z25dvmLogExceptionStackTracev");
		d->dvmCheckException_fnPtr  = mydlsym(d->dvm_hand, "_Z17dvmCheckExceptionP6Thread");
		d->dvmIsNativeMethod_fnPtr = mydlsym(d->dvm_hand, "_Z17dvmIsNativeMethodPK6Method");
		d->dvmFindSystemClass_fnPtr = mydlsym(d->dvm_hand, "_Z18dvmFindSystemClassPKc");
		d->dvmGetBoxedTypeDescriptor_fnPtr = mydlsym(d->dvm_hand, "_Z25dexGetBoxedTypeDescriptor13PrimitiveType");
		d->dvmReleaseTrackedAlloc_fnPtr = mydlsym(d->dvm_hand, "dvmReleaseTrackedAlloc");
		d->dvmGetBoxedReturnType_fnPtr = mydlsym(d->dvm_hand, "_Z21dvmGetBoxedReturnTypePK6Method");
		d->dvmUnboxPrimitive_fnPtr = mydlsym(d->dvm_hand, "_Z17dvmUnboxPrimitiveP6ObjectP11ClassObjectP6JValue");
		d->dvmIsPrimitiveClass_fnPtr = mydlsym(d->dvm_hand, "_Z19dvmIsPrimitiveClassPK11ClassObject");
		d->dvmConvertPrimitiveValue_fnPtr = mydlsym(d->dvm_hand, "_Z24dvmConvertPrimitiveValue13PrimitiveTypeS_PKiPi");
		d->dvmCreateInternalThread_fnPtr = mydlsym(d->dvm_hand, "_Z23dvmCreateInternalThreadPlPKcPFPvS2_ES2_");
		d->dvmMalloc_fnPtr = mydlsym(d->dvm_hand,"_Z9dvmMallocji");
		d->dvmLinearAlloc_fnPtr = mydlsym(d->dvm_hand, "_Z14dvmLinearAllocP6Objectj");
		d->dexProtoGetParameterCount_fnPtr = mydlsym(d->dvm_hand, "_Z25dexProtoGetParameterCountPK8DexProto");
		d->dvmGetArgLong_fnPtr = mydlsym(d->dvm_hand,"_Z13dvmGetArgLongPKji");
		d->dvmGetException_fnPtr = mydlsym(d->dvm_hand, "_Z15dvmGetExceptionP6Thread");
		d->dvmWrapException_fnPtr = mydlsym(d->dvm_hand, "_Z16dvmWrapExceptionPKc");
		d->dvmThrowFileNotFoundException_fnPtr = mydlsym(d->dvm_hand, "_Z29dvmThrowFileNotFoundExceptionPKc");
		d->dvmHumanReadableDescriptor_fnPtr = mydlsym(d->dvm_hand,"_Z26dvmHumanReadableDescriptorPKc"); //not usable
		d->dvmGetMethodThrows_fnPtr = mydlsym(d->dvm_hand, "_Z18dvmGetMethodThrowsPK6Method");
		d->dvmClearException_fnPtr = mydlsym(d->dvm_hand, "_Z17dvmClearExceptionP6Thread");
		d->dvmThrowClassNotFoundException_fnPtr = mydlsym(d->dvm_hand, "_Z30dvmThrowClassNotFoundExceptionPKc");
		d->dvmGetRawDexFileDex_fnPtr = mydlsym(d->dvm_hand, "_Z19dvmGetRawDexFileDexP10RawDexFile");


		if (!d->dvmUseJNIBridge_fnPtr)
			d->dvmUseJNIBridge_fnPtr = mydlsym(d->dvm_hand, "dvmUseJNIBridge");
		d->dvmDecodeIndirectRef_fnPtr =  mydlsym(d->dvm_hand, "_Z20dvmDecodeIndirectRefP6ThreadP8_jobject");
		d->dvmLinearSetReadWrite_fnPtr = mydlsym(d->dvm_hand, "_Z21dvmLinearSetReadWriteP6ObjectPv");
		d->dvmGetCurrentJNIMethod_fnPtr = mydlsym(d->dvm_hand, "_Z22dvmGetCurrentJNIMethodv");
		d->dvmFindInstanceField_fnPtr = mydlsym(d->dvm_hand, "_Z20dvmFindInstanceFieldPK11ClassObjectPKcS3_");
		//d->dvmCallJNIMethod_fnPtr = mydlsym(d->dvm_hand, "_Z21dvmCheckCallJNIMethodPKjP6JValuePK6MethodP6Thread");
		d->dvmCallJNIMethod_fnPtr = mydlsym(d->dvm_hand, "_Z16dvmCallJNIMethodPKjP6JValuePK6MethodP6Thread");		
		d->dvmDumpAllClasses_fnPtr = mydlsym(d->dvm_hand, "_Z17dvmDumpAllClassesi");
		d->dvmDumpClass_fnPtr = mydlsym(d->dvm_hand, "_Z12dvmDumpClassPK11ClassObjecti");
		d->dvmFindLoadedClass_fnPtr = mydlsym(d->dvm_hand, "_Z18dvmFindLoadedClassPKc");
		if (!d->dvmFindLoadedClass_fnPtr)
			d->dvmFindLoadedClass_fnPtr = mydlsym(d->dvm_hand, "dvmFindLoadedClass");
		d->dvmHashTableLock_fnPtr = mydlsym(d->dvm_hand, "_Z16dvmHashTableLockP9HashTable");
		d->dvmHashTableUnlock_fnPtr = mydlsym(d->dvm_hand, "_Z18dvmHashTableUnlockP9HashTable");
		d->dvmHashForeach_fnPtr = mydlsym(d->dvm_hand, "_Z14dvmHashForeachP9HashTablePFiPvS1_ES1_");
		d->dvmInstanceof_fnPtr = mydlsym(d->dvm_hand, "_Z13dvmInstanceofPK11ClassObjectS1_");
		d->gDvm = mydlsym(d->dvm_hand, "gDvm");
	}
}





void* _dvmGetBoxedTypeDescriptor(struct dexstuff_t* d, PrimitiveType p){
	return d->dvmGetBoxedTypeDescriptor_fnPtr(p);
}

void* _dvmFindSystemClass(struct dexstuff_t* d, char* cls){
	return d->dvmFindSystemClass_fnPtr(cls);
}

void* _dvmDecodeIndirectRef(struct dexstuff_t* d, Thread* self, jobject ref){
	return d->dvmDecodeIndirectRef_fnPtr(self, ref);
}


void*  _dvmFindPrimitiveClass(struct dexstuff_t*  d, char *c){
	return d->dvmFindPrimitiveClass_fnPtr(c);
}



void* _dvmFindArrayClass(struct dexstuff_t* d, char* desc, void* loader){
	return  d->dvmFindArrayClass_fnPtr(desc,loader);
}

void* _dvmAllocArrayByClass(struct dexstuff_t* d, struct ClassObject*  arrayClass, size_t length, int allocFlags){
	return d->dvmAllocArrayByClass_fnPtr(arrayClass, length, allocFlags);
}


void* _dvmComputeMethodArgsSize(struct dexstuff_t* d, struct Method* m){
	return d->dvmComputeMethodArgsSize_fnPtr(m);
}
void*  _dvmInitClass(struct dexstuff_t* d, void* cls){\
	log("dvminitclass\n")
	void *res = d->dvmInitClass_fnPtr(cls);
	log("chiamato dvminitclass = %p \n", res)
}

void*  _dvmFindStaticField(struct dexstuff_t* d, struct ClassObject*  cls, char* fname, char* fsig){
	void* res = d->dvmFindStaticField_fnPtr(cls,fname,fsig);
	log("cisono\n")
	log("XXX7 staticfield = 0x%x\n", res)
}

int _dvmDexChangeDex1(struct dexstuff_t *d, struct DvmDex* pdvmD, u1* addr, u1 newVal){
	void* res = d->dvmDexChangeDex1_fnPtr(pdvmD,addr,newVal);
	log("dvmDexChangeDex1 = %p\n", res)

}

int _sysChangeMapAccess(struct dexstuff_t *d, void* addr, size_t length, int want, struct MemMapping* pMap){
	return d->sysChangeMapAccess_fnPtr(addr,length,want,pMap);
}

void* _dvmDexFileOpenFromFd(struct dexstuff_t *d, int cookie, struct DvmDex* pdex){
	int res = d->dvmDexFileOpenFromFd_fnPtr(cookie, &pdex);
	log("XXX7 openfromfd res = 0x%x\n", res)
	return res;
}

//problemi con il passaggio del parametro objName di tipo StringObject*
void*  _loadClass(struct dexstuff_t *d, jobject clsname){
	log("XXX7 sono dentro LOADCLASS = 0x%x\n", &clsname)
	log("DEBUG VMCLASSLOADER = %s\n", d->dvm_java_lang_VMClassLoader[6].name)
	u4 args[2] = { (struct StringObject*) &clsname, (u4)0x0};
	JValue pResult;
	void* res = NULL;
	d->dvm_java_lang_VMClassLoader[6].fnPtr( args , &pResult );
	res = (void*) pResult.l;
	if(res){
		log("XXX7 TROVATA CLS = 0x%x\n", res)
	}
}
struct ClassObject* _getCallerClass(struct dexstuff_t* dex){
	Thread* t = getSelf(dex);
	//log("XXX5 THREAD = 0x%x, field = 0x%x\n", t, t->interpSave.curFrame);
	struct ClassObject* co = dex->dvmGetCallerClass_fnPtr(t->interpSave.curFrame);
	return co;
}
struct ClassObject* _getCaller2Class(struct dexstuff_t* dex){
	Thread* t = getSelf(dex);
	//log("XXX5 THREAD = 0x%x, field = 0x%x\n", t, t->interpSave.curFrame);
	struct ClassObject* co = dex->dvmGetCaller2Class_fnPtr(t->interpSave.curFrame);
	return co;
}
struct ClassObject* _getCaller3Class(struct dexstuff_t* dex){
	Thread* t = getSelf(dex);
	//log("XXX5 THREAD = 0x%x, field = 0x%x\n", t, t->interpSave.curFrame);
	struct ClassObject* co = dex->dvmGetCaller3Class_fnPtr(t->interpSave.curFrame);
	return co;
}

void* _mterprintmethod(struct dexstuff_t *dex){
	struct Method* m = dex->dvmGetCurrentJNIMethod_fnPtr();
	dex->dvmMterpPrintMethod_fnPtr(m);
}
void* _attachCurrentT(struct dexstuff_t  *dex){
	return dex->dvmAttachCurrentThread_fnPtr(NULL,1);
}
void _suspendAllT(struct dexstuff_t *dex){
	dex->dvmSuspendAllThreads_fnPtr(SUSPEND_FOR_DEBUG);
}
void _dumpAllT(struct dexstuff_t *dex){
	dex->dvmDumpAllThreadsb_fnPtr();
}
void _resumeAllT(struct dexstuff_t *dex){
	dex->dvmResumeAllThreads_fnPtr(SUSPEND_FOR_DEBUG);
	//dex->dvmDumpAllThreadsb_fnPtr();
}
void* _tryLockThreadList(struct dexstuff_t *dex){
	return dex->dvmTryLockThreadList_fnPtr();
}
void _unlockThreadList(struct dexstuff_t *dex){
	dex->dvmUnlockThreadList_fnPtr();
}
void _callSuspendThread(struct dexstuff_t *dex){
	tself = getSelf(dex);
	//dex->dvmDumpAllThreadsb_fnPtr();
	log("XXX5 sto sospendendo @0x%x\n", tself)
	dex->dvmSuspendThread_fnPtr(tself);
	//dex->dvmDumpAllThreadsb_fnPtr();
}

const char* _dexGetStringData(const struct DexFile* pDexFile,
        const struct DexStringId* pStringId) {
    const u1* ptr = pDexFile->baseAddr + pStringId->stringDataOff;

    // Skip the uleb128 length.
    while (*(ptr++) > 0x7f) /* empty */ ;

    return (const char*) ptr;
}

void* _dexStringById(const struct DexFile* pDexFile, u4 idx) {
    const struct DexStringId* pStringId = &pDexFile->pStringIds[idx];
    return _dexGetStringData(pDexFile, pStringId);
}
void* _dexGetTypeId(const struct DexFile* pDexFile, u4 idx){
	return &pDexFile->pTypeIds[idx];
}

void* _dexStringByTypeIdx(const struct DexFile* pDexFile, u4 idx){
	const struct DexTypeId* typeId = _dexGetTypeId(pDexFile, idx);
	return _dexStringById(pDexFile, typeId->descriptorIdx);
}

void _dexProtoSetFromMethodId(struct DexProto* pProto,
    const struct DexFile* pDexFile, const struct DexMethodId* pMethodId)
{
    pProto->dexFile = pDexFile;
    pProto->protoIdx = pMethodId->protoIdx;
}

/*
 * Get a copy of the utf-8 encoded method descriptor string from the
 * proto of a MethodId. The returned pointer must be free()ed by the
 * caller.
 */
char* _dexCopyDescriptorFromMethodId(struct dexstuff_t* d, const struct DexFile* pDexFile,
    const struct DexMethodId* pMethodId)
{
    struct DexProto proto;

    _dexProtoSetFromMethodId(&proto, pDexFile, pMethodId);
    return d->dexProtoCopyMethodDescriptor_fnPtr(&proto);
}

void _dumpMethod(struct dexstuff_t* d,struct DexFile* pDexFile, const struct DexMethod* pDexMethod, int i , char* classDescriptor, JNIEnv* env){
	log("DUMP METHOD CHIAMATO \n")
    const struct DexMethodId* pMethodId;
    const char* backDescriptor;
    const char* name;
    char* typeDescriptor = NULL;
    char* accessStr = NULL;

    pMethodId = &pDexFile->pMethodIds[pDexMethod->methodIdx];//dexGetMethodId(pDexFile, pDexMethod->methodIdx);
    name = _dexStringById(pDexFile, pMethodId->nameIdx);
    typeDescriptor = _dexCopyDescriptorFromMethodId(d,pDexFile, pMethodId);

    backDescriptor = _dexStringByTypeIdx(pDexFile, pMethodId->classIdx);

    log("DUMP METHOD, name = %s, type = %s \n", name, typeDescriptor)
    if(!strstr(name,"run"))
    	createAndInsertDhook(env, classDescriptor, name, typeDescriptor);
    
}

void _dumpClass(struct dexstuff_t* d, struct DexFile* pDexFile, int idx, JNIEnv* env,  char* targetCls){
	struct DexClassDef* pClassDef;
	struct DexClassData* pClassData = NULL;
	const u1* pEncodedData;
	const char* classDescriptor;
	int i;
	pClassDef = &pDexFile->pClassDefs[idx];
	if (pClassDef->classDataOff == 0)
        pEncodedData = NULL;
    else
    	pEncodedData = (const u1*) (pDexFile->baseAddr + pClassDef->classDataOff);
	pClassData = d->dexReadAndVerifyClassData_fnPtr(&pEncodedData, NULL);
	classDescriptor = _dexStringByTypeIdx(pDexFile, pClassDef->classIdx);

    if (!(classDescriptor[0] == 'L' &&
          classDescriptor[strlen(classDescriptor)-1] == ';')){
    	log("DUMPCLASS ERRORE FORMATO CLASSE \n")
    	return;
    }
    else{
    	//log("DUMPCLASS OTTENUTO DESCRIPTOR = %s\n", classDescriptor)
    	struct ClassObject* pCo  = d->dvmFindLoadedClass_fnPtr(classDescriptor);
    	if(!pCo){
    		//log("DUMP CLASS NON HO TROVATO CLASS: %s\n", classDescriptor)
    		return;
    	}
    	else{
    		if(!strstr(classDescriptor, targetCls))
    			return;
    		log("DUMPCLASS HO TROVATO CLASSE: %s\n", classDescriptor)
    	}
    }
    /**
    for (i = 0; i < (int) pClassData->header.directMethodsSize; i++) {
       //_dumpMethod(d,pDexFile, &pClassData->directMethods[i], i,classDescriptor, env);
    }
    */

    for (i = 0; i < (int) pClassData->header.virtualMethodsSize; i++) {
        _dumpMethod(d,pDexFile, &pClassData->virtualMethods[i], i,classDescriptor,env);
    }
}

void countMethods(struct ClassObject* pp){
	log("DENTRO COUNTMETHODS\n")
	int i =0;
	for(i=0;i<pp->vtableCount;i++){
		log("name: %s, insns=%x, native=%x\n", pp->vtable[i]->name, pp->vtable[i]->insns, pp->vtable[i]->nativeFunc)
	}
}

void prova(struct dexstuff_t* d, void* cookie){
	log("DENTRO PROVA, cookie = %x\n", cookie)
	struct DexOrJar* pDexOrJar = (struct DexOrJar*)cookie;
	log("cookie = 0x%x, pdexmem = 0x%x, %s\n", cookie, pDexOrJar->pDexMemory, pDexOrJar->fileName)
	log("rawdexfile %x \n", pDexOrJar->pRawDexFile)
	int i = 0;
	struct DexClassDef* pClassDef;
	struct DexClassData* pClassData = NULL;
	const u1* pEncodedData;
	const char* classDescriptor;
	struct DvmDex* apDvmDex;
	struct DexFile* pDexFile;
	log("provo a chiamare dvmgetrawdexfile\n")
	//pDvmDex = d->dvmGetRawDexFileDex_fnPtr(pDexOrJar->pRawDexFile);
	apDvmDex = pDexOrJar->pRawDexFile->pDvmDex;
    if(apDvmDex == NULL){
    	return;
    }
    log("PRESO pDVMDEX %x\n", apDvmDex)
    log("mappedreadonly %x, header %x \n", apDvmDex->isMappedReadOnly, apDvmDex->pHeader)
    log("dexFile %x\n", apDvmDex->pDexFile)
    pDexFile = apDvmDex->pDexFile;
    struct DexClassLookup* pLookup = pDexFile->pClassLookup;
    log("CLASSLOOKUP num: %d\n", pLookup->numEntries)


    return;
}
void _dumpClass2(struct dexstuff_t* d, struct DexFile* pDexFile, int idx, void* cookie){
	struct DexClassDef* pClassDef;
	struct DexClassData* pClassData = NULL;
	const u1* pEncodedData;
	const char* classDescriptor;
	
	int i;
	pClassDef = &pDexFile->pClassDefs[idx];
	if (pClassDef->classDataOff == 0)
        pEncodedData = NULL;
    else
    	pEncodedData = (const u1*) (pDexFile->baseAddr + pClassDef->classDataOff);
	pClassData = d->dexReadAndVerifyClassData_fnPtr(&pEncodedData, NULL);
	classDescriptor = _dexStringByTypeIdx(pDexFile, pClassDef->classIdx);
	char * pointer = malloc((sizeof(char) * strlen(classDescriptor)) + 1);
    if (!(classDescriptor[0] == 'L' &&
          classDescriptor[strlen(classDescriptor)-1] == ';')){
    	log("DUMPCLASS ERRORE FORMATO CLASSE \n")
    	return;
    }
    else{
    	log("DUMPCLASS HO TROVATO CLASSE: %s\n", classDescriptor)
    	if(strstr(classDescriptor, "Lorg/tesi/") || strstr(classDescriptor, "Lorg/sid/")){
			memcpy(pointer, classDescriptor, sizeof(char)*strlen(classDescriptor));
			pointer[strlen(pointer)-1] = '\0';
			pointer = pointer++;
			log("chiamo con: %s\n", pointer)
    		dexstuff_defineclass(d, pointer, cookie);
    	}
    }
}
void prova2(struct dexstuff_t* d, struct ClassObject* pCo, void* cookie){
	int i = (int) pCo->pDvmDex->pDexFile->pHeader->classDefsSize;
	log("prova2, dex contiene %d classi \n", i)
	for (i = 0; i < (int) pCo->pDvmDex->pDexFile->pHeader->classDefsSize; i++) {
        _dumpClass2(d,pCo->pDvmDex->pDexFile, i, cookie);
    }
}

void handleAllMethodClass(struct dexstuff_t* d, JNIEnv* env){
	log("DENTRO HANDLEALLMETHODCLASS\n")
	jthrowable exc;
	//struct ClassObject* pp = d->dvmFindLoadedClass_fnPtr("Lcourse/labs/activitylab/ActivityOne;");
	struct ClassObject* pp = d->dvmFindLoadedClass_fnPtr("Lappinventor/ai_garikoitzmartinez/crackme01/Screen1;"); //prendo il classloader
	//countMethods(pp);

	if(!pp){
		log("ALLMETHODCLASS NON HO TROVATO MAIN CLASS\n")
		return 0;
	}
	size_t i,k;
	struct DexClassDef* pCd  = NULL;
	char* targetCls = "Lappinventor/";
	dalvik_dump_class(d, "Lappinventor/ai_garikoitzmartinez/crackme01/Screen1;");
	//devo fare un  for per ogni metodo in ogni classe
	//struct Method** ppM =  pp->pDvmDex->pResMethods;
	for (i = 0; i < (int) pp->pDvmDex->pDexFile->pHeader->classDefsSize; i++) {
        _dumpClass(d,pp->pDvmDex->pDexFile, i, env, targetCls);
    }
    return;

	for(k=0; k < (int) pp->pDvmDex->pDexFile->pHeader->methodIdsSize;k++){
		struct DexMethodId*  pMethId = &(pp->pDvmDex->pDexFile->pMethodIds[k]);
		struct DexStringId* pDexSname = &(pp->pDvmDex->pDexFile->pStringIds[pMethId->nameIdx]);	
		struct DexProtoId* pDexProtosig = &(pp->pDvmDex->pDexFile->pProtoIds[pMethId->protoIdx]);
		struct DexStringId* pDexSsig = &(pp->pDvmDex->pDexFile->pStringIds[pDexProtosig->shortyIdx]);
		u1* ptr = pp->pDvmDex->pDexFile->baseAddr  + pDexSname->stringDataOff; 
		u1* ptr2 =  pp->pDvmDex->pDexFile->baseAddr  + pDexSsig->stringDataOff; 

		// Skip the uleb128 length.
   		while (*(ptr++) > 0x7f) /* empty */ ;
		// Skip the uleb128 length.
   		while (*(ptr2++) > 0x7f) /* empty */ ;
		struct Method *pM = d->dvmDexGetResolvedMethod_fnPtr(pp->pDvmDex,k);
		if(pM  != NULL)
			log("handleallmethod trovato  metodo* = %p, nome = %s\n", pM, pM->name)
   		log("handleallmethodclass, nome metodo = %s, signature = %s\n", ptr, ptr2)
	}

}

void diosolo(struct dexstuff_t* d, JNIEnv *env){
	//handleAllMethodClass(d,env);
	char* commName = "Lcourse/labs";
	//struct ClassObject* pp = d->dvmFindLoadedClass_fnPtr("Lcom/whatsapp/Main;");
	struct ClassObject* pp = d->dvmFindLoadedClass_fnPtr("Lcom/tinder/managers/ManagerApplication;");
	if(!pp){
		log("diosolo  NON HO TROVATO MAIN CLASS\n")
		//dalvik_dump_class(d, "");
		return;
	}
	size_t k = 0;
	struct DexClassDef* pCd  = NULL;
	jthrowable exc;
	log("diosolo sto per entrare nel for, #classi = %d\n", pp->pDvmDex->pDexFile->pHeader->classDefsSize)
	for(k=0; k < (int) pp->pDvmDex->pDexFile->pHeader->classDefsSize;k++){
		log("diosolo inizio for prendo dexclassdef\n")
		pCd = &(pp->pDvmDex->pDexFile->pClassDefs[k]);
		log("diosolo inizio for prendo dextypeid\n")
		struct DexTypeId* pTypeId = &(pp->pDvmDex->pDexFile->pTypeIds[k]);
		log("diosolo inizio for prendo dexstringid\n")
		struct DexStringId* pStringId = &(pp->pDvmDex->pDexFile->pStringIds[pTypeId->descriptorIdx]);
		u1* ptr = pp->pDvmDex->pDexFile->baseAddr  + pStringId->stringDataOff;    
		// Skip the uleb128 length.
   		while (*(ptr++) > 0x7f) /* empty */ ;
   		log("diosolo nome class = %s \n", ptr);
   		if(strstr(ptr, commName)  == NULL)
   			continue;
		struct ClassObject* pC = d->dvmFindLoadedClass_fnPtr(ptr);
		exc = (*env)->ExceptionOccurred(env);
  		if (exc) {
  			(*env)->ExceptionClear(env);
  		}
		if(!pC){
			log("diosolo classe %s non trovata!!!! chiamo dvmfindclass\n", ptr)
			//continue;
			pC = d->dvmFindClass_fnPtr(ptr,pp->classLoader);
			exc = (*env)->ExceptionOccurred(env);
  			if (exc) {
  				(*env)->ExceptionClear(env);
  			}
			log("chiamato dvmfindclass = %p\n", pC)
		}
		if(pC){
			//dalvik_dump_class(d,cls);
			//controllo lo stato della  classe e stampo il classLoader
			log("diosolo, status  %s = %d, classLoader = %p, sfieldcount = %d, instancefieldcount = %d \n",ptr,pC->status,pC->classLoader, pC->sfieldCount, pC->ifieldCount)
			if(pC->status < CLASS_INITIALIZED){
				log("diosolo CLASSE %s NON INIZIALIZZATA chiamo CLASSS INIT\n", ptr)
				continue;
				_dvmInitClass(d,pC);
			}
			//log("diosolo, status  classe = %d, classLoader = %p, sfieldcount = %d, instancefieldcount = %d \n",pC->status,pC->classLoader, pC->sfieldCount, pC->ifieldCount)
			log("Il package ha:  %d metodi diretti, %d metodi virtuali \n", pC->directMethodCount, pC->virtualMethodCount)
			//devo fare un loop sugli staticfield
			size_t j = 0;
			for(j=0;j<pC->sfieldCount;j++){
				//log("diosolo staticfield della classs [%2d]:  %20s, %s  \n", j,pC->sfields[j].f.name, pC->sfields[j].f.signature);
				if(strcmp(pC->sfields[j].f.signature, "[Ljava/lang/String;") == 0){
					struct StaticField* pId = d->dvmFindStaticField_fnPtr(pC, pC->sfields[j].f.name, "[Ljava/lang/String;");
					if(pId){
						// /d->dvmDumpObject_fnPtr(pId);
						log(" diosolo preso staticfield Id 0x%x, signature = %s \n", pId,  pId->f.signature)
						struct ArrayObject* pO = (struct ArrayObject*) d->dvmGetStaticFieldObject_fnPtr(pId);
						log("diosono ci sono1\n")
						log("diosolo arrayobj length = %d\n", pO->length)
						struct Object** argsArray = (struct Object**)(void*)pO->contents;
						size_t i = 0;
						char* c= NULL;
					    for (i = 0; i < pO->length; ++i) {
					        //log("pos[%d] = %s \n",i,argsArray[i]->clazz->descriptor);
					        //log("diosolo provo a ottenere char*\n")
					        c = d->dvmCreateCstrFromString_fnPtr((struct StringObject*)argsArray[i]);
					        log("diosolo [%d] preso char = %s\n", i, c)
					    }
					}
					else{
						log("diosolo non ho trovato staticfield\n")
					}
				}
			}
			log("XXX7 DIO SOLO FINITO FOR STATIFIELDS\n")

/**
		//JNI NON SI CAGA DALVIK
		log("diosolo ci sono\n")
		jobject gco =  (*env)->NewGlobalRef(env, co);
		log("diosolo ci sono1\n")
		int stringCount = (*env)->GetArrayLength(env, gco);
		log("diosolo array length = %d\n", stringCount)


		//JNI findClass dice java.lang.NoClassDefFoundError
		jclass cls = (*env)->FindClass(env,"com/whatsapp/Main");
		log(" diosolo trovata classe con JNI = %p \n", cls)
		jfieldID fid = (*env)->GetStaticFieldID(env, cls, "z", "[Ljava/lang/String;");
		log("diosolo trovato fieldID con JNI = %p\n", fid)

		if(pId){
			log("diosolo trovato static field\n")
			void* pCinit = d->dvmFindDirectMethodByDescriptor_fnPtr(pC, "<clinit>", "()V");
			struct Method* pM = (struct Method*)pCinit;
			log("diosolo shorty = %s\n", pM->shorty)
			if(pCinit){
				log("diosolo trovato clinit 0x%x\n", pCinit)
				void* pResult;
				//d->dvmCallMethod_fnPtr(getSelf(d),pCinit, NULL, false,NULL,NULL);
				log("diosolo dopo invoke method\n");
			}
		}
		*/
		}
		else{
			log("diosolo classe non trovata %s!!!!\n", ptr)
			continue;
		}
	//d->dvmFreeClassInnards_fnPtr(pC);
	}
	log("XXX7 FINITO DIO SOLO!!!!!!!!!!!!!!!!!!! \n")
	handleAllMethodClass(d,env);
}
void* _dvmFindLoadedClass(struct dexstuff_t *dex, char *clsname){
	log("XXX7 chiamato dvmFindLoadedClass con %s\n", clsname);
	struct ClassObject* co = dex->dvmFindLoadedClass_fnPtr(clsname);
	log("findloadedclass ci sono = %p\n", co)
	if(co)
		log("XXX7 ritorno cls = 0x%x, clazzloader = 0x%x\n", co, co->classLoader)
	return (void*)co;
}

void _suspendSelf(struct dexstuff_t *dex){
	dex->dvmSuspendSelf_fnPtr();
}

void _resumeThread(struct dexstuff_t *dex){
	void * th = getSelf(dex);
	log("XXX4 resume di @0x%x, tself @0x%x\n", th, tself)
	dex->dvmResumeThread_fnPtr(tself);
}

jclass  myfind_loaded_class(struct dexstuff_t *dex, char* clname, char* met_n, char* met_sig){
	jclass cls = dex->dvmFindLoadedClass_fnPtr(clname);
	if(!cls) 
		return 0;
	void *met = dex->dvmFindVirtualMethodHierByDescriptor_fnPtr(cls, met_n, met_sig);
	if(!met)
		met = dex->dvmFindDirectMethodByDescriptor_fnPtr(cls, met_n, met_sig);
	//*obj = dex->dvmAllocObject_fnPtr(cls, 0x0);
	return cls;
}

void dump_jni_ref_tables(struct dexstuff_t *dex){
	dex->dvmDumpJniReferenceTablesv_fnPtr();
}

void * get_jni_for_thread(struct dexstuff_t *dex){
	void* jni = dex->dvmGetJNIEnvForThreadv_fnPtr();
	if(jni)
		return jni;
	else
		return -1;
}

void * get_method(struct dexstuff_t *dex, char* name, char * desc){
	struct Method* h = dex->dvmGetCurrentJNIMethod_fnPtr();
	strcpy(name,h->clazz->descriptor);
	strcat(name,h->name);
	char *str = dex->dexProtoCopyMethodDescriptor_fnPtr(&h->prototype);
	log("XXX get_method NAME = %s, DESCRIPTOR = %s\n", h->name, str)
	log("XXX4 prova = %s\n", h->clazz->descriptor)
	strcpy(desc,str);
}
int is_static(struct dexstuff_t *dex, struct Method *m){
	return dex->dvmIsStaticMethod_fnPtr(m);
}
void* dexstuff_loaddex(struct dexstuff_t *d, char *path)
{
	jvalue pResult;
	//jint result;
	struct DexOrJar* result;
	log("dexstuff_loaddex, path = 0x%x, %s\n", path, path)
	void *jpath = d->dvmStringFromCStr_fnPtr(path, strlen(path), ALLOC_DEFAULT);
	
	u4 args[2] = { (u4)jpath, (u4)NULL };
	//log("XXX5 NOME0 = %s\n", d->dvm_dalvik_system_DexFile[0].name)
	d->dvm_dalvik_system_DexFile[0].fnPtr(args, &pResult);
	//result = (jint) pResult.l;
	result = (struct DexOrJar*) pResult.l;


	return result;
}

void* dexstuff_getStackClass2(struct dexstuff_t *d){
	jvalue pResult;
	u4 args[0] = {};	
	d->dvm_dalvik_system_VMStack[1].fnPtr(args, &pResult);
	jobject* ret = pResult.l;
	return ret;
}
void* dexstuff_defineclass(struct dexstuff_t *d, char *name, int cookie)
{
	u4 *nameObj = (u4*) name;
	jvalue pResult;
	
	log("dexstuff_defineclass: %s using %x\n", name, cookie)
	
	void* cl = d->dvmGetSystemClassLoader_fnPtr();
	struct Method *m = d->dvmGetCurrentJNIMethod_fnPtr();
	log("sys classloader = 0x%x\n", cl)
	log("cur m classloader = 0x%x\n", m->clazz->classLoader)
	
	void *jname = d->dvmStringFromCStr_fnPtr(name, strlen(name), ALLOC_DEFAULT);
	//log("called string...\n")

	u4 args[3] = { (u4)jname, (u4) m->clazz->classLoader, (u4) cookie };
	d->dvm_dalvik_system_DexFile[3].fnPtr( args , &pResult );
	log("cur m classloader = 0x%x\n", m->clazz->classLoader)

	jobject *ret = pResult.l;
	
	log("class = 0x%x\n", ret)
	
	return ret;
}

void* _setSelf(struct dexstuff_t*dex){
	tself = dex->dvmThreadSelf_fnPtr();
}

Thread* getSelf(struct dexstuff_t *d)
{
	Thread* self  = d->dvmThreadSelf_fnPtr();
	log("GETSELF = %p \n", self)
	return self;
}

void dalvik_dump_class(struct dexstuff_t *dex, char *clname)
{
	if (strlen(clname) > 0) {
		void *target_cls = dex->dvmFindLoadedClass_fnPtr(clname);
		if (target_cls)
			dex->dvmDumpClass_fnPtr(target_cls, (void*)1);
	}
	else {
		dex->dvmDumpAllClasses_fnPtr(0);
	}
}
void _dalvik_dump_class(struct dexstuff_t *dex, void *cls){
	dex->dvmDumpClass_fnPtr((struct ClassObject*)cls,(void*)1);
}


