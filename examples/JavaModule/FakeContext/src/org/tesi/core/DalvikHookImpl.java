package org.tesi.core;

import org.sid.addi.core.DEXHook;
import org.sid.addi.core.DalvikHook;
import org.sid.addi.core.HookT.HookType;


public class DalvikHookImpl extends DalvikHook {
	protected final static String _TAG ="Hooks";
	protected final static String _ETAG = "Hooks-Errors";

	public DalvikHookImpl(String clname, String method_name, String method_sig, String dex_method, String dex_class,DEXHook ih, HookType t){
		super(clname, method_name, method_sig, dex_method, dex_class,ih,  t );
	}
	protected DalvikHookImpl(){}
	

}
