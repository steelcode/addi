//http://androidxref.com/4.4.2_r2/xref/dalvik/vm/Thread.h

/*
 * VM thread support.
 */
#ifndef DALVIK_THREAD_H_
#define DALVIK_THREAD_H_

#include <pthread.h>

typedef struct InterpSaveState {
    const u2*       pc;         // Dalvik PC
    u4*             curFrame;   // Dalvik frame pointer
    const struct Method    *method;    // Method being executed
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

/* start point for an internal thread; mimics pthread args */
typedef void* (*InternalThreadStart)(void* arg);
/* args for internal thread creation */
struct InternalStartArgs {
    /* inputs */
    InternalThreadStart func;
    void*       funcArg;
    char*       name;
    struct Object*     group;
    bool        isDaemon;
    /* result */
    volatile struct Thread** pThread;
    volatile int*     pCreateStatus;
};


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
    const struct Method** stackTraceSample;
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

#endif  // DALVIK_THREAD_H_