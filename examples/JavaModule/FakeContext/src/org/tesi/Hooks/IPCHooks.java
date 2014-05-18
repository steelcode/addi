package org.tesi.Hooks;

import org.sid.addi.core.DEXHook;

import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
//import org.tesi.fakecontext.DalvikHook;

public class IPCHooks extends DEXHook{
	public static void check(Object... args){
		/*
		if(!checkContext())
			AppContextConfig.getcon();
		else{
			
		}
		*/
	}

	protected  String getExtras(Intent intent) {
		String out = "";
		try {
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

	
	public  void dump_intent(Object... args){
		Log.i(_TAG, "-------------- DUMP INTENT SONO STATO CHIAMATO ------------");
		Intent intent = (Intent) args[0];
		Log.i(_TAG, intent.toString());
		String out = "-> " + intent;	
		_logParameter("Intent", intent);
		String extra = getExtras(intent);
		if (!extra.isEmpty()) {
			Log.i(_TAG, "CONTENUTO EXTRA =  "+extra);
			_logParameter("Extra", extra);
			out += "\n-> Extra: \n" + extra + "";
		}
		_logFlush_I(out, this);
		Log.i(_TAG, "-------------- DUMP INTENT FINE ------------");
	}
	public void ipc_receiver(Object... args){
		_logBasicInfo();
		String out = "";
		
		// arg1 is an intent filter
		IntentFilter intentFilter = (IntentFilter) args[1];
		if (intentFilter != null) {
			out = "-> Intent Filter: \n";
			for (int i = 0; i < intentFilter.countActions(); i++)
				out += "--> [Action "+ i +":"+intentFilter.getAction(i)+"]\n";
			out = out.substring(0, out.length() - 1);
			_logParameter("Intent Filter", out);
		}
		
		// args[2] is the permissions
		if (args.length > 2 && args[2] != null) {
			out += ", permissions: " + args[2];
			_logParameter("Permissions", args[2]);
		}
		_logLine(out);
		
		if (args.length == 2 || (args.length > 2 && args[2] == null))			
			_logFlush_I("-> No permissions explicitely defined for the Receiver", this);
		else
			_logFlush_I(this);
	}
	public void uri_register(Object... args){
		String uriPath = (String)args[1];
		_logParameter("URI Path", uriPath);
		
		String data = "URI:"+_hook.get_method_name()+":"
						+_packageName+uriPath;
		_logBasicInfo();
		_logFlush_I(data, this);
	}
	public void ipc_modified(Object... args){
		// arg1: newState
		int newState = (Integer)args[1];
		if (newState == 
				android.content.pm.PackageManager.COMPONENT_ENABLED_STATE_ENABLED) {
			_logBasicInfo();
			_logParameter("New State", "COMPONENT_ENABLED_STATE_ENABLED");
			_logFlush_W("-> !!! Component ["+ args[0] +
					"] is ENABLED dynamically", this);
		}
	}
	
}
