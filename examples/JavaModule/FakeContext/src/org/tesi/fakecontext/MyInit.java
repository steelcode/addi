package org.tesi.fakecontext;

import org.sid.addi.core.*;

import org.tesi.Hooks.HookList;

public class MyInit {
	public DalvikHook[] _myhooks;
	public final String _TAG = "Hook";
	public final String _ETAG = "HookError";
/*
	static {
        System.loadLibrary("strmon");
    }
   */
	
	public MyInit(){
		_myhooks = HookList.getHookList();
	}
	public void place_hook(){
		for (final DalvikHook elem : _myhooks) {
			//Log.i(_TAG,"DEBUG2:::: " + elem.get_clname() + elem.get_method_name() + elem.get_method_sig());
			//createStruct(elem);
			manageADDI._init(elem);
		}		
	}

}
