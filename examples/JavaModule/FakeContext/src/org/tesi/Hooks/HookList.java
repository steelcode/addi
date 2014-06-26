package org.tesi.Hooks;

//import org.tesi.fakecontext.DalvikHookImpl;

import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

import org.tesi.core.DalvikHookImpl;
import org.sid.addi.utils.AppContextConfig;





public class HookList {
	
	public static List<?> _hookList = new LinkedList<DalvikHookImpl>(Arrays.asList(
			// nello skip > 1 return 0, ski	p == 1 return 1
			new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "testJD", "(JD)V", "","", null, 0),
			new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "testAll", "(ZBCSIJFDLjava/lang/String;)D", "","", null, 0),
			
			
		
			/*
			new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "trovami", "()V", "","", null, 0),
			new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "trovami1", "()V", "","", null, 0),
			new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "trovami5", "(I)V", "","", null, 0),
			new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "trovami3", "()Z", "","", null, 0),
			new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "trovami4", "(I)I", "","", null, 0),
			new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "trovami6", "(ILjava/lang/String;JD)V", "","", null, 0),
			*/
			/** FILE SYSTEM HOOK */		
			//new DalvikHookImpl("Ljava/io/File;", "<init>", "(Ljava/lang/String;)V", "","", null,2, 1,0),
			//new DalvikHookImpl("Ljava/io/File;", "<init>", "(Ljava/net/URI;)V", "","",null, 2, 1,0),
			//new DalvikHookImpl("Ljava/io/File;", "<init>", "(Ljava/io/File;Ljava/lang/String;)V", "", "",null,3,2,0),
			//new DalvikHookImpl("Ljava/io/File;", "<init>", "(Ljava/lang/String;Ljava/lang/String;)V", "", "",null,3,2,0),
			new DalvikHookImpl("Ljava/io/File;", "setWritable", "(ZZ)Z", "check_fs_perm","org/tesi/Hooks/FileSystemHook", new FileSystemHook(), 0),
			new DalvikHookImpl("Ljava/io/File;", "setWritable", "(Z)Z", "","",null,0),
			new DalvikHookImpl("Ljava/io/File;", "setReadable", "(ZZ)Z", "check_fs_perm","org/tesi/Hooks/FileSystemHook",new FileSystemHook(),0),
			new DalvikHookImpl("Ljava/io/File;", "setReadable", "(Z)Z", "", "",null,0),
			new DalvikHookImpl("Ljava/io/File;", "setExecutable", "(ZZ)Z", "check_fs_perm","org/tesi/Hooks/FileSystemHook",new FileSystemHook(),0),
			new DalvikHookImpl("Ljava/io/File;", "setExecutable", "(Z)Z", "", "",null,0),
			new DalvikHookImpl("Ljava/io/FileOutputStream;", "<init>", "(Ljava/io/File;)V", "check_dir","org/tesi/Hooks/FileSystemHook",new FileSystemHook(),0),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "openFileOutput", "(Ljava/lang/String;I)Ljava/io/FileOutputStream;", "check_fs_mode","org/tesi/Hooks/FileSystemHook", new FileSystemHook(),0),

			/** NETWORK */
			new DalvikHookImpl("Ljava/net/InetAddress;", "getByName", "(Ljava/lang/String;)Ljava/net/InetAddress;", "getbyname", "org/tesi/Hooks/NetworkHook", new NetworkHook(),0),
			//new DalvikHookImpl("Ljava/net/Socket;", "connect", "(Ljava/net/SocketAddress;)V", "connect", "org/tesi/Hooks/NetworkHook", new NetworkHook(),0),
			//new DalvikHookImpl("Ljava/net/ServerSocket;", "bind", "(Ljava/net/SocketAddress;)V", "bind", "org/tesi/Hooks/NetworkHook", new NetworkHook(),0),
			//new DalvikHookImpl("Ljava/net/ServerSocket;", "bind", "(Ljava/net/SocketAddress;I)V", "bind", "org/tesi/Hooks/NetworkHook", new NetworkHook(),0),
			new DalvikHookImpl("Ljava/net/InetSocketAddress;", "<init>", "()V", "initInetSocketAddress", "org/tesi/Hooks/NetworkHook", new NetworkHook(),0),
			new DalvikHookImpl("Ljava/net/InetSocketAddress;", "<init>", "(Ljava/net/InetAddress;I)V", "initInetSocketAddress", "org/tesi/Hooks/NetworkHook", new NetworkHook(),0),
			new DalvikHookImpl("Ljava/net/InetSocketAddress;", "<init>", "(Ljava/lang/String;I)V", "initInetSocketAddress", "org/tesi/Hooks/NetworkHook", new NetworkHook(),0),
			//new DalvikHookImpl("Ljava/net/InetSocketAddress;", "<init>", "(Ljava/lang/String;IZ)V", "initInetSocketAddress", "org/tesi/Hooks/NetworkHook", new NetworkHook(),0),		
			
			//new DalvikHookImpl("Ljava/net/InetAddress;", "getAllByName", "(Ljava/lang/String;)[Ljava/net/InetAddress;", "getbyname", "org/tesi/Hooks/NetworkHook", 1, 1, 0)
			new DalvikHookImpl("Landroid/provider/Settings$Secure;", "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;","", "",null, 0),
			/** SSL */
			new DalvikHookImpl("Ljava/security/Signature;","verify","([B)Z","","",null,1),
			new DalvikHookImpl("Ljava/security/Signature;","verify","([BII)Z","","",null,1),
			new DalvikHookImpl("Lorg/apache/harmony/xnet/provider/jsse/OpenSSLSocketImpl;", "verifyCertificateChain", "([[BLjava/lang/String;)V", "","", null,1),
			new DalvikHookImpl("Lorg/apache/http/client/methods/HttpGet;", "<init>", "(Ljava/lang/String;)V", "check_uri", "org/tesi/Hooks/SSLHook", new SSLHook(),0),
			new DalvikHookImpl("Lorg/apache/http/client/methods/HttpPost;", "<init>", "(Ljava/lang/String;)V", "check_uri", "org/tesi/Hooks/SSLHook",new SSLHook(), 0),
			new DalvikHookImpl("Ljavax/net/ssl/SSLContextSpi;", "engineInit", "([Ljavax/net/ssl/KeyManager;[Ljavax/net/ssl/TrustManager;Ljava/security/SecureRandom;)V", "check_trust_manager", "org/tesi/Hooks/SSLHook",new SSLHook(),0),
			new DalvikHookImpl("Ljavax/net/ssl/HttpsURLConnection;", "setSSLSocketFactory", "(Ljavax/net/ssl/SSLSocketFactory;)V","check_trust_socketfactory", "org/tesi/Hooks/SSLHook", new SSLHook(),0),
			new DalvikHookImpl("Lorg/apache/http/conn/ssl/SSLSocketFactory;", "setHostnameVerifier", "(Lorg/apache/http/conn/ssl/X509HostnameVerifier;)V", "check_hostname_verifier", "org/tesi/Hooks/SSLHook", new SSLHook(),0),
			//new DalvikHookImpl("Ljavax/net/ssl/HttpsURLConnection;", "setDefaultHostnameVerifier", "(Ljavax/net/ssl/HostnameVerifier;)V", "check_hostname_verifier", "org/tesi/Hooks/SSLHook", 1,1,0),
			
			/** Emulator vs RealPhone */
			new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getDeviceId","()Ljava/lang/String;","","",null,0),
			new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getLine1Number","()Ljava/lang/String;","","",null,0),
			new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getSubscriberId","()Ljava/lang/String;","","",null,0),
			new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getSimSerialNumber","()Ljava/lang/String;","","",null,0),

			/** beat reflection */
			new DalvikHookImpl("Ljava/lang/Class;", "getMethod", "(Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;", "","",null,0),

			/** SQLite  
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "execSQL", "(Ljava/lang/String;)V", "sql_intro","org/tesi/Hooks/SQLiteHook", new SQLiteHook(), 2,1 ,0),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "execSQL", "(Ljava/lang/String;[Ljava/lang/Object;)V", "sql_intro","org/tesi/Hooks/SQLiteHook", new SQLiteHook(),3, 2,0),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "update", "(Ljava/lang/String;Landroid/content/ContentValues;Ljava/lang/String;[Ljava/lang/String;)I", "my_update","org/tesi/Hooks/SQLiteHook",new SQLiteHook(), 5, 4, 0),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "updateWithOnConflict","(Ljava/lang/String;Landroid/content/ContentValues;Ljava/lang/String;[Ljava/lang/String;I)I", "","", null,6 ,5 ,0),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "insert", "(Ljava/lang/String;Ljava/lang/String;Landroid/content/ContentValues;)J", "my_insert","org/tesi/Hooks/SQLiteHook",new SQLiteHook(),4,3,0),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "insertOrThrow","(Ljava/lang/String;Ljava/lang/String;Landroid/content/ContentValues;)J","","",null,4,3,0),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "insertWithOnConflict","(Ljava/lang/String;Ljava/lang/String;Landroid/content/ContentValues;I)J","","",null,5,4,0),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "replace", "(Ljava/lang/String;Ljava/lang/String;Landroid/content/ContentValues;)J", "","",null,4, 3, 0),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "replace", "(Ljava/lang/String;Ljava/lang/String;Landroid/content/ContentValues;)J", "","",null,4, 3,0),
			*/
			/** SHARED PREFERENCES */
			//new DalvikHookImpl("Landroid/content/ContextWrapper;", "getSharedPreferences", "(Ljava/lang/String;I)Landroid/content/SharedPreferences;", "", 3,2,0),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getString", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", "getSharedPref", "org/tesi/Hooks/SharedPrefHook", new SharedPrefHook(),0),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getStringSet", "(Ljava/lang/String;Ljava/util/Set;)Ljava/util/Set;", "getSharedPref","org/tesi/Hooks/SharedPrefHook", new SharedPrefHook(),0),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getAll", "()Ljava/util/Map;", "","", null,0),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getBoolean", "(Ljava/lang/String;Z)Z", "", "",null,0),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getFloat", "(Ljava/lang/String;F)F", "","",null, 0),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getInt", "(Ljava/lang/String;I)I", "", "",null,0),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getLong", "(Ljava/lang/String;J)J", "","",null, 0),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "contains", "(Ljava/lang/String;)Z", "","",null, 0),
			new DalvikHookImpl("Landroid/content/SharedPreferences$Editor;", "putString", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/SharedPreferences$Editor;", "put_pref","org/tesi/Hooks/SharedPrefHook", new SharedPrefHook(),0),
			new DalvikHookImpl("Landroid/content/SharedPreferences$Editor;", "putBoolean", "(Ljava/lang/String;Z)Landroid/content/SharedPreferences$Editor;", "put_pref","org/tesi/Hooks/SharedPrefHook", new SharedPrefHook(),0),
			new DalvikHookImpl("Landroid/content/SharedPreferences$Editor;", "putInt", "(Ljava/lang/String;I)Landroid/content/SharedPreferences$Editor;", "put_pref","org/tesi/Hooks/SharedPrefHook", new SharedPrefHook(),0),
			new DalvikHookImpl("Landroid/content/SharedPreferences$Editor;", "commit", "()Z", "", "", null, 0),		

			/** IPC */
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "startService", "(Landroid/content/Intent;)Landroid/content/ComponentName;", "dump_intent","org/tesi/Hooks/IPCHooks", new IPCHooks(),0),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "startActivities", "([Landroid/content/Intent;)V", "", "", null,0),
			new DalvikHookImpl("Landroid/app/Activity;", "startActivity", "(Landroid/content/Intent;)V", "dump_intent","org/tesi/Hooks/IPCHooks", new IPCHooks(), 0),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "sendBroadcast", "(Landroid/content/Intent;)V", "dump_intent","org/tesi/Hooks/IPCHooks", new IPCHooks(), 0),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "sendBroadcast", "(Landroid/content/Intent;Ljava/lang/String;)V", "dump_intent","org/tesi/Hooks/IPCHooks", new IPCHooks(), 0),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "registerReceiver", "(Landroid/content/BroadcastReceiver;Landroid/content/IntentFilter;)Landroid/content/Intent;", "ipc_receiver", "org/tesi/Hooks/IPCHooks", new IPCHooks(),0),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "registerReceiver", "(Landroid/content/BroadcastReceiver;Landroid/content/IntentFilter;Ljava/lang/String;Landroid/os/Handler;)Landroid/content/Intent;", "ipc_receiver","org/tesi/Hooks/IPCHooks",  new IPCHooks() ,0),
			new DalvikHookImpl("Landroid/app/Application;", "onCreate", "()V", "getcon","org/tesi/utils/AppContextConfig", new AppContextConfig(), 0),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "grantUriPermission", "(Ljava/lang/String;Landroid/net/Uri;I)V", "","", null, 0),		

			/** CRYPTO */
			new DalvikHookImpl("Ljava/security/MessageDigest;", "update", "([B)V", "hook_hash", "org/tesi/Hooks/HashHook", new HashHook(),0),
			new DalvikHookImpl("Ljava/security/MessageDigest;", "update", "([BII)V", "hook_hash","org/tesi/Hooks/HashHook", new HashHook(),0),
			new DalvikHookImpl("Ljava/security/MessageDigest;", "update", "(Ljava/nio/ByteBuffer;)V", "hook_hash", "org/tesi/Hooks/HashHook", new HashHook(),0),	
			//new DalvikHookImpl("Ljava/security/MessageDigest;", "digest", "([BII)I", "", "", null,3, 2,0),
			//new DalvikHookImpl("Ljava/security/MessageDigest;", "digest", "()[B", "", "", null,1, 0,0),
			//new DalvikHookImpl("Ljava/security/MessageDigest;", "digest", "([B)[B", "","",  null,2, 1,0),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "doFinal", "()[B", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(), 0),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "doFinal", "(Ljava/nio/ByteBuffer;Ljava/nio/ByteBuffer;)I", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(),0),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "doFinal", "([B)[B", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(),0),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "doFinal", "([BII[B)I", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(),0),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "doFinal", "([BII[BI)I", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(),0),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "doFinal", "([BII)[B", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(),0),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "doFinal", "([BI)I", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(),0),
			new DalvikHookImpl("Lorg/apache/http/conn/ssl/SSLSocketFactory;", "<init>", "(Ljava/security/KeyStore;Ljava/lang/String;)V", "keystore","org/tesi/Hooks/HookCryptoKey", new HookCryptoKey(),0),
			new DalvikHookImpl("Lorg/apache/http/conn/ssl/SSLSocketFactory;", "<init>", "(Ljava/security/KeyStore;Ljava/lang/String;Ljava/security/KeyStore;)V", "keystore","org/tesi/Hooks/HookCryptoKey", new HookCryptoKey(),0),
			new DalvikHookImpl("Lorg/apache/http/conn/ssl/SSLSocketFactory;", "<init>", "(Ljava/lang/String;Ljava/security/KeyStore;Ljava/lang/String;Ljava/security/KeyStore;Ljava/security/SecureRandom;Lorg/apache/http/conn/scheme/HostNameResolver;)V", "keystore_hostname","org/tesi/Hooks/HookCryptoKey", new HookCryptoKey(),0),
			//new DalvikHookImpl("Ljavax/crypto/spec/PBEKeySpec;", ",<init>", "([B)[B", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(),2, 1,0),
			//new DalvikHookImpl("Lorg/apache/http/conn/ssl/SSLSocketFactory;", "digest", "([BII)I", "", 3, 2,0),	
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "init", "(ILjava/security/Key;)V", "crypto_init","org/tesi/Hooks/CryptoHook", new CryptoHook(), 0),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "init", "(ILjava/security/Key;Ljava/security/SecureRandom;)V", "crypto_init","org/tesi/Hooks/CryptoHook", new CryptoHook(),0),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "init", "(ILjava/security/Key;Ljava/security/spec/AlgorithmParameterSpec;Ljava/security/SecureRandom;)V", "crypto_init","org/tesi/Hooks/CryptoHook", new CryptoHook(),0),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "init", "(ILjava/security/Key;Ljava/security/AlgorithmParameters;Ljava/security/SecureRandom;)V", "crypto_init","org/tesi/Hooks/CryptoHook", new CryptoHook(), 0),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "init", "(ILjava/security/Key;Ljava/security/AlgorithmParameters;)V", "crypto_init","org/tesi/Hooks/CryptoHook", new CryptoHook(),0),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "init", "(ILjava/security/Key;Ljava/security/spec/AlgorithmParameterSpec;)V", "crypto_init","org/tesi/Hooks/CryptoHook", new CryptoHook(),0),
			
			/** WebView */
			new DalvikHookImpl("Landroid/webkit/WebSettings;","setJavaScriptEnabled", "(Z)V","webview_set","org/tesi/Hooks/WebviewHook", new WebviewHook(), 0),
			new DalvikHookImpl("Landroid/webkit/WebSettings;","setAllowFileAccess", "(Z)V","webview_set","org/tesi/Hooks/WebviewHook", new WebviewHook(),0),
			new DalvikHookImpl("Landroid/webkit/WebView;","addJavascriptInterface", "(Ljava/lang/Object;Ljava/lang/String;)V","webview_js","org/tesi/Hooks/WebviewHook", new WebviewHook(),0),

			/** Disable Signature and check */
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkCallingOrSelfPermission", "(Ljava/lang/String;)I", "", "",null,1),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkCallingOrSelfUriPermission", "(Landroid/net/Uri;I)I", "", "",null,1),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkPermission", "(Ljava/lang/String;II)I", "", "",null,1),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkCallingPermission", "(Ljava/lang/String;)I", "", "",null,1),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkCallingUriPermission", "(Landroid/net/Uri;I)I", "", "",null,1),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkUriPermission", "(Landroid/net/Uri;III)I", "", "",null,1),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkUriPermission", "(Landroid/net/Uri;Ljava/lang/String;Ljava/lang/String;III)I", "", "",null,1),
			new DalvikHookImpl("Landroid/content/pm/PackageManager;", "checkPermission", "(Ljava/lang/String;Ljava/lang/String;)I", "", "",null,1),
			new DalvikHookImpl("Landroid/app/ContextImpl;", "checkPermission", "(Ljava/lang/String;II)I", "", "",null, 1),
			new DalvikHookImpl("Landroid/app/ContextImpl;", "checkCallingPermission", "(Ljava/lang/String;)I", "", "",null, 1),
			new DalvikHookImpl("Landroid/app/ActivityManagerNative;", "checkPermission", "(Ljava/lang/String;II)I", "", "",null, 1),
			new DalvikHookImpl("Landroid/app/ApplicationPackageManager;", "checkPermission", "(Ljava/lang/String;Ljava/lang/String;)I", "", "",null, 2)
			//new DalvikHookImpl("android/content/pm/IPackageManager$Stub$Proxy;", "checkPermission", "(Ljava/lang/String;Ljava/lang/String;)I", "", "",3,2,1),
			//new DalvikHookImpl("Lcom/whatsapp/contact/ContactProvider;",  "onCreate", "()Z", "","", null, 1,0,0),
		
		)); 
	
	
	static public List<?> getHookList() {
		return _hookList;

	}

}
