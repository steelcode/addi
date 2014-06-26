/*******************************************************************************
 * Advanced Dalvik Dynamic Instrumentation Android Library
 * 
 * (c) 2014, 
 ******************************************************************************/
/**
 * 
 */
package org.sid.addi.Logs;

//import org.tesi.fakecontext.DalvikHook;
import org.sid.addi.core.DalvikHook;
import org.sid.addi.core.StringHelper;



/**
 * @author sid
 *
 */
public class LoggerConfig extends StringHelper{
	protected LoggerConfig() {
	}
	protected DalvikHook _config;
	public static String _TAG = "Hooks";
	public static String _TAG_ERROR = "Hooks-Error";
	public static String _TAG_LOG = "Hooks-Log";
	
	public static String get_TAG() {
		return _TAG;
	}
	public static String get_TAG_ERROR() {
		return _TAG_ERROR;
	}
	public static String get_TAG_LOG() {
		return _TAG_LOG;
	}
	
	protected String _out = "";
	protected String _notes = "";
	protected String _traces = "";
	
	protected boolean _enableDB = true;
	
	// this can be enabled via the _config file
	protected boolean _stackTraces = true;
	
	// change this value to get full traces
	protected boolean _fullTraces = true;
	
}
