package org.tesi.Hooks;

import java.util.Stack;

import javax.crypto.Cipher;

import org.sid.addi.core.manageADDI;
import org.tesi.fakecontext.DalvikHookImpl;

import android.util.Log;

public class CryptoHook extends DalvikHookImpl{
	public static 	Stack<byte[]> IVList = new Stack<byte[]>();
	public static String 	_out = "";
	public static boolean 	_warning = false;
	
	public static void _getIV(Cipher cipher) {
		if (cipher.getIV() != null) {
			//String iv = org.tesi.fakecontext.StringHelper._getReadableByteArr(cipher.getIV());
			String iv = "";
			_out += "; IV: " + iv;
			//_logParameter("IV", iv);
			
			if (cipher.getIV()[0] == 0) {
				Log.w("Introspy", "!!! IV of 0");
				_warning = true;
			}
			else {
				// check if this IV has already been used
				if (IVList.contains(cipher.getIV())) {
					_out  += " - !!! Static IV";
					_warning = true;
				}
				IVList.push(cipher.getIV());
				// keep a list of a max of 10 IVs
				if (IVList.size() > 10)
					IVList.pop();
			}
		}
	}
	
	public static void _getAlgo(Cipher cipher) {
		String algo = cipher.getAlgorithm();
		if (algo != null) {
			_out = "-> Algo: " + algo;
			
			//_logParameter("Algo", algo);
			if (cipher.getAlgorithm().contains("ECB")) {
				_warning = true;
				_out += " - !!! ECB used. ECB mode is broken and should not be used.";
			}
		}
	}
	private void _getInput(byte[] data) {
		if (data != null && data.length != 0) { // when no args to doFinal (used only update())
			String i_sdata = null;
			i_sdata = new String(data);
			if (i_sdata != null && !i_sdata.isEmpty()) {
				if (_isItReadable(i_sdata)) {
					i_sdata = _byteArrayToReadableStr(data);
					_logParameter("input (Encrypt)", i_sdata);
					_logLine("-> ENCRYPT: [" + i_sdata + "]");
				}
				else {
					String sdata = _byteArrayToB64(data);
					_logLine("-> Input data is not in a readable format, " +
								"base64: ["+ sdata +"]");
					_logParameter("Output (converted to b64)", sdata);
				}
			}
		}
	}

	private void _getOutput(Object... args) {
		byte[] data = null;
		String o_sdata = null;
//		if (cipher == _lastCipher && _lastMode == Cipher.DECRYPT_MODE)
			try {
			manageADDI.unhook("");
			//	data  = (byte[]) _hookInvoke(args);
			}
			catch (Throwable e) {
				Log.i(_ETAG, "doFinal function failed: "+e);
			}
			if (data != null) {
				o_sdata = new String(data);
				if (_isItReadable(o_sdata)) {
					o_sdata = _byteArrayToReadableStr(data);
					_logParameter("Ouput (Decrypt)", o_sdata);
					_logLine("-> DECRYPT: [" + o_sdata + "]");
				}
				else {
					String sdata = _byteArrayToB64(data);
					_logLine("-> Output data is not in a readable format," +
								" base64: ["+ sdata +"]");
					_logReturnValue("Output (converted to b64)", sdata);
				}
			}
//		} else {
//		}
	}

	
	
	
}
