package org.tesi.core;

import java.util.List;

import org.sid.addi.core.DalvikHook;
import org.sid.addi.core.manageADDI;
import org.tesi.Hooks.HookList;

/**
 * @author Valerio Costamagna
 *
 */
public class MyInit {
	public List<DalvikHook> _myhooks;
	
	@SuppressWarnings("unchecked")
	public MyInit(){
		_myhooks = (List<DalvikHook>) HookList._hookList;
	}
	public void place_hook(){
		manageADDI.setmyHooks(_myhooks);
		manageADDI.writeXMLHooks("");
		for (final DalvikHook elem : _myhooks) {
			//Log.i(_TAG,"DEBUG2:::: " + elem.get_clname() + elem.get_method_name() + elem.get_method_sig());
			//createStruct(elem);
			manageADDI._init(elem);
		}		
	}

}
