/**
 * @author Valerio Costamagna
 *
 */
package org.tesi.Hooks;


import java.lang.reflect.Method;
import java.security.MessageDigest;

import org.sid.addi.core.DEXHook;

import android.util.Base64;
import android.util.Log;

public class HashHook extends DEXHook{
	protected MessageDigest dg = null;
	
	//chiamato su update()
	public void hook_hash(Object... args) {
		Log.i(_TAG, "DENTRO HOOK HASH!! BBBB");
		String _out = "";
	    StackTraceElement[] ste = Thread.currentThread().getStackTrace();
	    
	    // this is called within apps and is super noisy so not displaying it
	    if (ste[7].toString().contains("com.crashlytics."))
	    	return;

	    StringBuilder sb = new StringBuilder();
	    for (StackTraceElement element : ste) {
	        sb.append(element.toString());
	        sb.append("\n");
	    }
	    //Log.i(_TAG,sb.toString());
	    
		if (args[0] != null) {
			Log.i(_TAG, "DENTRO HOOK HASH!! ARGS[0] NOT NULL");
           // _logBasicInfo();
            String input = _getReadableByteArr((byte[])args[0]);            
            byte[] output = null;
            String s_output = "";
            try {
                    // execution the method to calculate the digest
                    // using reflection to call digest from the object's instance
                    try {
                    	/*
                    		Log.i(_TAG, "DENTRO HOOK HASH  faccio mie cose");
				   			MessageDigest md = MessageDigest.getInstance("SHA-256");
				   			Log.i(_TAG, "DENTRO HOOK HASH CHIAMO MIO UPDATE");
				   			md.update(input.getBytes());
				   			Log.i(_TAG, "DENTRO HOOK HASH CHIAMO MIO DIGEST");
				   			byte byteData[] = md.digest();
				   	        StringBuffer sb1 = new StringBuffer();
				   	        for (int i = 0; i < byteData.length; i++) {
				   	         sb1.append(Integer.toString((byteData[i] & 0xff) + 0x100, 16).substring(1));
				   	        }
				   	        */
				   			/*
                            Class<?> cls = Class.forName("java.security.MessageDigest");
                            MessageDigest obj = (MessageDigest) _thiz;
                         
                            Class<?> noparams[] = {};
                            Class<?> bytep[] = {byte[].class};
                            Method umethod = cls.getDeclaredMethod("update", bytep);
                            umethod.invoke(obj, (byte[])args[0]);
                            Method xmethod = cls.getDeclaredMethod("digest", noparams);
                            output = (byte[]) xmethod.invoke(obj);
                            StringBuffer sb2 = new StringBuffer();
				   	        for (int i = 0; i < output.length; i++) {
					   	         sb2.append(Integer.toString((output[i] & 0xff) + 0x100, 16).substring(1));
					   	    }

				   	        //Log.i(_TAG,"Hex format : " + sb1.toString());
				   	        Log.i(_TAG,"Hex format : " + sb2.toString());
                            s_output = Base64.encodeToString(sb2.toString().getBytes(), Base64.DEFAULT);
                            //s_output = _getReadableByteArr(output);
                            Log.i(_TAG, "DENTRO HOOK HASH finito mie cose");
                            */
                    }
                    catch (Exception e) {
                    	Log.w(_TAG_ERROR, "Error in Hash func: " + e);
                    }
            }
            catch (Throwable e) {
            	Log.w(_TAG_ERROR, "Error in Hash func: " + e);
            }

            // use reflection to call a method from this instance
            String algoName = null;
            /**
            try {
                    Class<?> cls = Class.forName("java.security.MessageDigest");
                    Object obj =_thiz;
                    Class<?> noparams[] = {};
                    Method xmethod = cls.getDeclaredMethod("getAlgorithm", noparams);
                    algoName = (String) xmethod.invoke(obj);
            }
            catch (Exception e) {
                    algoName = "error: " + e;        
            }
            //
            //Log.i(_TAG, " HASH of : [ " + input + "] is : ["+s_output +"],  alg: [" + algoName+"]");
            
           _logLine("-> Hash of : [" + input + "] is: [" + 
                            s_output +"] , Algo: [" + algoName + "]");
            _out += input+s_output+algoName;
           _logParameter("Input", input);
           _logParameter("Algo", algoName);
           _logReturnValue("Output", s_output);
            
            if (algoName.contains("MD5")) {
                    _logFlush_W("MD5 used, this hashing algo " +
                    		"is broken and should not be used", this);
            }
            //_logFlush_I(this);
            _logFlush_I(_out,this);
            */
            Log.i(_TAG,"HO CALCOLATO UN HASH");
		}
	}

}
