package org.tesi.Hooks;

import java.io.File;

import org.tesi.core.DalvikHookImpl;
import org.tesi.utils.AppContextConfig;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
//import org.tesi.fakecontext.DalvikHook;

public class IPCHooks extends DalvikHookImpl{
	public static void check(Object... args){
		/*
		if(!checkContext())
			AppContextConfig.getcon();
		else{
			
		}
		*/
	}

	protected static String getExtras(Intent intent) {
		Log.i(_TAG, "--------- DENTRO GET EXTRAS -------------");
		String out = "";
		try {
			Log.i(_TAG,"------------- Prendo gli extras");
			Bundle bundle = intent.getExtras();
		    if (bundle != null) {
		    	Log.i(_TAG,"------------- PRESO gli extras");
				for (String key : bundle.keySet()) {
				    Object value = bundle.get(key);
				    out += String.format("--> [%s %s (%s)]\n", key,  
				        value.toString(), value.getClass().getName());
				}
				out = out.substring(0, out.length() - 1);
		    }
		    else{
		    	Log.i(_TAG,"------------- NO extras");
		    }
		}
		catch (Exception e) {
			out = "Cannot get intent extra";
		}
		return out;
	}
	public static void openfile(){
		String FILENAME = "hello_file";
		String string = "hello world!\n";
		File file = new File(AppContextConfig.getContext().getFilesDir(), FILENAME);	
		Log.i(_TAG, "creato file!");
	}
	
	public static void dump_intent(Object... args){
		check(args);
		//openfile();
		Log.i(_TAG, "-------------- SONO STATO CHIAMATO ------------");
		Intent intent = (Intent) args[0];
		Log.i(_TAG, intent.toString());
		String out = "-> " + intent;	
		String extra = getExtras(intent);
		if (!extra.isEmpty()) {
			
			out += "\n-> Extra: \n" + extra + "";
		}
		Log.i(_TAG,out);
	}
}
