/**
 * @author Valerio Costamagna
 *
 */
package org.tesi.Hooks;

import org.sid.addi.core.DEXHook;

/**
 * @author Valerio Costamagna
 *
 */
public class HookCryptoKey extends DEXHook{
	
	public void get_key(Object... args){
		byte[] key = (byte[]) args[0];
		if (key != null) {
			String skey = _getReadableByteArr(key);
			_logParameter("Key", skey);
			_logParameter("Algo", args[1]);
			_logBasicInfo(this);
			_logFlush_I("-> Key: ["+skey+"], algo: "+args[1], this);
		}
	}
	
	public void keystore_hostname(Object... args){
		_logBasicInfo(this);
		// arg2 is the passcode for this trustore
		if (args[2] != null) {
			String passcode = 
					_getReadableByteArr((byte[]) args[2]);
			_logParameter("Passcode", args[2]);
			_logFlush_I("-> TrustStore passcode: " + passcode, this);
		}
	}
	
	public void keystore(Object... args){
		_logBasicInfo(this);
		// arg1 is the passcode for the trustore
		if (args[1] != null) {
			String passcode = 
					_getReadableByteArr((byte[]) args[1]);
			_logParameter("Passcode", args[1]);
			_logFlush_I("-> TrustStore passcode: " + passcode, this);
		}
	}
	
	public void pbekey(Object... args){
		_logBasicInfo(this);

		String passcode = new String((char[])args[0]);
		String salt = null;
		int iterationCount = -1;
		if (args.length >= 2 && args[1] != null) {
			salt = 
				_byteArrayToReadableStr((byte[])args[1]);
			iterationCount = (Integer)args[2];
			_logParameter("Passcode", passcode);
			_logParameter("Salt", salt);
			_logParameter("Iterations", iterationCount);
			// _logReturnValue("Key", _hookInvoke(args));
			_logFlush_I("-> Passcode: [" + passcode + "], Salt: [" + salt + 
					"], iterations: " + iterationCount + "", this);
		}
		else {
			_logParameter("Passcode", passcode);
			_logFlush_I("Passcode: [" + passcode + "]", this);
		}
	}
	
	
	
	
}
