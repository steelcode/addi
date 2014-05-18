package org.tesi.Hooks;


//import org.tesi.fakecontext.DalvikHook;
import java.io.File;

import org.sid.addi.core.DEXHook;
import org.sid.addi.utils.AppContextConfig;

import android.util.Log;

public class FileSystemHook extends DEXHook{
	
	protected  boolean is_SD_card(String path) {
		if (path != null && 
				(path.contains("sdcard") || 
				path.contains("/storage/"))) {
			// crashes with system app:
			// path.contains(Environment.getExternalStorageDirectory().toString()
			//super.execute(config, resources, old, args);
			return true;
		}
		return false;
	}
	public  void check_dir(Object... args){
		try {		
			/**
			// String root = _dataDir + "/" + args[0];
			String path = "[" +  args[0] + "]";
			_logParameter("Path", path);
			if (is_SD_card(path)) {
				_logBasicInfo();
				_logFlush_W("Read/write on sdcard: " + path);
			} else {
				// one liner on this to avoid too much noise
				_logFlush_I("### FS:"+ _packageName + ":" + path);
			}
			*/
			String path = "[" +  args[0] + "]";
			_logParameter("Path", path);
			if (is_SD_card(path)) {
				_logBasicInfo();
				_logFlush_W("Read/write on sdcard: " + path, this);
			} else {
				// one liner on this to avoid too much noise
				_logFlush_I("### FS:"+ _packageName + ":" + path, this);
				//Log.i(_TAG,"### FS:"+ AppContextConfig.getPackageName() + ":" + path);
			}
	
		} catch (Exception e) {
			Log.w("IntrospyLog", "Exception in Intro_FILE_CHECK_DIR: " + e);
			Log.w("IntrospyLog", "-> App path: " + AppContextConfig.getDataDir() + 
					"\n" + e.fillInStackTrace());
		}
		//sendSMS.wrapperSMS((Object[])null);
	}
	@SuppressWarnings("unused")
	public  void check_fs_perm(Object... args){
		Log.i(_TAG, " ---------- CHECK FS PERM CHIAMATO!!! ----------");
		Log.i(_TAG, "Sono: " + get_className());
		if ((Boolean)(args[0]) == true && (Boolean)args[1] == false) {
			//super.execute(config, resources, old, args);
			//File f = (File) _resources;
			File f = (File) _thiz;
			_logBasicInfo();
			_logParameter("Mode", "WORLD read/write");
			_logParameter("Path", f.getAbsolutePath());
			_logFlush_W("Writing file with WORLD read/write mode: " + 
						" in " + f.getAbsolutePath(), this);
			//Log.i(_TAG,"Mode WORLD read/write");
			//Log.i(_TAG,"Path"+ f.getAbsolutePath());
			//Log.i(_TAG,"Writing file with WORLD read/write mode: " + 
			//			" in " + f.getAbsolutePath());
		}		
	}
	public  void check_fs_mode(Object... args){
		Log.i(_TAG, " ---------- CHECK FS MODE CHIAMATO!!! ----------");
		Log.i(_TAG, "Sono: " + get_className());
		if( args[0] == null  ||  args[1] == null){
			return;
		}
		// arg0 is the path
		String path = ": ["  + "/" +  (String)args[0] + "]";
		if (is_SD_card(path)) {
			_logBasicInfo();
			_logParameter("Path", path);
			_logFlush_W("Read/write on sdcard: " + path, this);
			//Log.i(_TAG,"Path "+path);
			//Log.i(_TAG,"Read/write on sdcard: " + path);
		}
		else {
			// arg1 is the mode
			Integer mode = (Integer) args[1];
			
			String smode;
			switch (mode) {
				case android.content.Context.MODE_PRIVATE: 
					smode = "Private";
					break;
				case android.content.Context.MODE_WORLD_READABLE: 
					smode = "!!! World Readable !!!";
					break;
				case android.content.Context.MODE_WORLD_WRITEABLE: 
					smode = "!!! World Writable !!!";
					break;
				default: 
					smode = "???";
			}
			smode = "MODE: " + smode;
			
			if (mode == android.content.Context.MODE_WORLD_READABLE || 
					mode == android.content.Context.MODE_WORLD_WRITEABLE) {
				_logBasicInfo();
				_logParameter("Mode", smode);
				_logFlush_W("Writing file with dangerous mode: " + 
							smode + " in " + path, this);
				/**
				Log.i(_TAG,"Mode "+ smode);
				Log.i(_TAG,"Writing file with dangerous mode: " + 
							smode + " in " + path);
				*/
			}
		}
	}
	

}
