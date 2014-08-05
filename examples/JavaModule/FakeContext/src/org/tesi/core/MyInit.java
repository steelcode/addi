package org.tesi.core;

import java.util.LinkedList;
import java.util.List;

import org.sid.addi.core.DalvikHook;
import org.sid.addi.core.manageADDI;
import org.tesi.Hooks.HookList;

/**
 * @author Valerio Costamagna
 *
 */
public class MyInit {
	public static List<DalvikHook> _myhooks;
	public static List<DalvikHook> _failedHooks = new LinkedList<DalvikHook>();
	@SuppressWarnings("unchecked")
	
	//cambiato il costruttore
	public static void _MyInit(){
		_myhooks = (List<DalvikHook>) HookList._hookList;
	}
	public static void searchFailedHookWithClsName(String clsname){
		System.out.println("CHIAMANTO SEARCH FAILED CON "+clsname);
		if(_failedHooks.isEmpty()){
			System.out.println("LA LISTA DEI FAILED HOOKS E VUOTA!!!!");			
			return;
		}
		for(DalvikHook t : _failedHooks){
			//System.out.println(t.get_clname());
			if(t.get_clname().equals(clsname)){
				placeAnHook(t);
				//questo non e da fare, multithread!!
				//_failedHooks.remove(t);
			}		
		}
	}
	public static void placeAnHook(DalvikHook d){		
		int i = manageADDI._init(d);
		System.out.println("CHIAMATO PLACE AN HOOK, ret= "+i);
	}
	//cambiato messo statico
	public static void place_hook(){
		_MyInit();
		System.out.println("CHIAMATO PLACE HOOK1");
		manageADDI.setmyHooks(_myhooks);
		System.out.println("CHIAMATO PLACE HOOK2");
		//manageADDI.writeXMLHooks("");
		for (final DalvikHook elem : _myhooks) {
			System.out.println("CHIAMATO PLACE HOOK3");
			//createStruct(elem);
			int error = manageADDI._init(elem);
			if(error == 1){
				//l'hook non è stato aggiunto
				elem.setError(error);
				_failedHooks.add(elem);
			}
		}		
	}

}

