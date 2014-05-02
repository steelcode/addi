package org.tesi.Hooks;

//import org.tesi.fakecontext.DalvikHookImpl;

import org.tesi.core.DalvikHookImpl;


public class HookList {
	
	static private DalvikHookImpl[] _hookList = new DalvikHookImpl[] {
		
		// nello skip > 1 return 0, skip == 1 return 1
		
		new DalvikHookImpl("Landroid/provider/Settings$Secure;", "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;","", "", 2, 2,0),
		
		/** SSL */
		new DalvikHookImpl("Ljava/security/Signature;","verify","([B)Z","","",2, 1, 1),
		new DalvikHookImpl("Ljava/security/Signature;","verify","([BII)Z","","",4,3,1),
		new DalvikHookImpl("Lorg/apache/harmony/xnet/provider/jsse/OpenSSLSocketImpl;", "verifyCertificateChain", "([[BLjava/lang/String;)V", "","", 3, 2 ,1),
		new DalvikHookImpl("Lorg/apache/http/client/methods/HttpGet;", "<init>", "(Ljava/lang/String;)V", "check_uri", "org/tesi/Hooks/SSLHook", 2,1,0),
		new DalvikHookImpl("Lorg/apache/http/client/methods/HttpPost;", "<init>", "(Ljava/lang/String;)V", "check_uri", "org/tesi/Hooks/SSLHook", 2,1,0),
		new DalvikHookImpl("Ljavax/net/ssl/SSLContextSpi;", "engineInit", "([Ljavax/net/ssl/KeyManager;[Ljavax/net/ssl/TrustManager;Ljava/security/SecureRandom;)V", "check_trust_manager", "org/tesi/Hooks/SSLHook", 4,3,0),
		new DalvikHookImpl("Ljavax/net/ssl/HttpsURLConnection;", "setSSLSocketFactory", "(Ljavax/net/ssl/SSLSocketFactory;)V", "", "", 2,1,0),
		new DalvikHookImpl("Lorg/apache/http/conn/ssl/SSLSocketFactory;", "setHostnameVerifier", "(Lorg/apache/http/conn/ssl/X509HostnameVerifier;)V", "check_hostname_verifier", "org/tesi/Hooks/SSLHook", 2,1,0),
		//new DalvikHookImpl("Ljavax/net/ssl/HttpsURLConnection;", "setDefaultHostnameVerifier", "(Ljavax/net/ssl/HostnameVerifier;)V", "check_hostname_verifier", "org/tesi/Hooks/SSLHook", 1,1,0),
	
		
		/** Emulator vs RealPhone */
		new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getDeviceId","()Ljava/lang/String;","","",1,  0,0),
		new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getLine1Number","()Ljava/lang/String;","","",1,0,0),
		new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getSubscriberId","()Ljava/lang/String;","","",1, 0,0),
		new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getSimSerialNumber","()Ljava/lang/String;","","",1, 0,0),
		
		/** beat reflection */
		new DalvikHookImpl("Ljava/lang/Class;", "getMethod", "(Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;", "","",3, 2, 0),
		
		/** FILE SYSTEM HOOK */
		
		new DalvikHookImpl("Ljava/io/File;", "<init>", "(Ljava/lang/String;)V", "","", 2, 1,0),
		new DalvikHookImpl("Ljava/io/File;", "<init>", "(Ljava/net/URI;)V", "","", 2, 1,0),
		new DalvikHookImpl("Ljava/io/File;", "<init>", "(Ljava/io/File;Ljava/lang/String;)V", "", "",3,2,0),
		new DalvikHookImpl("Ljava/io/File;", "<init>", "(Ljava/lang/String;Ljava/lang/String;)V", "", "",3,2,0),
		new DalvikHookImpl("Ljava/io/File;", "setWritable", "(ZZ)Z", "check_fs_perm","org/tesi/Hooks/FileSystemHook", 3, 2,0),
		new DalvikHookImpl("Ljava/io/File;", "setWritable", "(Z)Z", "","", 2, 1,0),
		new DalvikHookImpl("Ljava/io/File;", "setReadable", "(ZZ)Z", "check_fs_perm","org/tesi/Hooks/FileSystemHook", 3,2,0),
		new DalvikHookImpl("Ljava/io/File;", "setReadable", "(Z)Z", "", "",2,1,0),
		new DalvikHookImpl("Ljava/io/File;", "setExecutable", "(ZZ)Z", "check_fs_perm","org/tesi/Hooks/FileSystemHook", 3, 2,0),
		new DalvikHookImpl("Ljava/io/File;", "setExecutable", "(Z)Z", "", "",2, 1,0),
		new DalvikHookImpl("Ljava/io/FileOutputStream;", "<init>", "(Ljava/io/File;)V", "check_dir","org/tesi/Hooks/FileSystemHook", 2,1,0),
		new DalvikHookImpl("Landroid/content/ContextWrapper;", "openFileOutput", "(Ljava/lang/String;I)Ljava/io/FileOutputStream;", "check_fs_mode","org/tesi/Hooks/FileSystemHook", 3, 2,0),
		
		/** SQLite */
		
		new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "execSQL", "(Ljava/lang/String;)V", "sql_intro","org/tesi/Hooks/SQLiteHook", 2,1 ,0),
		new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "execSQL", "(Ljava/lang/String;[Ljava/lang/Object;)V", "sql_intro","org/tesi/Hooks/SQLiteHook",3, 2,0),
		new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "update", "(Ljava/lang/String;Landroid/content/ContentValues;Ljava/lang/String;[Ljava/lang/String;)I", "my_update","org/tesi/Hooks/SQLiteHook", 5, 4, 0),
		new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "updateWithOnConflict","(Ljava/lang/String;Landroid/content/ContentValues;Ljava/lang/String;[Ljava/lang/String;I)I", "","", 6 ,5 ,0),
		new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "insert", "(Ljava/lang/String;Ljava/lang/String;Landroid/content/ContentValues;)J", "my_insert","org/tesi/Hooks/SQLiteHook",4,3,0),
		new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "insertOrThrow","(Ljava/lang/String;Ljava/lang/String;Landroid/content/ContentValues;)J","","",4,3,0),
		new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "insertWithOnConflict","(Ljava/lang/String;Ljava/lang/String;Landroid/content/ContentValues;I)J","","",5,4,0),
		new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "replace", "(Ljava/lang/String;Ljava/lang/String;Landroid/content/ContentValues;)J", "","",4, 3, 0),
		new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "replace", "(Ljava/lang/String;Ljava/lang/String;Landroid/content/ContentValues;)J", "","",4, 3,0),
		
		
		/** SHARED PREFERENCES */
		//new DalvikHookImpl("Landroid/content/ContextWrapper;", "getSharedPreferences", "(Ljava/lang/String;I)Landroid/content/SharedPreferences;", "", 3,2,0),
		new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getString", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", "getSharedPref", "org/tesi/Hooks/SharedPrefHook",3,2,0),
		new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getStringSet", "(Ljava/lang/String;Ljava/util/Set;)Ljava/util/Set;", "getSharedPref","org/tesi/Hooks/SharedPrefHook", 3,2,0),
		new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getAll", "()Ljava/util/Map;", "","org/tesi/Hooks/SharedPrefHook", 1,0,0),
		new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getBoolean", "(Ljava/lang/String;Z)Z", "", "org/tesi/Hooks/SharedPrefHook",3,2,0),
		new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getFloat", "(Ljava/lang/String;F)F", "","org/tesi/Hooks/SharedPrefHook", 3,2,0),
		new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getInt", "(Ljava/lang/String;I)I", "", "org/tesi/Hooks/SharedPrefHook",3,2,0),
		new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getLong", "(Ljava/lang/String;J)J", "","org/tesi/Hooks/SharedPrefHook", 4,3,0),
		new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "contains", "(Ljava/lang/String;)Z", "","", 2,1,0),
		new DalvikHookImpl("Landroid/content/SharedPreferences$Editor;", "putString", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/SharedPreferences$Editor;", "","", 2,2,0),
		new DalvikHookImpl("Landroid/content/SharedPreferences$Editor;", "putBoolean", "(Ljava/lang/String;Z)Landroid/content/SharedPreferences$Editor;", "", "",2,2,0),
		new DalvikHookImpl("Landroid/content/SharedPreferences$Editor;", "putInt", "(Ljava/lang/String;I)Landroid/content/SharedPreferences$Editor;", "","", 2,2,0),
		new DalvikHookImpl("Landroid/content/SharedPreferences$Editor;", "commit", "()Z", "", "",1,0,0),
		
	
		/** IPC */
		new DalvikHookImpl("Landroid/content/ContextWrapper;", "startService", "(Landroid/content/Intent;)Landroid/content/ComponentName;", "","", 2,1,0),
		new DalvikHookImpl("Landroid/content/ContextWrapper;", "startActivities", "([Landroid/content/Intent;)V", "", "",2,1,0),
		new DalvikHookImpl("Landroid/app/Activity;", "startActivity", "(Landroid/content/Intent;)V", "dump_intent","org/tesi/Hooks/IPCHooks", 2,1,0),
		//new DalvikHookImpl("Landroid/app/Activity;", "startActivity", "(Landroid/content/Intent;Landroid/os/Bundle;)V", "/data/local/tmp/classes.dex", 3,2,0),
		new DalvikHookImpl("Landroid/content/ContextWrapper;", "sendBroadcast", "(Landroid/content/Intent;)V", "","", 2,1,0),
		new DalvikHookImpl("Landroid/content/ContextWrapper;", "sendBroadcast", "(Landroid/content/Intent;Ljava/lang/String;)V", "","", 3,2,0),
		//new DalvikHookImpl("Landroid/app/Activity;", "onCreate", "(Landroid/os/Bundle;)V", "getcon","org/tesi/fakecontext/AppContextConfig", 2,1,0),
		new DalvikHookImpl("Landroid/content/ContextWrapper;", "registerReceiver", "(Landroid/content/BroadcastReceiver;Landroid/content/IntentFilter;)Landroid/content/Intent;", "", "",3,2,0),
		new DalvikHookImpl("Landroid/content/ContextWrapper;", "registerReceiver", "(Landroid/content/BroadcastReceiver;Landroid/content/IntentFilter;Ljava/lang/String;Landroid/os/Handler;)Landroid/content/Intent;", "","", 5,4,0),
		new DalvikHookImpl("Landroid/app/Application;", "onCreate", "()V", "getcon","org/tesi/fakecontext/AppContextConfig", 1,0,0),
		
		/** CRYPTO */
		new DalvikHookImpl("Ljava/security/MessageDigest;", "digest", "([BII)I", "", "",3, 2,0),
		new DalvikHookImpl("Ljava/security/MessageDigest;", "digest", "()[B", "", "",1, 0,0),
		new DalvikHookImpl("Ljava/security/MessageDigest;", "digest", "([B)[B", "","", 2, 1,0),
		new DalvikHookImpl("Ljava/security/MessageDigest;", "update", "([B)V", "", "",2, 1,0),
		new DalvikHookImpl("Ljava/security/MessageDigest;", "update", "([BII)V", "","", 4, 3,0),
		new DalvikHookImpl("Ljava/security/MessageDigest;", "update", "(Ljava/nio/ByteBuffer;)V", "", "",2, 1,0),
		//new DalvikHookImpl("Ljavax/crypto/Cypher;", "doFinal", "()[B", "doFinal","org/tesi/Hooks/CryptoHook", 1, 0,0),
		//new DalvikHookImpl("Ljavax/crypto/Cypher;", "doFinal", "({BII)[B", "doFinal","org/tesi/Hooks/CryptoHook", 1, 0,0),
		//new DalvikHookImpl("Lorg/apache/http/conn/ssl/SSLSocketFactory;", "digest", "([BII)I", "", 3, 2,0)
		
		
		
		/** WebView */
		new DalvikHookImpl("Landroid/webkit/WebSettings;","setJavaScriptEnabled", "(Z)V","webview_set","org/tesi/Hooks/WebviewHook",2,1,0),
		new DalvikHookImpl("Landroid/webkit/WebSettings;","setAllowFileAccess", "(Z)V","webview_set","org/tesi/Hooks/WebviewHook",2,1,0),
		new DalvikHookImpl("Landroid/webkit/WebView;","addJavascriptInterface", "(Ljava/lang/Object;Ljava/lang/String;)V","webview_js","org/tesi/Hooks/WebviewHook",3,2,0),
		
		/** Disable Signature and check 
		new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkCallingOrSelfPermission", "(Ljava/lang/String;)I", "", "",2,1,1),
		new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkCallingOrSelfUriPermission", "(Landroid/net/Uri;I)I", "", "",3,2,1),
		new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkPermission", "(Ljava/lang/String;II)I", "", "",4,3,1),
		new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkCallingPermission", "(Ljava/lang/String;)I", "", "",2,1,1),
		new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkCallingUriPermission", "(Landroid/net/Uri;I)I", "", "",3,2,1),
		new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkUriPermission", "(Landroid/net/Uri;III)I", "", "",5,4,1),
		new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkUriPermission", "(Landroid/net/Uri;Ljava/lang/String;Ljava/lang/String;III)I", "", "",7,6,1),
		new DalvikHookImpl("Landroid/content/pm/PackageManager;", "checkPermission", "(Ljava/lang/String;Ljava/lang/String;)I", "", "",2,2,1),
		new DalvikHookImpl("Landroid/app/ContextImpl;", "checkPermission", "(Ljava/lang/String;II)I", "", "",4,3,1),
		new DalvikHookImpl("Landroid/app/ContextImpl;", "checkCallingPermission", "(Ljava/lang/String;)I", "", "",2,1,1),
		new DalvikHookImpl("Landroid/app/ActivityManagerNative;", "checkPermission", "(Ljava/lang/String;II)I", "", "",3,3,1),
		*/
		new DalvikHookImpl("Landroid/app/ApplicationPackageManager;", "checkPermission", "(Ljava/lang/String;Ljava/lang/String;)I", "", "",3,2,2),
		//new DalvikHookImpl("android/content/pm/IPackageManager$Stub$Proxy;", "checkPermission", "(Ljava/lang/String;Ljava/lang/String;)I", "", "",3,2,1),
		new DalvikHookImpl("Landroid/os/Debug;", "isDebuggerConnected", "()Z", "", "",0,0,2),
		/** NETWORK */
		new DalvikHookImpl("Ljava/net/InetAddress;", "getByName", "(Ljava/lang/String;)Ljava/net/InetAddress;", "getbyname", "org/tesi/Hooks/NetworkHook", 1, 1, 0)
				
	
	};
	static public DalvikHookImpl[] getHookList() {
		return _hookList;
	}

}
