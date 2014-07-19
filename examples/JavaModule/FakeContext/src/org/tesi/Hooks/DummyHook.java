/**
 * @author Valerio Costamagna
 *
 */
package org.tesi.Hooks;

import org.sid.addi.core.DEXHook;

import android.util.Log;

/**
 * @author Valerio Costamagna
 *
 */
public class DummyHook extends DEXHook{

	public void entrypoint(Object[] args){
		Log.i(_TAG,"DUMMYHOOK!!!!!!");
	}
}
