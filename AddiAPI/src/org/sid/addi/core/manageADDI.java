package org.sid.addi.core;

public abstract class manageADDI {
	/* sospende tutti i thread tranne il chiamante*/
	public static native void suspendALL();
	/* risveglia tutti i thread */
	public static native void resumeALL();
	/* Inserisce nella lista un DalvikHook*/
	public static native int createStruct(DalvikHook d);
	
	
	public static native void handleAllMethods();
	
	
	public static int _init(DalvikHook d){
		int i = createStruct(d); //1 is error, 0 ok
		return i; 
	}
}
