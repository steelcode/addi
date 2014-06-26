package org.tesi.Hooks;

//import org.tesi.fakecontext.DalvikHook;
import org.sid.addi.core.DEXHook;

import android.util.Log;

public class SQLiteHook extends DEXHook {
	
	public  void sql_intro(Object...args){
		Log.i(_TAG,"-> [" + args[0] + "]");
		//_logBasicInfo(this);
		//_logFlush_I("-> [" + args[0] + "]", this);
	}
	public  void my_update(Object...args){
		Log.i(_TAG,"Table " + args[0]);
		Log.i(_TAG,"Content Values " + args[1]);
		Log.i(_TAG,"Where " + args[2]);
		Log.i(_TAG,"Where Args " + args[3]);
		
		Log.i(_TAG,"-> " + "Table: " + args[0] + ", " +
				"ContentValues: " + args[1] + ", " +
				"Where: " + args[2] + ", " +
				"WhereArgs: " + args[3]);
		/**
		_logBasicInfo(this);
		
		_logParameter("Table", args[0]);
		_logParameter("Content Values", args[1]);
		_logParameter("Where", args[2]);
		_logParameter("Where Args", args[3]);
		
		_logFlush_I("-> " + "Table: " + args[0] + ", " +
				"ContentValues: " + args[1] + ", " +
				"Where: " + args[2] + ", " +
				"WhereArgs: " + args[3], this);
				*/
		// TODO: dump 'where' args (array of strings for args[3])
	}
	public  void my_insert(Object... args){
		Log.i(_TAG,"Table "+  args[0]);
		Log.i(_TAG,"Content Values " + args[1]);	
		Log.i(_TAG,"-> " + "Table: " + args[0] + ", " +
						"ContentValues: " + args[2]);
		/**
		_logBasicInfo(this);
		
		_logParameter("Table", args[0]);
		_logParameter("Content Values", args[1]);
		
		_logFlush_I("-> " + "Table: " + args[0] + ", " +
						"ContentValues: " + args[2], this);
						*/
	}

}
