/*******************************************************************************
 * Advanced Dalvik Dynamic Instrumentation Android Library
 * 
 * (c) 2014, 
 ******************************************************************************/
package org.sid.addi.Logs;

import java.util.ArrayList;
import java.util.List;

import org.sid.addi.utils.AppContextConfig;

import android.app.Application;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;


public class LogToSQLite {
	private static LogToSQLite _instance = null;
	// Database fields
	  private SQLiteDatabase database;
	  private LogToSQLiteHelper dbHelper;
	  private String[] allColumns = { 
			  LogToSQLiteHelper.COLUMN_ID,
			  LogToSQLiteHelper.COLUMN_TYPE,
			  LogToSQLiteHelper.COLUMN_SUBTYPE,
			  LogToSQLiteHelper.COLUMN_CLASS,
			  LogToSQLiteHelper.COLUMN_METHOD,
			  LogToSQLiteHelper.COLUMN_DETAILS,
			  LogToSQLiteHelper.COLUMN_LOG_TYPE,
			  LogToSQLiteHelper.COLUMN_NOTES,
			  LogToSQLiteHelper.COLUMN_ST
			  };
	
	  public LogToSQLite(Context context) {
	    dbHelper = new LogToSQLiteHelper(context);
	  }
	
	  public void open() throws SQLException {
	    database = dbHelper.getWritableDatabase();
	  }
	
	  public void close() {
	    dbHelper.close();
	  }
	
	  public LogToSQLiteClass createRow(String type, String subType,
			  String className, String methodName, 
			  String details, String logType) {
		  return createRow(type, subType, className, 
				  methodName, details, logType, "", "");
	  }
	  
	  public LogToSQLiteClass createRow(String type, String subType,
			  String className, String methodName, 
			  String details, String logType, String notes) {
		  return createRow(type, subType, className, 
				  methodName, details, logType, notes, "");
	  }
	  
	  public LogToSQLiteClass createRow(String type, String subType,
			  String className, String methodName, String details,
			  String logType, String notes, String st) {
	    ContentValues values = new ContentValues();
	    values.put(LogToSQLiteHelper.COLUMN_TYPE, type);
	    values.put(LogToSQLiteHelper.COLUMN_SUBTYPE, subType); 
	    values.put(LogToSQLiteHelper.COLUMN_CLASS, className);
	    values.put(LogToSQLiteHelper.COLUMN_METHOD, methodName);
	    values.put(LogToSQLiteHelper.COLUMN_DETAILS, details);
	    values.put(LogToSQLiteHelper.COLUMN_LOG_TYPE, logType);
	    values.put(LogToSQLiteHelper.COLUMN_NOTES, notes);
	    values.put(LogToSQLiteHelper.COLUMN_ST, st);
	    
	    long insertId = database.insert(
	    		LogToSQLiteHelper.TABLE_TRACES, 
	    		null, values);
	    
	    Cursor cursor = database.query(
	    		LogToSQLiteHelper.TABLE_TRACES,
	    		allColumns, LogToSQLiteHelper.COLUMN_ID + 
	    		" = " + insertId, null,
	    		null, null, null);
	    
	    cursor.moveToFirst();
	    LogToSQLiteClass 
	    	newRow = cursorToRow(cursor);
	    cursor.close();
	    return newRow;
	  }
	
	  private LogToSQLiteClass cursorToRow(Cursor cursor) {
		  LogToSQLiteClass Row = 
				  new LogToSQLiteClass();
	    Row.setId(cursor.getLong(0));
	    Row.setRow(cursor.getString(1));
	    return Row;
	}
	
	public void deleteRow(LogToSQLiteClass Row) {
	    long id = Row.getId();
	    System.out.println("Row deleted with id: " + id);
	    database.delete(LogToSQLiteHelper.TABLE_TRACES, 
	    		LogToSQLiteHelper.COLUMN_ID
	    		+ " = " + id, null);
	  }
	
	  public List<LogToSQLiteClass> getAllRows() {
		    List<LogToSQLiteClass> Rows = 
		    		new ArrayList<LogToSQLiteClass>();
	
		    Cursor cursor = database.query(
		    	LogToSQLiteHelper.TABLE_TRACES,
		        allColumns, null, null, null, null, null);
	
		    cursor.moveToFirst();
		    while (!cursor.isAfterLast()) {
		    	LogToSQLiteClass Row = cursorToRow(cursor);
		      Rows.add(Row);
		      cursor.moveToNext();
		    }
		    // make sure to close the cursor
		    cursor.close();
		    return Rows;
		  }
	
	public static LogToSQLite getInstance() {
		Context c = AppContextConfig.get_app();
		if(c  == null)
			c = AppContextConfig.getcon();
		if (_instance  == null) {
			_instance = new LogToSQLite(c); 
					//LogToSQLite(AppContextConfig.getContext());
					//LogToSQLite(null);
		}
		return _instance;
	}
}
