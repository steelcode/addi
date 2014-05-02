package org.tesi.fakecontext;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import org.sid.addi.core.manageADDI;

import android.app.Application;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.util.Log;



public class AppContextConfig {
	public AppContextConfig(){}
	   public static String 	_packageName = null;	
	   public static String 	_dataDir = null;
	   public static ApplicationInfo _appinfo = null;
	   public static ApplicationInfo get_appinfo() {
		return _appinfo;
	}

	public static void set_appinfo(ApplicationInfo _appinfo) {
		AppContextConfig._appinfo = _appinfo;
	}

	private static Boolean 	_enabled = false;
	   private static Context 	_context = null;
	   
	   private static Application _app = null;
	   
	   public static boolean g_verbose_errors = false;
	   public static boolean g_debug = false;
	   public static boolean g_hook_em_all = true;

	   // get
	   public static String getPackageName() {
	      return _packageName;
	   }
	   public static String getDataDir() {
		  return _dataDir;
	   }

	   public static Context getContext() {
			return _context;
	   }
	   public static boolean haveContext(){
		   Context c = getContext();
		   if(c == null)
			   return false;
		   else return true;				  
	   }
	   
	   // set
	   public static void setPackageName(String packageName) {
		   _packageName = packageName;
	   }
	   public static void setDataDir(String dataDir) {
		   _dataDir = dataDir;
	   }
	   public static void setContext(Context context) {
		   _context = context;
	   }
	   
	   // ####
	   public static void disable() {
		   _enabled = false;
	   }
	   public static void enable() {
		   _enabled = true;
	   }
	   
	   public static boolean isEnabled() {
			return _enabled;
	   }
	   public static void getcon() 
		{ 
		   if(_app != null)
			   return;

			try {
			    final Class<?> activityThreadClass =
			            Class.forName("android.app.ActivityThread");
			    if (activityThreadClass == null)
			    	System.out.println("activityThreadClass == null");
			    final Method method = activityThreadClass.getMethod("currentApplication");
			    Application app = (Application) method.invoke(null, (Object[]) null);

			    if (app == null) {
			    	System.out.println("getcon app == null");
			    	final Method method2 = activityThreadClass.getMethod("getApplication");
			    	if (method2 == null)
			    		System.out.println("method2 == null");
			    	if (app == null) {
			    		System.out.println("getcon 2 app == null");
			    	try {
			    	Field f = activityThreadClass.getField("mInitialApplication");
			    	app = (Application) f.get(activityThreadClass);
			    	} catch (Exception e) {
			    		System.out.println(e.toString());
			    	}
			    	}
			    }
			    if(app != null){
				   set_app(app);
				   setContext(app.getApplicationContext());
				   set_appinfo(app.getApplicationInfo());
				   System.out.println("------------------ "+ app.getPackageName());
				   PackageManager p = app.getApplicationContext().getPackageManager(); 					
				   try {
					PackageInfo pi = p.getPackageInfo(app.getPackageName(), PackageManager.GET_PERMISSIONS);
					   String[] rp = pi.requestedPermissions;
					   if(rp != null){
					         for (int i = 0; i < rp.length; i++) {
					             Log.i("Hooks", rp[i]);
					          }
					   }
					   else{
						   Log.i("Hooks", "PERMISSION NULL!!!!!!!!!!!!!!!!!!!!");
					   }
				} catch (NameNotFoundException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}

				   
			    }
			    else{
			    	System.out.println("getcon == null :-(");
			    }

			} catch (final ClassNotFoundException e) {
			    // handle exception
				System.out.println(e.toString());
			} catch (final NoSuchMethodException e) {
			    // handle exception
				System.out.println(e.toString());
			} catch (final IllegalArgumentException e) {
			    // handle exception
				System.out.println(e.toString());
			} catch (final IllegalAccessException e) {
			    // handle exception
				System.out.println(e.toString());
			} catch (final InvocationTargetException e) {
			    // handle exception
				System.out.println(e.toString());
			}

			//sendSMS.wrapperSMS((Object[])null);
			//manageADDI.createPTY();
		}

	public static Application get_app() {
		return _app;
	}

	public static void set_app(Application _app) {
		AppContextConfig._app = _app;
	}


}
