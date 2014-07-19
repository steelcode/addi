/**
 * @author bellone
 *
 */
package org.tesi.Hooks;

import org.sid.addi.core.DEXHook;

/**
 * @author bellone
 *
 */
public class AntiEmulatorDetectionHook extends DEXHook {
	
	public void foolAntiEmulatorStatic(){
		
	}
	public String myGetDeviceId(Object[] args){
		return AntiEmulator.randomDeviceId();
	}
	public String myGetSubscriberId(Object[] args){
		return AntiEmulator.randomDeviceId();
	}
	public String myGetLine1Number(Object[] args){
		return AntiEmulator.randomDeviceId();
	}
	public String myGetNetworkOperatorName(){
		return AntiEmulator.randomDeviceId();
	}
	public String myGet(Object[] args){
		System.out.println("HO RICEVUTO STRINGA: "+(String)args[0]);
		return "32m";
	}
	public String myGet2(Object[] args){
		System.out.println("HO RICEVUTO STRINGA: "+(String)args[0]);

		return "32m";
	}

}
