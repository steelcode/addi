package org.tesi.Hooks;

import org.tesi.fakecontext.DalvikHookImpl;

import android.util.Log;

public class NetworkHook extends DalvikHookImpl{
	public static void getbyname(Object...args){
		String host = (String)args[0];
		Log.i(_TAG, "Connessione ad host: " + host);
	}
	public static void getbyaddress(Object... args){
		Log.i(_TAG, "chiamato  getbyaddress");
	}
}
