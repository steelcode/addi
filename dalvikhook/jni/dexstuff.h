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
#include <pthread.h>

#include "Common.h"



#ifndef __dexstuff_h__
#define __dexstuff_h__

#define ALLOC_DEFAULT  0x00
#define ALLOC_DONT_TRACK 0x02

struct StringObject;
struct ArrayObject;

typedef struct DexProto {
    u4* dexFile;     /* file the idx refers to */
    u4 protoIdx;                /* index into proto_ids table of dexFile */
} DexProto;


typedef void (*DalvikBridgeFunc)(const u4* args, void* pResult,
    const void* method, void* self);

struct Field {
   void*    clazz;          /* class in which the field is declared */
    const char*     name;
    const char*     signature;      /* e.g. "I", "[C", "Landroid/os/Debug;" */
    u4              accessFlags;
};


struct Method;
struct ClassObject;



struct Object {
    /* ptr to class object */
    struct ClassObject*    clazz;

    /*
     * A word containing either a "thin" lock or a "fat" monitor.  See
     * the comments in Sync.c for a description of its layout.
     */
    u4              lock;
};

struct InitiatingLoaderList {
    /* a list of initiating loader Objects; grown and initialized on demand */
    void**  initiatingLoaders;
    /* count of loaders in the above list */
    int       initiatingLoaderCount;
};

enum PrimitiveType {
    PRIM_NOT        = 0,       /* value is a reference type, not a primitive type */
    PRIM_VOID       = 1,
    PRIM_BOOLEAN    = 2,
    PRIM_BYTE       = 3,
    PRIM_SHORT      = 4,
    PRIM_CHAR       = 5,
    PRIM_INT        = 6,
    PRIM_LONG       = 7,
    PRIM_FLOAT      = 8,
    PRIM_DOUBLE     = 9,
} typedef PrimitiveType;

enum ClassStatus {
    CLASS_ERROR         = -1,

    CLASS_NOTREADY      = 0,
    CLASS_IDX           = 1,    /* loaded, DEX idx in super or ifaces */
    CLASS_LOADED        = 2,    /* DEX idx values resolved */
    CLASS_RESOLVED      = 3,    /* part of linking */
    CLASS_VERIFYING     = 4,    /* in the process of being verified */
    CLASS_VERIFIED      = 5,    /* logically part of linking; done pre-init */
    CLASS_INITIALIZING  = 6,    /* class init in progress */
    CLASS_INITIALIZED   = 7,    /* ready to go */
} typedef ClassStatus;

struct ClassObject {
    struct Object o; // emulate C++ inheritance, Collin
	
    /* leave space for instance data; we could access fields directly if we
       freeze the definition of java/lang/Class */
    u4              instanceData[4];

    /* UTF-8 descriptor for the class; from constant pool, or on heap
       if generated ("[C") */
    const char*     descriptor;
    char*           descriptorAlloc;

    /* access flags; low 16 bits are defined by VM spec */
    u4              accessFlags;

    /* VM-unique class serial number, nonzero, set very early */
    u4              serialNumber;

    /* DexFile from which we came; needed to resolve constant pool entries */
    /* (will be NULL for VM-generated, e.g. arrays and primitive classes) */
    void*         pDvmDex;

    /* state of class initialization */
    ClassStatus     status;

    /* if class verify fails, we must return same error on subsequent tries */
    struct ClassObject*    verifyErrorClass;

    /* threadId, used to check for recursive <clinit> invocation */
    u4              initThreadId;

    /*
     * Total object size; used when allocating storage on gc heap.  (For
     * interfaces and abstract classes this will be zero.)
     */
    size_t          objectSize;

    /* arrays only: class object for base element, for instanceof/checkcast
       (for String[][][], this will be String) */
    struct ClassObject*    elementClass;

    /* arrays only: number of dimensions, e.g. int[][] is 2 */
    int             arrayDim;
 	PrimitiveType   primitiveType;

    /* superclass, or NULL if this is java.lang.Object */
    struct ClassObject*    super;

    /* defining class loader, or NULL for the "bootstrap" system loader */
    struct Object*         classLoader;
	
	struct InitiatingLoaderList initiatingLoaderList;

    /* array of interfaces this class implements directly */
    int             interfaceCount;
    struct ClassObject**   interfaces;

    /* static, private, and <init> methods */
    int             directMethodCount;
    struct Method*         directMethods;

    /* virtual methods defined in this class; invoked through vtable */
    int             virtualMethodCount;
    struct Method*         virtualMethods;

    /*
     * Virtual method table (vtable), for use by "invoke-virtual".  The
     * vtable from the superclass is copied in, and virtual methods from
     * our class either replace those from the super or are appended.
     */
    int             vtableCount;
    struct Method**        vtable;

};
	
typedef struct Method {
	struct ClassObject *clazz;
	u4 a; // accessflags
	
	u2             methodIndex;
	
	u2              registersSize;  /* ins + locals */
    u2              outsSize;
    u2              insSize;

    /* method name, e.g. "<init>" or "eatLunch" */
    const char*     name;

    /*
     * Method prototype descriptor string (return and argument types).
     *
     * TODO: This currently must specify the DexFile as well as the proto_ids
     * index, because generated Proxy classes don't have a DexFile.  We can
     * remove the DexFile* and reduce the size of this struct if we generate
     * a DEX for proxies.
     */
    DexProto        prototype;

    /* short-form method descriptor string */
    const char*     shorty;

    /*
     * The remaining items are not used for abstract or native methods.
     * (JNI is currently hijacking "insns" as a function pointer, set
     * after the first call.  For internal-native this stays null.)
     */

    /* the actual code */
    const u2*       insns;      
	
	 /* cached JNI argument and return-type hints */
    int             jniArgInfo;

    /*
     * Native method ptr; could be actual function or a JNI bridge.  We
     * don't currently discriminate between DalvikBridgeFunc and
     * DalvikNativeFunc; the former takes an argument superset (i.e. two
     * extra args) which will be ignored.  If necessary we can use
     * insns==NULL to detect JNI bridge vs. internal native.
     */
    DalvikBridgeFunc  nativeFunc;

#ifdef WITH_PROFILER
    bool            inProfile;
#endif
#ifdef WITH_DEBUGGER
    short           debugBreakpointCount;
#endif

  bool fastJni;

    /*
     * JNI: true if this method has no reference arguments. This lets the JNI
     * bridge avoid scanning the shorty for direct pointers that need to be
     * converted to local references.
     *
     * TODO: replace this with a list of indexes of the reference arguments.
     */
    bool noRef;

	
} Method;




/*
 * Array objects have these additional fields.
 *
 * We don't currently store the size of each element.  Usually it's implied
 * by the instruction.  If necessary, the width can be derived from
 * the first char of obj->clazz->descriptor.
 */
struct ArrayObject  {
    struct Object o;
    /* number of elements; immutable after init */
    u4              length;

    /*
     * Array contents; actual size is (length * sizeof(type)).  This is
     * declared as u8 so that the compiler inserts any necessary padding
     * (e.g. for EABI); the actual allocation may be smaller than 8 bytes.
     */
    u8              contents[1];
}ArrayObject;



typedef void (*DalvikNativeFunc)(const u4* args, jvalue* pResult);

typedef struct DalvikNativeMethod_t {
    const char* name;
    const char* signature;
    DalvikNativeFunc  fnPtr;
} DalvikNativeMethod;


typedef struct InterpSaveState {
    const u2*       pc;         // Dalvik PC
    u4*             curFrame;   // Dalvik frame pointer
    const Method    *method;    // Method being executed
    //DvmDex*         methodClassDex;
    void*         methodClassDex;
    JValue          retval;
    void*           bailPtr;
#if defined(WITH_TRACKREF_CHECKS)
    int             debugTrackedRefStart;
#else
    int             unused;        // Keep struct size constant
#endif
    struct InterpSaveState* prev;  // To follow nested activations
}InterpSaveState;



/*
 * Interpreter control struction.  Packed into a long long to enable
 * atomic updates.
 */
union InterpBreak {
    volatile int64_t   all;
    struct {
        uint16_t   subMode;
        uint8_t    breakFlags;
        int8_t     unused;   /* for future expansion */
#ifndef DVM_NO_ASM_INTERP
        void* curHandlerTable;
#else
        int32_t    unused1;
#endif
    } ctl;
}typedef InterpBreak;


/*
 * Current status; these map to JDWP constants, so don't rearrange them.
 * (If you do alter this, update the strings in dvmDumpThread and the
 * conversion table in VMThread.java.)
 *
 * Note that "suspended" is orthogonal to these values (so says JDWP).
 */
enum ThreadStatus {
    THREAD_UNDEFINED    = -1,       /* makes enum compatible with int32_t */

    /* these match up with JDWP values */
    THREAD_ZOMBIE       = 0,        /* TERMINATED */
    THREAD_RUNNING      = 1,        /* RUNNABLE or running now */
    THREAD_TIMED_WAIT   = 2,        /* TIMED_WAITING in Object.wait() */
    THREAD_MONITOR      = 3,        /* BLOCKED on a monitor */
    THREAD_WAIT         = 4,        /* WAITING in Object.wait() */
    /* non-JDWP states */
    THREAD_INITIALIZING = 5,        /* allocated, not yet running */
    THREAD_STARTING     = 6,        /* started, not yet on thread list */
    THREAD_NATIVE       = 7,        /* off in a JNI native method */
    THREAD_VMWAIT       = 8,        /* waiting on a VM resource */
    THREAD_SUSPENDED    = 9,        /* suspended, usually by GC or debugger */
}typedef ThreadStatus;



/*
 * Our per-thread data.
 *
 * These are allocated on the system heap.
 */
typedef struct Thread {
    /*
     * Interpreter state which must be preserved across nested
     * interpreter invocations (via JNI callbacks).  Must be the first
     * element in Thread.
     */
    InterpSaveState interpSave;

    /* small unique integer; useful for "thin" locks and debug messages */
    u4          threadId;

    /*
     * Begin interpreter state which does not need to be preserved, but should
     * be located towards the beginning of the Thread structure for
     * efficiency.
     */

    /*
     * interpBreak contains info about the interpreter mode, as well as
     * a count of the number of times the thread has been suspended.  When
     * the count drops to zero, the thread resumes.
     */
    InterpBreak interpBreak;
    /*
     * "dbgSuspendCount" is the portion of the suspend count that the
     * debugger is responsible for.  This has to be tracked separately so
     * that we can recover correctly if the debugger abruptly disconnects
     * (suspendCount -= dbgSuspendCount).  The debugger should not be able
     * to resume GC-suspended threads, because we ignore the debugger while
     * a GC is in progress.
     *
     * Both of these are guarded by gDvm.threadSuspendCountLock.
     *
     * Note the non-debug component will rarely be other than 1 or 0 -- (not
     * sure it's even possible with the way mutexes are currently used.)
     */

    int suspendCount;
    int dbgSuspendCount;

    u1*         cardTable;

    /* current limit of stack; flexes for StackOverflowError */
    const u1*   interpStackEnd;

    /* FP of bottom-most (currently executing) stack frame on interp stack */
    void*       XcurFrame;
    /* current exception, or NULL if nothing pending */
    struct Object*     exception;

    bool        debugIsMethodEntry;
    /* interpreter stack size; our stacks are fixed-length */
    int         interpStackSize;
    bool        stackOverflowed;

    /* thread handle, as reported by pthread_self() */
    pthread_t   handle;

    /* Assembly interpreter handler tables */
#ifndef DVM_NO_ASM_INTERP
    void*       mainHandlerTable;   // Table of actual instruction handler
    void*       altHandlerTable;    // Table of breakout handlers
#else
    void*       unused0;            // Consume space to keep offsets
    void*       unused1;            //   the same between builds with
#endif

    /*
     * singleStepCount is a countdown timer used with the breakFlag
     * kInterpSingleStep.  If kInterpSingleStep is set in breakFlags,
     * singleStepCount will decremented each instruction execution.
     * Once it reaches zero, the kInterpSingleStep flag in breakFlags
     * will be cleared.  This can be used to temporarily prevent
     * execution from re-entering JIT'd code or force inter-instruction
     * checks by delaying the reset of curHandlerTable to mainHandlerTable.
     */
    int         singleStepCount;


    /* JNI local reference tracking */
   // IndirectRefTable jniLocalRefTable;

    /*
     * Thread's current status.  Can only be changed by the thread itself
     * (i.e. don't mess with this from other threads).
     */
    volatile ThreadStatus status;

    /* thread ID, only useful under Linux */
    pid_t       systemTid;

    /* start (high addr) of interp stack (subtract size to get malloc addr) */
    u1*         interpStackStart;

    /* the java/lang/Thread that we are associated with */
    struct Object*     threadObj;

    /* the JNIEnv pointer associated with this thread */
    JNIEnv*     jniEnv;

    /* internal reference tracking */
   // ReferenceTable  internalLocalRefTable;


    /* JNI native monitor reference tracking (initialized on first use) */
   // ReferenceTable  jniMonitorRefTable;

    /* hack to make JNI_OnLoad work right */
    struct Object*     classLoaderOverride;

    /* mutex to guard the interrupted and the waitMonitor members */
    pthread_mutex_t    waitMutex;

    /* pointer to the monitor lock we're currently waiting on */
    /* guarded by waitMutex */
    /* TODO: consider changing this to Object* for better JDWP interaction */
  //  Monitor*    waitMonitor;

    /* thread "interrupted" status; stays raised until queried or thrown */
    /* guarded by waitMutex */
    bool        interrupted;

    /* links to the next thread in the wait set this thread is part of */
    struct Thread*     waitNext;

    /* object to sleep on while we are waiting for a monitor */
    pthread_cond_t     waitCond;

    /*
     * Set to true when the thread is in the process of throwing an
     * OutOfMemoryError.
     */
    bool        throwingOOME;

    /* links to rest of thread list; grab global lock before traversing */
    struct Thread* prev;
    struct Thread* next;

    /* used by threadExitCheck when a thread exits without detaching */
    int         threadExitCheckCount;

    /* JDWP invoke-during-breakpoint support */
  //  DebugInvokeReq  invokeReq;

    /* base time for per-thread CPU timing (used by method profiling) */
    bool        cpuClockBaseSet;
    u8          cpuClockBase;

    /* previous stack trace sample and length (used by sampling profiler) */
    const Method** stackTraceSample;
    size_t stackTraceSampleLength;

    /* memory allocation profiling state */
 //   AllocProfState allocProf;

#ifdef WITH_JNI_STACK_CHECK
    u4          stackCrc;
#endif

#if WITH_EXTRA_GC_CHECKS > 1
    /* PC, saved on every instruction; redundant with StackSaveArea */
    const u2*   currentPc2;
#endif

    /* Safepoint callback state */
    pthread_mutex_t   callbackMutex;
 //   SafePointCallback callback;
    void*             callbackArg;

#if defined(ARCH_IA32) && defined(WITH_JIT)
    u4 spillRegion[MAX_SPILL_JIT_IA];
#endif
}Thread;



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
typedef void* (*dvmAddToReferenceTable_func)(void*,void*);
typedef void (*dvmDumpAllClasses_func)(int);
typedef void* (*dvmFindLoadedClass_func)(const char*);

typedef void (*dvmUseJNIBridge_func)(void*, void*);

typedef void* (*dvmDecodeIndirectRef_func)(void*,void*);

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
typedef void* (*dvmGetVirtualizedMethod_func)(void*, const Method*);
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
		
	void *gDvm; // dvm globals !
	
	int done;
};

enum SuspendCause {
    SUSPEND_NOT = 0,
    SUSPEND_FOR_GC,
    SUSPEND_FOR_DEBUG,
    SUSPEND_FOR_DEBUG_EVENT,
    SUSPEND_FOR_STACK_DUMP,
    SUSPEND_FOR_DEX_OPT,
    SUSPEND_FOR_VERIFY,
    SUSPEND_FOR_HPROF,
    SUSPEND_FOR_SAMPLING,
#if defined(WITH_JIT)
    SUSPEND_FOR_TBL_RESIZE,  // jit-table resize
    SUSPEND_FOR_IC_PATCH,    // polymorphic callsite inline-cache patch
    SUSPEND_FOR_CC_RESET,    // code-cache reset
    SUSPEND_FOR_REFRESH,     // Reload data cached in interpState
#endif
};

#endif

void dexstuff_resolv_dvm(struct dexstuff_t *d);
int dexstuff_loaddex(struct dexstuff_t *d, char *path);
void* dexstuff_defineclass(struct dexstuff_t *d, char *name, int cookie);
Thread* getSelf(struct dexstuff_t *d);
void dalvik_dump_class(struct dexstuff_t *dex, char *clname);
int is_static(struct dexstuff_t *, Method *);
void * get_method(struct dexstuff_t *, char* , char * );
void * get_jni_for_thread(struct dexstuff_t *);
