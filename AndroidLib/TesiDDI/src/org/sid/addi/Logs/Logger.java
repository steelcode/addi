/*******************************************************************************
 * Advanced Dalvik Dynamic Instrumentation Android Library
 * 
 * (c) 2014, 
 ******************************************************************************/
package org.sid.addi.Logs;

import org.sid.addi.core.DEXHook;
import org.sid.addi.core.DalvikHook;

import android.util.Log;

public class Logger extends LogToDB{
	private void clean() {
		_out = "";
		_pListArgsBody = "";
		_pListRetBody = "";
	}

	protected void _log(String data) {
		_out += data;
	}
	
	// ####### public

	public void logInit(DalvikHook config) {
		_config = config;
	}	
	
	public void logLine(String line) {
		_out += line + "\n";
	}
	
	public void logFlush_I(String notes, DEXHook dexh) {
		_notes = notes;
		_out += notes;
		logFlush_I(dexh);
	}
	public void logFlush_W(String notes,DEXHook dexh) {
		_notes = notes;
		_out += "-> !!! " + notes;
		logFlush_W(dexh);
	}
	
	// use a static ref to synchronize across threads
	public void logFlush_I(DEXHook dexh) {
		_addTraces();
		synchronized (_TAG) {
			if (_enableDB)
				_logInDB("I", dexh);
			else
				Log.i(_TAG, _out);
		}
		clean();
	}
	
	public void logFlush_W(DEXHook dexh) {
		_addTraces();
		synchronized (_TAG) {
			if (_enableDB)
				_logInDB("W", dexh);
			//Log.w(_TAG, _out);
		}
		clean();
	}
	
	public void logParameter(String name, String value) {
		if (_enableDB)
			_logDBParameter(name, value);
	}
	
	public void logParameter(String name, Object value) {
		if (_enableDB)
			_logDBParameter(name, "" + value);
	}
	
	public void logReturnValue(String name, String value) {
		if (_enableDB)
			_logDBReturnValue(name, value);
	}
	
	public void logReturnValue(String name, Object value) {
		if (_enableDB)
			_logDBReturnValue(name, "" + value);
	}
	
	public void logBasicInfo(DEXHook dexh) {
		
		_log("### "+ "CATEGORY"+" ### " + 
				dexh.get_className()+ 
				" - " + dexh.get_className() + "->" 
				+ dexh.get_methodName() + "()\n");
				
	}
}
