//http://androidxref.com/4.4.2_r2/xref/dalvik/vm/interp/Stack.h

/*
 * Stack frames, and uses thereof.
 */
#ifndef DALVIK_INTERP_STACK_H_
#define DALVIK_INTERP_STACK_H_


struct StackSaveArea {
#ifdef PAD_SAVE_AREA
    u4          pad0, pad1, pad2;
#endif

#ifdef EASY_GDB
    /* make it easier to trek through stack frames in GDB */
    StackSaveArea* prevSave;
#endif

    /* saved frame pointer for previous frame, or NULL if this is at bottom */
    u4*         prevFrame;

    /* saved program counter (from method in caller's frame) */
    const u2*   savedPc;

    /* pointer to method we're *currently* executing; handy for exceptions */
    const Method* method;

    union {
        /* for JNI native methods: bottom of local reference segment */
        u4          localRefCookie;

        /* for interpreted methods: saved current PC, for exception stack
         * traces and debugger traces */
        const u2*   currentPc;
    } xtra;

    /* Native return pointer for JIT, or 0 if interpreted */
    const u2* returnAddr;
#ifdef PAD_SAVE_AREA
    u4          pad3, pad4, pad5;
#endif
};


#endif  // DALVIK_INTERP_STACK_H_
