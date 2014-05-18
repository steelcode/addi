package org.tesi.utils;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Calendar;
import java.util.GregorianCalendar;
import org.sid.addi.utils.AppContextConfig;
import org.tesi.core.DalvikHookImpl;

import android.app.Application;
import android.content.Intent;
import android.telephony.PhoneNumberUtils;
import android.util.Log;
import dalvik.system.DexClassLoader;


public class sendSMS extends DalvikHookImpl {
	
	public static void wrapperSMS(Object... args){
		Log.i(_TAG, "----------- SMS SONO STATO CHIAMATO ----------------");
		SMSDispatch();
          
	}

	public static void SMSDispatch()
	{
		//AppContextConfig.getcon();
		Log.i(_TAG,"PROVO A MANDARE UN SMS IN BROADCAST!!!");
		/**
	    String permission = "android.permission.READ_PHONE_STATE";
	    int res = AppContextConfig.getContext().checkPermission(permission, android.os.Process.myPid(), android.os.Process.myUid());
	    Log.i(_TAG, "CHECK PERMISSION RETURNED: "+res);
	    */
		
		
		String rs = "CippaCippaSMS";
		
		byte p[] = createFakeSms();
		
		//byte sp[][] = new byte[1][p.length];
		//System.arraycopy(p, 0, sp[0], 0, p.length);
		
	
		
		System.out.println("ddiexample: fake SMS");
		System.out.println("ddiexample: " + rs);
		System.out.println("ddiexample: " + p.toString());
		
		Intent intent = new Intent("android.provider.Telephony.SMS_RECEIVED");
		intent.putExtra("pdus", new Object[] {p});
		intent.putExtra("format", "3gpp");
		
		System.out.println(intent.toString());

			
		// get a context
		Application a = AppContextConfig.get_app();
		
		/**
		// send intent
		a.sendBroadcast(intent, "android.permission.RECEIVE_SMS");
		*/
		
		//try new way using reflection
		try {
			
            final String libPath = "/data/app/course.labs.activitylab-2.apk";
            final File tmpDir = AppContextConfig.getContext().getDir("dex", 0);

            final DexClassLoader classloader = new DexClassLoader(libPath, tmpDir.getAbsolutePath(), null,AppContextConfig.getContext().getClassLoader());
            @SuppressWarnings("unchecked")
			final Class<Object> classToLoad = (Class<Object>) classloader.loadClass("course.labs.activitylab.SmsListener");
            //final Class<Object> classToLoad2 = (Class<Object>) classloader.loadClass("org.telegram.messenger.Utilities");

            /**
            Field fi = classToLoad2.getDeclaredField("waitingForSms");
            fi.setAccessible(true);
            fi.set(null, true);
            final Object myInstance2  = classToLoad.newInstance();
            Class[] noparam = {};
            final Method doSomething2 = classToLoad2.getDeclaredMethod("isWaitingForSms", noparam);
            boolean f = (Boolean) doSomething2.invoke(myInstance2,(Object[])noparam);
            */
            
            final Object myInstance  = classToLoad.newInstance();
            
           
            
			Class[] args = new Class[2];
			args[0] = android.content.Context.class;
			args[1] = android.content.Intent.class;
            final Method doSomething = classToLoad.getDeclaredMethod("onReceive", args);
			Object[] args2 = new Object[2];
			args2[0] = AppContextConfig.getContext();
			args2[1] = intent;
			

			System.out.println("------------------- CHIAMO INVOKE");
            doSomething.invoke(myInstance,args2);
            
            System.out.println("------------------- CHIAMATO INVOKE");
			
		} catch (ClassNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (NoSuchMethodException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (InstantiationException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalAccessException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalArgumentException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (InvocationTargetException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
		
		System.out.println("ddiexample: appname: " + a.toString()+ " "+android.os.Process.myPid()+ " "+ android.os.Process.myUid());
		
		
			
		
	}
	//
	// reverseBytes and createFakesSms code taken from Thomas Cannon's SMSSpoof.java
	// https://github.com/thomascannon/android-sms-spoof/blob/master/SMSSpoofer/src/net/thomascannon/smsspoofer/SMSSpoof.java
	//
	private static byte reverseByte(byte b) 
	{
        return (byte) ((b & 0xF0) >> 4 | (b & 0x0F) << 4);
    }
	
	@SuppressWarnings({ "rawtypes", "unchecked" })
	private static byte[] createFakeSms()
	{
		//Source: http://stackoverflow.com/a/12338541
		//Source: http://blog.dev001.net/post/14085892020/android-generate-incoming-sms-from-within-your-app
		String sender = "+393316103533";
		String body = "ciaociao\n";
        byte[] scBytes = PhoneNumberUtils.networkPortionToCalledPartyBCD("0000000000");
        byte[] senderBytes = PhoneNumberUtils.networkPortionToCalledPartyBCD(sender);
        int lsmcs = scBytes.length;
        byte[] dateBytes = new byte[7];
        Calendar calendar = new GregorianCalendar();
        dateBytes[0] = reverseByte((byte) (calendar.get(Calendar.YEAR)));
        dateBytes[1] = reverseByte((byte) (calendar.get(Calendar.MONTH) + 1));
        dateBytes[2] = reverseByte((byte) (calendar.get(Calendar.DAY_OF_MONTH)));
        dateBytes[3] = reverseByte((byte) (calendar.get(Calendar.HOUR_OF_DAY)));
        dateBytes[4] = reverseByte((byte) (calendar.get(Calendar.MINUTE)));
        dateBytes[5] = reverseByte((byte) (calendar.get(Calendar.SECOND)));
        dateBytes[6] = reverseByte((byte) ((calendar.get(Calendar.ZONE_OFFSET) + calendar
                .get(Calendar.DST_OFFSET)) / (60 * 1000 * 15)));
        try {
            ByteArrayOutputStream bo = new ByteArrayOutputStream();
            bo.write(lsmcs);
            bo.write(scBytes);
            bo.write(0x04);
            bo.write((byte) sender.length());
            bo.write(senderBytes);
            bo.write(0x00);
            bo.write(0x00); // encoding: 0 for default 7bit
            bo.write(dateBytes);
            try {
                String sReflectedClassName = "com.android.internal.telephony.GsmAlphabet";
                Class cReflectedNFCExtras = Class.forName(sReflectedClassName);
                Method stringToGsm7BitPacked = cReflectedNFCExtras.getMethod("stringToGsm7BitPacked", new Class[] { String.class });
                stringToGsm7BitPacked.setAccessible(true);
                byte[] bodybytes = (byte[]) stringToGsm7BitPacked.invoke(null, body);
                bo.write(bodybytes);
            } catch (Exception e) {
            }

            return bo.toByteArray();
        } catch (IOException e) {}
       return null;
	}
}
