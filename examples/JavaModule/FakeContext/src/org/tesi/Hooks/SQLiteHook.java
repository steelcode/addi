package org.tesi.Hooks;

//import org.tesi.fakecontext.DalvikHook;
import org.tesi.core.DalvikHookImpl;

import android.util.Log;

public class SQLiteHook extends DalvikHookImpl {
	
	public static void sql_intro(Object...args){
		Log.i(_TAG,"-> [" + args[0] + "]");
	}
	public static void my_update(Object...args){
		Log.i(_TAG,"Table " + args[0]);
		Log.i(_TAG,"Content Values " + args[1]);
		Log.i(_TAG,"Where " + args[2]);
		Log.i(_TAG,"Where Args " + args[3]);
		
		Log.i(_TAG,"-> " + "Table: " + args[0] + ", " +
				"ContentValues: " + args[1] + ", " +
				"Where: " + args[2] + ", " +
				"WhereArgs: " + args[3]);
		// TODO: dump 'where' args (array of strings for args[3])
	}
	public static void my_insert(Object... args){
		Log.i(_TAG,"Table "+  args[0]);
		Log.i(_TAG,"Content Values " + args[1]);	
		Log.i(_TAG,"-> " + "Table: " + args[0] + ", " +
						"ContentValues: " + args[2]);
	}

}
