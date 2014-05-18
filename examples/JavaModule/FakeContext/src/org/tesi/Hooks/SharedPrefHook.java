package org.tesi.Hooks;

import org.sid.addi.core.DEXHook;
import org.sid.addi.utils.AppContextConfig;

import android.content.SharedPreferences;
import android.util.Log;
//import org.tesi.fakecontext.DalvikHook;

public class SharedPrefHook extends DEXHook{

	
	public  void checkSharedPref(Object... args){

		String prefName = (String) args[0];
		Log.i(_TAG,"Preference Name "+ args[0]);
		Log.i(_TAG,"### PREF:"+ AppContextConfig.getPackageName() + 
				":getSharedPref:"+prefName);
	}
	@SuppressWarnings("unused")
	public  void getSharedPref(Object... args){
		Log.i(_TAG, "GET SHARED PREF CHIAMATO");
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
		_logParameter("Preference Name", args[0]);
		Object o = null;
		try {
			//o = _hookInvoke(args);
			SharedPreferences s = (SharedPreferences) _thiz;
			String res = s.getString((String)args[0],(String) args[1]);
			Log.i(_TAG, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA ritornato: "+res);
			if (s != null) {
				out += "; retrieves: ["+s+"]";
				_logReturnValue("Value", res);
				_logFlush_I(out, this);
			}
			else {
				_logLine(out);
				_logFlush_I("-> Preference not found or incorrect type specified", this);
			}
		} catch (Throwable e) {
			// this may throw if incorrect type specified in the code
			Log.w("IntrospyLog", "error in Intro_GET_SHARED_PREF: "+e);
		}
		Log.i(_TAG, "GET SHARED PREF FINE");
	}
	public void put_pref(Object... args){
		String prefName = (String) args[0]; // name of pref to retrieve
		_logParameter("Preference Name", args[0]);
		_logParameter("Value", args[1]);
		String out = "### PREF:"+_packageName + ":writeSharedPref:"
						+prefName+", value: "+args[1];
		_logFlush_I(out, this);
	}
	

}
