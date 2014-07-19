package org.tesi.Hooks;

//import org.tesi.fakecontext.DalvikHook;
import org.sid.addi.core.DEXHook;

import android.util.Log;

public class WebviewHook extends DEXHook{

    public  void webview_set(Object... args){
		// should only display data when first arg is set to true
		
		if ((Boolean)args[0] == true) {
			_logBasicInfo(this);
			_logFlush_W("-> !!! Set of a potentially dangerous " +
					"functionality to true for the webview using "+
					_methodName + ", make sure this " +
					"functionality is necessary", this);
		}
		}
	public  void webview_js(Object... args){
		Log.i(_TAG,"-> !!! Javascript interface " +
				"added for the webview. Details: " +
				args[0] + ", name: " + args[1]);
		
		// this only display data when there is a potential issue
		_logBasicInfo(this);
		_logFlush_W("-> !!! Javascript interface " +
				"added for the webview. Details: " +
				args[0] + ", name: " + args[1], this);
	}
	public void myLoadUrl(Object... args){
		String s = (String)args[0];
		System.out.println("DENTRO MY LOADURL: con url ="+s);
	}
	public void myEvaluateJavascript(Object... args){
		String s = (String)args[0];
		System.out.println("DENTRO MY LOADURL: con script ="+s);
	}
}
