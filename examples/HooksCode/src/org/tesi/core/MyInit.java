package org.tesi.core;

import java.util.List;

import org.sid.addi.core.DalvikHook;
import org.sid.addi.core.manageADDI;
import org.tesi.Hooks.HookList;

public class MyInit {
	static {
		System.load("/data/local/tmp/libdynsec.so");
		System.load("/data/local/tmp/libmstring.so");	
	}
	public static List<DalvikHook> _myhooks;
	
	//cambiato il costruttore
	public static void _MyInit(){
		_myhooks = (List<DalvikHook>) HookList._hookList;
	}
	//cambiato messo statico
	public static void place_hook(){
		_MyInit();
		for (final DalvikHook elem : _myhooks) {
			//System.out.println("CHIAMATO PLACE HOOK3");
			int error = manageADDI._init(elem);
			if(error == 1){
				//l'hook non è stato aggiunto
				;
			}
		}		
	}
}
