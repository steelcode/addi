package org.tesi.fakecontext;

import org.sid.addi.core.DalvikHook;

public class DalvikHookImpl extends DalvikHook {
	protected final static String _TAG ="Hooks";
	protected final static String _ETAG = "HooksErrors";

	public DalvikHookImpl(String clname, String method_name, String method_sig, String dex_method, String dex_class,int num_args, int mya, int skip){
		super(clname, method_name, method_sig, dex_method, dex_class, num_args, mya, skip );
	}
	protected DalvikHookImpl(){}
	

}
