package org.tesi.Hooks;

//import org.tesi.fakecontext.DalvikHook;
import org.sid.addi.core.DEXHook;

import android.util.Log;

public class WebviewHook extends DEXHook{

    public  void webview_set(Object... args){
			Log.i(_TAG,"-> !!! Set of a potentially dangerous " +
					"functionality to true for the webview using blabla"+
					 ", make sure this " +
					"functionality is necessary");
		}
	public  void webview_js(Object... args){
		Log.i(_TAG,"-> !!! Javascript interface " +
				"added for the webview. Details: " +
				args[0] + ", name: " + args[1]);
	}
}
