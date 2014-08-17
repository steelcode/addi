package org.tesi.Hooks;


import android.content.SharedPreferences;
import android.util.Log;

public class SharedPrefHook {
	public  static void getSharedPref(Object... args){
		System.out.println("GET SHARED PREF CHIAMATO");
		Object thiz = args[0];
		
		String prefName = (String) args[1]; // name of pref to retrieve
		if (prefName == null) {
			return;
		}
		/** FUNZIONAAAA
		SharedPreferences s = (SharedPreferences) thiz;
		String res = s.getString(prefName, "");
		Log.i(_TAG, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA ritornato: "+res);
		*/
		// args[1] is the default value 
		String out = "### PREF:"+ 
						":getSharedPref:"+ 
						"; name: [" + args[0] + "]" +
						", default: [" + args[1] + "]";
		System.out.println(out);

		Object o = null;
		try {
			//o = _hookInvoke(args);
			SharedPreferences s = (SharedPreferences) thiz;
			String res = s.getString((String)args[1],(String) args[2]);
			System.out.println("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA ritornato: "+res);
		} catch (Throwable e) {
			// this may throw if incorrect type specified in the code
			Log.w("IntrospyLog", "error in Intro_GET_SHARED_PREF: "+e);
		}
		System.out.println("GET SHARED PREF FINE");
	}
}
