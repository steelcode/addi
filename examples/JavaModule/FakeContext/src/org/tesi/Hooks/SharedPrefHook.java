package org.tesi.Hooks;

import org.tesi.fakecontext.AppContextConfig;
import org.tesi.fakecontext.DalvikHookImpl;

import android.util.Log;
//import org.tesi.fakecontext.DalvikHook;

public class SharedPrefHook extends DalvikHookImpl{
	public static Object thiz = null;
	
	public static void setThiz(Object t){
		thiz = t;
	}
	
	public static void check(Object... args){
		/**
		if(!checkContext())
			AppContextConfig.getcon();
			*/
	}
	
	public static void checkSharedPref(Object... args){
		check(args);
		String prefName = (String) args[0];
		Log.i(_TAG,"Preference Name "+ args[0]);
		Log.i(_TAG,"### PREF:"+ AppContextConfig.getPackageName() + 
				":getSharedPref:"+prefName);
	}
	public static void getSharedPref(Object... args){
		check(args);
		String prefName = (String) args[0]; // name of pref to retrieve
		if (prefName == null) {
			return;
		}
		/** FUNZIONAAAA
		SharedPreferences s = (SharedPreferences) thiz;
		String res = s.getString(prefName, "");
		Log.i(_TAG, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA ritornato: "+res);
		*/
		// args[1] is the default value 
		String out = "### PREF:"+AppContextConfig._packageName + 
						":getSharedPref:"+ 
						"; name: [" + args[0] + "]" +
						", default: [" + args[1] + "]";
		Log.i(_TAG,out);
	}
	

}
