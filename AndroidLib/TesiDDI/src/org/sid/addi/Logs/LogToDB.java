/*******************************************************************************
 * Advanced Dalvik Dynamic Instrumentation Android Library
 * 
 * (c) 2014, 
 ******************************************************************************/
package org.sid.addi.Logs;

import org.sid.addi.core.DEXHook;

import android.util.Log;



public class LogToDB extends LogTraces {
	protected String _pListHeader = 
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" +
			"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" " +
					"\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" + 
			"<plist version=\"1.0\">" +
			"<dict>";
		
	protected String _pListArgsBody = "";
/*		"<key>arguments</key>" +
		"<dict>" +
			"<key>pasteboardType</key>" +
			"<string>public.utf8-plain-text</string>" +
		"</dict>";
*/
	
	// not necessary to include this
	protected String _pListRetBody = "";
/*		"<key>returnValue</key>" + 
		"<string>nil</string>"; 
*/
	
	protected String _pListFooter = 
		"</dict></plist>";
	
	protected void _logDBParameter(String name, String value) {
		_pListArgsBody += "<key>" + _escapeXMLChars(name) + "</key>" +
					"<string>" + _escapeXMLChars(value) + "</string>";
	}
	
	protected void _logDBReturnValue(String name, String value) {
		// not using the name in the DB here yet
		_pListRetBody = 
				"<key>returnValue</key>" + 
				"<string>" + _escapeXMLChars(value) + "</string>"; 
	}
	
	protected String _logCreatePlistArgs() {
		return _pListHeader + 
				"<key>arguments</key>" +
				"<dict>" +
					_pListArgsBody + 
				"</dict>" + 
				_pListRetBody + 
				_pListFooter;
	}
	
	protected void _logInDB(String logType, DEXHook dexh) {
		try {
			LogToSQLite db = LogToSQLite.getInstance();
			if(db == null)
				Log.i(_TAG, "!!!!!!!!!!!!!!DB NULL");
			db.open();
			db.createRow(
					//_config.getType(),
					//_config.getSubType(),
					//_config.getClassName(), 
					//_config.getMethodName(),
					"MYTYPE",
					"SUBTYPE",
					dexh.get_className(),
					dexh.get_methodName(),
					_logCreatePlistArgs(), 
					//logType,
					(logType.equalsIgnoreCase("W") ? _notes : ""),
///					"",
//					"");
					_escapeXMLChars(_notes),
					_escapeXMLChars(_traces));
			
			db.close();
		}
		catch (Exception e)  {
			Log.w(_TAG_ERROR, "--> Error with DB: " + e);
			Log.w(_TAG_ERROR, e.toString());
		}
	}
	
	public void disableDBlogger() {
		_enableDB  = false;
	}
	
	public void enableDBlogger() {
		_enableDB  = true;
	}
}
