/*******************************************************************************
 * Advanced Dalvik Dynamic Instrumentation Android Library
 * 
 * (c) 2014, 
 ******************************************************************************/
package org.sid.addi.core;

import org.sid.addi.Logs.Logger;


public class LogWrapper extends StringHelper{
private static Logger _l = new Logger();
	
	protected static void _logInit(DalvikHook config) {
		_l.logInit(config);
	}	
	
	protected static void _logLine(String line) {
		_l.logLine(line);
	}
	
	protected static void _logFlush_I(String notes,DEXHook dexh) {
		_l.logFlush_I(notes,dexh);
	}
	protected static void _logFlush_W(String notes,DEXHook dexh) {
		_l.logFlush_W(notes,dexh);
	}
	
	protected static void _logFlush_I(DEXHook dexh) {
		_l.logFlush_I(dexh);
	}
	
	protected static void _logFlush_W(DEXHook dexh) {
		_l.logFlush_W(dexh);
	}
	
	protected static void _logParameter(String name, String value) {
		_l.logParameter(name, value);
	}
	
	protected static void _logParameter(String name, Object value) {
		_l.logParameter(name, "" + value);
	}
	
	protected static void _logReturnValue(String name, String value) {
		_l.logReturnValue(name, value);
	}
	
	protected static void _logReturnValue(String name, Object value) {
		_l.logReturnValue(name, "" + value);
	}
	
	protected static void _logBasicInfo() {
		_l.logBasicInfo();
	}
	
	protected static String _getFullTraces() {
		return _l.getFullTraces();
	}

	protected static String _getLightTraces() {
		return _l.getLightTraces();		
	}
	
	public static void disableDBlogger() {
		_l.disableDBlogger();
	}
	
	public static void enableDBlogger() {
		_l.enableDBlogger();
	}
	
	public static void enableTraces() {
		_l.enableTraces();
	}
	
	public static void disableTraces() {
		_l.disableTraces();
	}
}
