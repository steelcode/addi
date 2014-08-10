package org.tesi.Hooks;

//import org.tesi.fakecontext.DalvikHookImpl;

import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

import org.tesi.core.DalvikHookImpl;
import org.sid.addi.utils.AppContextConfig;
import org.sid.addi.core.HookT.HookType;

public class HookList {	
	public static List<?> _hookList = new LinkedList<DalvikHookImpl>(Arrays.asList(
			// se skip > 0 e > 3, return skip-1
			//
			
			//new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "testJD", "(JD)V", "","", null, HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "hookMe", "()V", "cippalippa","org/tesi/Hooks/OnlyLogHook", new OnlyLogHook(), HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "testAll", "(ZBCSIJFDLjava/lang/String;)D", "","", null, HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "trovami6", "(ILjava/lang/String;)V", "entrypoint","org/tesi/Hooks/DummyHook", new DummyHook(), HookType.NORMAL_HOOK),	
			new DalvikHookImpl("Lcom/example/hellojni/HelloJni;", "testStatic", "()V", "","", null, HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcom/example/hellojni/chiaraSuca;", "testException2", "()V", "","", null, HookType.NORMAL_HOOK),
			
			new DalvikHookImpl("Lcom/android/vending/billing/IInAppBillingService$Stub;", "getBuyIntent", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Landroid/os/Bundle;", "myGetBuyIntent","org/tesi/Hooks/BillingHook", new BillingHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcom/android/vending/billing/IInAppBillingService$Stub;", "getPurchases", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Landroid/os/Bundle;", "","", null,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcom/android/vending/billing/IInAppBillingService$Stub;", "getSkuDetails", "(ILjava/lang/String;Ljava/lang/String;Landroid/os/Bundle;)Landroid/os/Bundle;", "","", null,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcom/android/vending/billing/IInAppBillingService$Stub;", "asInterface", "(Landroid/os/IBinder;)Lcom/android/vending/billing/IInAppBillingService;", "","", null,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcom/android/vending/billing/IInAppBillingService$Stub;", "isBillingSupported", "(ILjava/lang/String;Ljava/lang/String;)I", "","", null,HookType.NORMAL_HOOK),
			
			new DalvikHookImpl("Lcom/google/android/finsky/billing/iab/InAppBillingService;", "checkBillingEnabled", "(ILjava/lang/String;)Lcom/google/android/finsky/billing/iab/InAppBillingUtils$ResponseCode;", "printArgs","org/tesi/Hooks/BillingHook", new BillingHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcom/google/android/finsky/billing/iab/InAppBillingService;", "fetchSkuDetails", "(Ljava/lang/String;Landroid/os/Bundle;Ljava/lang/String;Landroid/os/Bundle;)V", "printArgs","org/tesi/Hooks/BillingHook", new BillingHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcom/google/android/finsky/billing/iab/InAppBillingService;", "makePurchaseIntent", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Landroid/app/PendingIntent;", "myMakePurchaseIntent","org/tesi/Hooks/BillingHook", new BillingHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcom/google/android/finsky/billing/iab/InAppBillingService;", "consumeIabPurchase", "(Ljava/lang/String;Ljava/lang/String;)Lcom/google/android/finsky/billing/iab/InAppBillingUtils$ResponseCode;", "printArgs","org/tesi/Hooks/BillingHook", new BillingHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcom/google/android/finsky/billing/iab/InAppBillingService;", "checkBillingApiVersionSupport", "(I)Lcom/google/android/finsky/billing/iab/InAppBillingUtils$ResponseCode;", "printArgs","org/tesi/Hooks/BillingHook", new BillingHook(),HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Lcom/google/android/finsky/billing/iab/InAppBillingService;", "populatePurchasesForPackage", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Landroid/os/Bundle;)V", "myGetPurchases","org/tesi/Hooks/BillingHook",new BillingHook(),HookType.NOX_HOOK),
			new DalvikHookImpl("Lcom/google/android/finsky/billing/iab/InAppBillingService;", "computeSignatureHash", "(Landroid/content/pm/PackageInfo;)Ljava/lang/String;", "printArgs","org/tesi/Hooks/BillingHook", new BillingHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcom/google/android/finsky/billing/iab/InAppBillingService;", "consumeIabPurchase", "(Ljava/lang/String;Ljava/lang/String;)Lcom/google/android/finsky/billing/iab/InAppBillingUtils$ResponseCode;", "printArgs","org/tesi/Hooks/BillingHook", new BillingHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcom/google/android/finsky/billing/iab/InAppBillingService;", "performIabPromoCheck", "(Ljava/lang/String;Ljava/lang/String;)Lcom/google/android/finsky/billing/iab/InAppBillingUtils$ResponseCode;", "printArgs","org/tesi/Hooks/BillingHook", new BillingHook(),HookType.NORMAL_HOOK),
	
			//new DalvikHookImpl("Lcom/android/vending/billing/IInAppBillingService;", "getBuyIntent", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Landroid/os/Bundle;", "","", null,HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Lcom/android/vending/billing/IInAppBillingService;", "getPurchases", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Landroid/os/Bundle;", "","", null,HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Lcom/android/vending/billing/IInAppBillingService;", "getSkuDetails", "(ILjava/lang/String;Ljava/lang/String;Landroid/os/Bundle;)Landroid/os/Bundle;", "","", null,HookType.NORMAL_HOOK),
			/*
			new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "trovami", "()V", "","", null, HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "trovami1", "()V", "","", null, HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "trovami5", "(I)V", "","", null, HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "trovami3", "()Z", "","", null, HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "trovami4", "(I)I", "","", null, HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lcourse/labs/activitylab/ActivityOne;", "trovami6", "(ILjava/lang/String;JD)V", "","", null, HookType.NORMAL_HOOK),
			*/
			/** String */
			//new DalvikHookImpl("Ljava/lang/StringBuilder;", "toString", "()Ljava/lang/String;", "myToString","org/tesi/Hooks/OnlyLogHook", new OnlyLogHook(), HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ljava/lang/StringBuffer;", "toString", "()Ljava/lang/String;", "myToString","org/tesi/Hooks/OnlyLogHook", new OnlyLogHook(), HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ljava/lang/String;", "compareTo", "(Ljava/lang/String;)I", "myCompare","org/tesi/Hooks/OnlyLogHook", new OnlyLogHook(), HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ljava/lang/String;", "matches", "(Ljava/lang/String;)Z", "dumpArgs","org/tesi/Hooks/OnlyLogHook", new OnlyLogHook(), HookType.NORMAL_HOOK),
			
			//new DalvikHookImpl("Ldalvik/system/PathClassLoader;", "<init>", "(Ljava/lang/String;Ljava/lang/ClassLoader;)V", "dumpArgs","org/tesi/Hooks/OnlyLogHook", new OnlyLogHook(), HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ldalvik/system/PathClassLoader;", "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V", "dumpArgs","org/tesi/Hooks/OnlyLogHook", new OnlyLogHook(), HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ldalvik/system/BaseDexClassLoader;", "<init>", "(Ljava/lang/String;Ljava/io/File;Ljava/lang/String;Ljava/lang/ClassLoader;)V", "dumpArgs","org/tesi/Hooks/OnlyLogHook", new OnlyLogHook(), HookType.AFTER_HOOK),
			new DalvikHookImpl("Ljava/lang/ClassLoader;", "loadClass", "(Ljava/lang/String;Z)Ljava/lang/Class;", "myLoadClass","org/tesi/Hooks/OnlyLogHook", new OnlyLogHook(), HookType.AFTER_HOOK),
			/** FILE SYSTEM HOOK */		
			//new DalvikHookImpl("Ljava/io/File;", "<init>", "(Ljava/lang/String;)V", "","", null,2, 1,HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ljava/io/File;", "<init>", "(Ljava/net/URI;)V", "","",null, 2, 1,HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ljava/io/File;", "<init>", "(Ljava/io/File;Ljava/lang/String;)V", "", "",null,3,2,HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ljava/io/File;", "<init>", "(Ljava/lang/String;Ljava/lang/String;)V", "", "",null,3,2,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljava/io/File;", "setWritable", "(ZZ)Z", "check_fs_perm","org/tesi/Hooks/FileSystemHook", new FileSystemHook(), HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljava/io/File;", "setWritable", "(Z)Z", "","",null,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljava/io/File;", "setReadable", "(ZZ)Z", "check_fs_perm","org/tesi/Hooks/FileSystemHook",new FileSystemHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljava/io/File;", "setReadable", "(Z)Z", "", "",null,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljava/io/File;", "setExecutable", "(ZZ)Z", "check_fs_perm","org/tesi/Hooks/FileSystemHook",new FileSystemHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljava/io/File;", "setExecutable", "(Z)Z", "", "",null,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljava/io/FileOutputStream;", "<init>", "(Ljava/io/File;)V", "check_dir","org/tesi/Hooks/FileSystemHook",new FileSystemHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "openFileOutput", "(Ljava/lang/String;I)Ljava/io/FileOutputStream;", "check_fs_mode","org/tesi/Hooks/FileSystemHook", new FileSystemHook(),HookType.NORMAL_HOOK),

			/** NETWORK */
			new DalvikHookImpl("Ljava/net/InetAddress;", "getByName", "(Ljava/lang/String;)Ljava/net/InetAddress;", "getbyname", "org/tesi/Hooks/NetworkHook", new NetworkHook(),HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ljava/net/Socket;", "connect", "(Ljava/net/SocketAddress;)V", "connect", "org/tesi/Hooks/NetworkHook", new NetworkHook(),HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ljava/net/ServerSocket;", "bind", "(Ljava/net/SocketAddress;)V", "bind", "org/tesi/Hooks/NetworkHook", new NetworkHook(),HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ljava/net/ServerSocket;", "bind", "(Ljava/net/SocketAddress;I)V", "bind", "org/tesi/Hooks/NetworkHook", new NetworkHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljava/net/InetSocketAddress;", "<init>", "()V", "initInetSocketAddress", "org/tesi/Hooks/NetworkHook", new NetworkHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljava/net/InetSocketAddress;", "<init>", "(Ljava/net/InetAddress;I)V", "initInetSocketAddress", "org/tesi/Hooks/NetworkHook", new NetworkHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljava/net/InetSocketAddress;", "<init>", "(Ljava/lang/String;I)V", "initInetSocketAddress", "org/tesi/Hooks/NetworkHook", new NetworkHook(),HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ljava/net/InetSocketAddress;", "<init>", "(Ljava/lang/String;IZ)V", "initInetSocketAddress", "org/tesi/Hooks/NetworkHook", new NetworkHook(),HookType.NORMAL_HOOK),		
			
			//new DalvikHookImpl("Ljava/net/InetAddress;", "getAllByName", "(Ljava/lang/String;)[Ljava/net/InetAddress;", "getbyname", "org/tesi/Hooks/NetworkHook", 1, 1, HookType.NORMAL_HOOK)
			new DalvikHookImpl("Landroid/provider/Settings$Secure;", "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;","", "",null, HookType.NORMAL_HOOK),
			/** SSL */
			new DalvikHookImpl("Ljava/security/Signature;","verify","([B)Z","","",null,HookType.TRUE_HOOK),
			new DalvikHookImpl("Ljava/security/Signature;","verify","([BII)Z","","",null,HookType.TRUE_HOOK),
			new DalvikHookImpl("Lorg/apache/harmony/xnet/provider/jsse/OpenSSLSocketImpl;", "verifyCertificateChain", "([[BLjava/lang/String;)V", "","", null,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lorg/apache/http/client/methods/HttpGet;", "<init>", "(Ljava/lang/String;)V", "check_uri", "org/tesi/Hooks/SSLHook", new SSLHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lorg/apache/http/client/methods/HttpPost;", "<init>", "(Ljava/lang/String;)V", "check_uri", "org/tesi/Hooks/SSLHook",new SSLHook(), HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljavax/net/ssl/SSLContextSpi;", "engineInit", "([Ljavax/net/ssl/KeyManager;[Ljavax/net/ssl/TrustManager;Ljava/security/SecureRandom;)V", "check_trust_manager", "org/tesi/Hooks/SSLHook",new SSLHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljavax/net/ssl/HttpsURLConnection;", "setSSLSocketFactory", "(Ljavax/net/ssl/SSLSocketFactory;)V","check_trust_socketfactory", "org/tesi/Hooks/SSLHook", new SSLHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lorg/apache/http/conn/ssl/SSLSocketFactory;", "setHostnameVerifier", "(Lorg/apache/http/conn/ssl/X5HookType.NORMAL_HOOK9HostnameVerifier;)V", "check_hostname_verifier", "org/tesi/Hooks/SSLHook", new SSLHook(),HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ljavax/net/ssl/HttpsURLConnection;", "setDefaultHostnameVerifier", "(Ljavax/net/ssl/HostnameVerifier;)V", "check_hostname_verifier", "org/tesi/Hooks/SSLHook", 1,1,HookType.NORMAL_HOOK),
			
			/** Emulator vs RealPhone 
			new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getDeviceId","()Ljava/lang/String;","","",null,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getLine1Number","()Ljava/lang/String;","","",null,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getSubscriberId","()Ljava/lang/String;","","",null,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getSimSerialNumber","()Ljava/lang/String;","","",null,HookType.NORMAL_HOOK),
			*/
			/** beat reflection */
			new DalvikHookImpl("Ljava/lang/Class;", "getMethod", "(Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;", "","",null,HookType.NORMAL_HOOK),

			/** SQLite  
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "execSQL", "(Ljava/lang/String;)V", "sql_intro","org/tesi/Hooks/SQLiteHook", new SQLiteHook(), 2,1 ,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "execSQL", "(Ljava/lang/String;[Ljava/lang/Object;)V", "sql_intro","org/tesi/Hooks/SQLiteHook", new SQLiteHook(),3, 2,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "update", "(Ljava/lang/String;Landroid/content/ContentValues;Ljava/lang/String;[Ljava/lang/String;)I", "my_update","org/tesi/Hooks/SQLiteHook",new SQLiteHook(), 5, 4, HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "updateWithOnConflict","(Ljava/lang/String;Landroid/content/ContentValues;Ljava/lang/String;[Ljava/lang/String;I)I", "","", null,6 ,5 ,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "insert", "(Ljava/lang/String;Ljava/lang/String;Landroid/content/ContentValues;)J", "my_insert","org/tesi/Hooks/SQLiteHook",new SQLiteHook(),4,3,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "insertOrThrow","(Ljava/lang/String;Ljava/lang/String;Landroid/content/ContentValues;)J","","",null,4,3,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "insertWithOnConflict","(Ljava/lang/String;Ljava/lang/String;Landroid/content/ContentValues;I)J","","",null,5,4,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "replace", "(Ljava/lang/String;Ljava/lang/String;Landroid/content/ContentValues;)J", "","",null,4, 3, HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/database/sqlite/SQLiteDatabase;", "replace", "(Ljava/lang/String;Ljava/lang/String;Landroid/content/ContentValues;)J", "","",null,4, 3,HookType.NORMAL_HOOK),
			*/
			/** SHARED PREFERENCES */
			//new DalvikHookImpl("Landroid/content/ContextWrapper;", "getSharedPreferences", "(Ljava/lang/String;I)Landroid/content/SharedPreferences;", "", 3,2,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getString", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;", "getSharedPref", "org/tesi/Hooks/SharedPrefHook", new SharedPrefHook(),HookType.AFTER_HOOK),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getStringSet", "(Ljava/lang/String;Ljava/util/Set;)Ljava/util/Set;", "getSharedPref","org/tesi/Hooks/SharedPrefHook", new SharedPrefHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getAll", "()Ljava/util/Map;", "","", null,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getBoolean", "(Ljava/lang/String;Z)Z", "", "",null,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getFloat", "(Ljava/lang/String;F)F", "","",null, HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getInt", "(Ljava/lang/String;I)I", "", "",null,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "getLong", "(Ljava/lang/String;J)J", "","",null, HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/app/SharedPreferencesImpl;", "contains", "(Ljava/lang/String;)Z", "","",null, HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/content/SharedPreferences$Editor;", "putString", "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/SharedPreferences$Editor;", "put_pref","org/tesi/Hooks/SharedPrefHook", new SharedPrefHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/content/SharedPreferences$Editor;", "putBoolean", "(Ljava/lang/String;Z)Landroid/content/SharedPreferences$Editor;", "put_pref","org/tesi/Hooks/SharedPrefHook", new SharedPrefHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/content/SharedPreferences$Editor;", "putInt", "(Ljava/lang/String;I)Landroid/content/SharedPreferences$Editor;", "put_pref","org/tesi/Hooks/SharedPrefHook", new SharedPrefHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/content/SharedPreferences$Editor;", "commit", "()Z", "", "", null, HookType.NORMAL_HOOK),		

			/** IPC */
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "startService", "(Landroid/content/Intent;)Landroid/content/ComponentName;", "dump_intent","org/tesi/Hooks/IPCHooks", new IPCHooks(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "startActivities", "([Landroid/content/Intent;)V", "", "", null,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/app/Activity;", "startActivity", "(Landroid/content/Intent;)V", "dump_intent","org/tesi/Hooks/IPCHooks", new IPCHooks(), HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "sendBroadcast", "(Landroid/content/Intent;)V", "dump_intent","org/tesi/Hooks/IPCHooks", new IPCHooks(), HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "sendBroadcast", "(Landroid/content/Intent;Ljava/lang/String;)V", "dump_intent","org/tesi/Hooks/IPCHooks", new IPCHooks(), HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "registerReceiver", "(Landroid/content/BroadcastReceiver;Landroid/content/IntentFilter;)Landroid/content/Intent;", "ipc_receiver", "org/tesi/Hooks/IPCHooks", new IPCHooks(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "registerReceiver", "(Landroid/content/BroadcastReceiver;Landroid/content/IntentFilter;Ljava/lang/String;Landroid/os/Handler;)Landroid/content/Intent;", "ipc_receiver","org/tesi/Hooks/IPCHooks",  new IPCHooks() ,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/app/Application;", "onCreate", "()V", "getcon","org/tesi/utils/AppContextConfig", new AppContextConfig(), HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "grantUriPermission", "(Ljava/lang/String;Landroid/net/Uri;I)V", "","", null, HookType.NORMAL_HOOK),		

			/** CRYPTO */
			new DalvikHookImpl("Ljava/security/MessageDigest;", "update", "([B)V", "hook_hash", "org/tesi/Hooks/HashHook", new HashHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljava/security/MessageDigest;", "update", "([BII)V", "hook_hash","org/tesi/Hooks/HashHook", new HashHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljava/security/MessageDigest;", "update", "(Ljava/nio/ByteBuffer;)V", "hook_hash", "org/tesi/Hooks/HashHook", new HashHook(),HookType.NORMAL_HOOK),	
			//new DalvikHookImpl("Ljava/security/MessageDigest;", "digest", "([BII)I", "", "", null,3, 2,HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ljava/security/MessageDigest;", "digest", "()[B", "", "", null,1, HookType.NORMAL_HOOK,HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ljava/security/MessageDigest;", "digest", "([B)[B", "","",  null,2, 1,HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "doFinal", "()[B", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(), HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "doFinal", "(Ljava/nio/ByteBuffer;Ljava/nio/ByteBuffer;)I", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "doFinal", "([B)[B", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "doFinal", "([BII[B)I", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "doFinal", "([BII[BI)I", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "doFinal", "([BII)[B", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "doFinal", "([BI)I", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lorg/apache/http/conn/ssl/SSLSocketFactory;", "<init>", "(Ljava/security/KeyStore;Ljava/lang/String;)V", "keystore","org/tesi/Hooks/HookCryptoKey", new HookCryptoKey(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lorg/apache/http/conn/ssl/SSLSocketFactory;", "<init>", "(Ljava/security/KeyStore;Ljava/lang/String;Ljava/security/KeyStore;)V", "keystore","org/tesi/Hooks/HookCryptoKey", new HookCryptoKey(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Lorg/apache/http/conn/ssl/SSLSocketFactory;", "<init>", "(Ljava/lang/String;Ljava/security/KeyStore;Ljava/lang/String;Ljava/security/KeyStore;Ljava/security/SecureRandom;Lorg/apache/http/conn/scheme/HostNameResolver;)V", "keystore_hostname","org/tesi/Hooks/HookCryptoKey", new HookCryptoKey(),HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Ljavax/crypto/spec/PBEKeySpec;", ",<init>", "([B)[B", "doFinal2","org/tesi/Hooks/CryptoHook", new CryptoHook(),2, 1,HookType.NORMAL_HOOK),
			//new DalvikHookImpl("Lorg/apache/http/conn/ssl/SSLSocketFactory;", "digest", "([BII)I", "", 3, 2,HookType.NORMAL_HOOK),	
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "init", "(ILjava/security/Key;)V", "crypto_init","org/tesi/Hooks/CryptoHook", new CryptoHook(), HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "init", "(ILjava/security/Key;Ljava/security/SecureRandom;)V", "crypto_init","org/tesi/Hooks/CryptoHook", new CryptoHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "init", "(ILjava/security/Key;Ljava/security/spec/AlgorithmParameterSpec;Ljava/security/SecureRandom;)V", "crypto_init","org/tesi/Hooks/CryptoHook", new CryptoHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "init", "(ILjava/security/Key;Ljava/security/AlgorithmParameters;Ljava/security/SecureRandom;)V", "crypto_init","org/tesi/Hooks/CryptoHook", new CryptoHook(), HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "init", "(ILjava/security/Key;Ljava/security/AlgorithmParameters;)V", "crypto_init","org/tesi/Hooks/CryptoHook", new CryptoHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Ljavax/crypto/Cipher;", "init", "(ILjava/security/Key;Ljava/security/spec/AlgorithmParameterSpec;)V", "crypto_init","org/tesi/Hooks/CryptoHook", new CryptoHook(),HookType.NORMAL_HOOK),
			
			/** WebView */
			new DalvikHookImpl("Landroid/webkit/WebSettings;","setJavaScriptEnabled", "(Z)V","webview_set","org/tesi/Hooks/WebviewHook", new WebviewHook(), HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/webkit/WebSettings;","setAllowFileAccess", "(Z)V","webview_set","org/tesi/Hooks/WebviewHook", new WebviewHook(),HookType.NORMAL_HOOK),
			new DalvikHookImpl("Landroid/webkit/WebView;","addJavascriptInterface", "(Ljava/lang/Object;Ljava/lang/String;)V","webview_js","org/tesi/Hooks/WebviewHook", new WebviewHook(),HookType.NORMAL_HOOK),

			/** Anti emulator detection 
			new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getDeviceId", "()Ljava/lang/String;","myGetDeviceId","org/tesi/Hooks/AntiEmulatorDetectionHook", new AntiEmulatorDetectionHook(),HookType.NOX_HOOK),
			new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getSubscriberId", "()Ljava/lang/String;","myGetSubscriberId","org/tesi/Hooks/AntiEmulatorDetectionHook", new AntiEmulatorDetectionHook(),HookType.NOX_HOOK),
			new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getNetworkOperatorName", "()Ljava/lang/String;","myGetNetworkOperatorName","org/tesi/Hooks/AntiEmulatorDetectionHook", new AntiEmulatorDetectionHook(),HookType.NOX_HOOK),
			new DalvikHookImpl("Landroid/telephony/TelephonyManager;","getLine1Number", "()Ljava/lang/String;","myGetLine1Number","org/tesi/Hooks/AntiEmulatorDetectionHook", new AntiEmulatorDetectionHook(),HookType.NOX_HOOK),
			new DalvikHookImpl("Landroid/os/SystemProperties;","get", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;","myGet","org/tesi/Hooks/AntiEmulatorDetectionHook", new AntiEmulatorDetectionHook(),HookType.NOX_HOOK),
			new DalvikHookImpl("Landroid/os/SystemProperties;","get", "(Ljava/lang/String;)Ljava/lang/String;","myGet2","org/tesi/Hooks/AntiEmulatorDetectionHook", new AntiEmulatorDetectionHook(),HookType.NOX_HOOK),
			new DalvikHookImpl("Landroid/content/pm/PackageManager;","getInstallerPackageName", "(Ljava/lang/String;)Ljava/lang/String;","myGetInstallerPackageName","org/tesi/Hooks/AntiEmulatorDetectionHook", new AntiEmulatorDetectionHook(),HookType.FALSE_HOOK),
			*/
			
			/** Disable Signature and check */
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkCallingOrSelfPermission", "(Ljava/lang/String;)I", "", "",null,HookType.TRUE_HOOK),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkCallingOrSelfUriPermission", "(Landroid/net/Uri;I)I", "", "",null,HookType.TRUE_HOOK),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkPermission", "(Ljava/lang/String;II)I", "", "",null,HookType.TRUE_HOOK),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkCallingPermission", "(Ljava/lang/String;)I", "", "",null,HookType.TRUE_HOOK),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkCallingUriPermission", "(Landroid/net/Uri;I)I", "", "",null,HookType.TRUE_HOOK),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkUriPermission", "(Landroid/net/Uri;III)I", "", "",null,HookType.TRUE_HOOK),
			new DalvikHookImpl("Landroid/content/ContextWrapper;", "checkUriPermission", "(Landroid/net/Uri;Ljava/lang/String;Ljava/lang/String;III)I", "", "",null,HookType.TRUE_HOOK),
			new DalvikHookImpl("Landroid/content/pm/PackageManager;", "checkPermission", "(Ljava/lang/String;Ljava/lang/String;)I", "", "",null,HookType.TRUE_HOOK),
			new DalvikHookImpl("Landroid/app/ContextImpl;", "checkPermission", "(Ljava/lang/String;II)I", "", "",null, HookType.TRUE_HOOK),
			new DalvikHookImpl("Landroid/app/ContextImpl;", "checkCallingPermission", "(Ljava/lang/String;)I", "", "",null, HookType.TRUE_HOOK),
			new DalvikHookImpl("Landroid/app/ActivityManagerNative;", "checkPermission", "(Ljava/lang/String;II)I", "", "",null, HookType.TRUE_HOOK),
			new DalvikHookImpl("Landroid/app/ApplicationPackageManager;", "checkPermission", "(Ljava/lang/String;Ljava/lang/String;)I", "", "",null, HookType.FALSE_HOOK)
			//new DalvikHookImpl("android/content/pm/IPackageManager$Stub$Proxy;", "checkPermission", "(Ljava/lang/String;Ljava/lang/String;)I", "", "",3,2,1),
			//new DalvikHookImpl("Lcom/whatsapp/contact/ContactProvider;",  "onCreate", "()Z", "","", null, 1,HookType.NORMAL_HOOK,HookType.NORMAL_HOOK),
		
		)); 
	
	
	static public List<?> getHookList() {
		return _hookList;

	}

}
