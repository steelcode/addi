package org.sid.addi.core;

import org.sid.addi.Logs.LoggerConfig;

import android.util.Log;


public class DEXHook extends LogWrapper {
	protected DalvikHook _hook = null;
	protected String _TAG = LoggerConfig._TAG;
	protected String _TAG_ERROR = LoggerConfig._TAG_ERROR;
	protected String _className, _methodName, _type, _hash;
	protected String _packageName, _dataDir;
	protected String _notes;
	protected Object _thiz = null;
	protected Object[] _args;
	
	
	public DalvikHook getDalvikHook(){
		return _hook;
	}
	public String get_className() {
		return _className;
	}
	public void set_className(String _className) {
		this._className = _className;
	}
	public String get_methodName() {
		return _methodName;
	}
	public void set_methodName(String _methodName) {
		this._methodName = _methodName;
	}

	protected DEXHook(){}
	public void init(DalvikHook d){
		_hook = d;
		_className = d.get_clname();
		_methodName = d.get_method_name();
		_hash = d.get_hashvalue();
		_thiz = d.getThiz();
		Log.i(_TAG, "DEXHOOK INIT SONO "+_className+" nome metodo: "+_methodName);
		_logInit(d);
		
	}

	
	
}
