/**
 * 
 */
package org.sid.addi.core;

import org.sid.addi.core.DalvikHook;

/**
 * @author sid
 *
 */
public abstract class manageADDI {
	
	static {
    System.loadLibrary("strmon");
	}
	
	public static native void createStruct(DalvikHook d);
	public static native void createPTY();
	public static native void suspendALL();
	public static native void resumeALL();
	public static native void unhook(String hook);
	public static native void rehook(String hook);
	public static native void delhook(String hook);

	public static void _init(DalvikHook d){
		createStruct(d);
	}
	
}
