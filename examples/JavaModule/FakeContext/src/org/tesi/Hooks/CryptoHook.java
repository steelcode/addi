package org.tesi.Hooks;

import java.security.Key;
import java.util.Stack;

import javax.crypto.Cipher;

import org.sid.addi.core.DEXHook;

import android.util.Log;

public class CryptoHook extends DEXHook{
	public static 	Stack<byte[]> IVList = new Stack<byte[]>();
	public static String 	_out = "";
	public static boolean 	_warning = false;
	Cipher cipher = null;
	protected static Cipher 	_lastCipher;
	protected static Integer 	_lastMode;
	
	
	protected void _getIV(Cipher cipher) {
		Log.i(_TAG, " DENTRO GET IV");
		if (cipher.getIV() != null) {
			String iv = _getReadableByteArr(cipher.getIV());
			_out += "; IV: " + iv;
			_logParameter("IV", iv);

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
	
	protected void _getAlgo(Cipher cipher) {
		Log.i(_TAG, " DENTRO GET ALGO");
		String algo = cipher.getAlgorithm();
		if (algo != null) {
			_out = "-> Algo: " + algo;
			_logParameter("Algo", algo);
			Log.i(_TAG, algo);
			if (cipher.getAlgorithm().contains("ECB")) {
				_warning = true;
				_out += " - !!! ECB used. ECB mode is broken and should not be used.";
			}
		}
	}
	
	private void _getInput(byte[] data) {
		Log.i(_TAG, " DENTRO GET INPUT");

		if (data != null && data.length != 0) { // when no args to doFinal (used only update())
			String i_sdata = null;
			i_sdata = new String(data);
			if (i_sdata != null && !i_sdata.isEmpty()) {
				if (_isItReadable(i_sdata)) {
					i_sdata = _byteArrayToReadableStr(data);
					Log.i(_TAG, " INPUT DATA = "+ i_sdata);
					_logParameter("input (Encrypt)", i_sdata);
					_logLine("-> ENCRYPT: [" + i_sdata + "]");
				}
				else {
					String sdata = _byteArrayToB64(data);
					Log.i(_TAG, " INPUT DATA = "+ sdata);
					_logLine("-> Input data is not in a readable format, " +
								"base64: ["+ sdata +"]");
					_logParameter("Output (converted to b64)", sdata);
				}
			}
		}
	}

	@SuppressWarnings("unused")
	private void _getOutput(Object... args) {
		Log.i(_TAG, " DENTRO GET OUTPUT");
		byte[] data = null;
		String o_sdata = null;
//		if (cipher == _lastCipher && _lastMode == Cipher.DECRYPT_MODE)
			try {
				//data  = (byte[]) _hookInvoke(args);
				if(args.length != 0 &&  args[0] != null){
					Log.i(_TAG, " DENTRO GET OUTPUT CHIAMO CON ARGOMENTI");
					byte[] in = (byte[]) args[0];
					Object[] myargs = new Object[]{args};
					data = cipher.doFinal(in);
				}else{
					data = cipher.doFinal();
				}

			}
			catch (Throwable e) {
				Log.i(_TAG_ERROR, "doFinal function failed: "+e);
			}
			if (data != null) {
				o_sdata = new String(data);
				if (_isItReadable(o_sdata)) {
					o_sdata = _byteArrayToReadableStr(data);
					Log.i(_TAG, " OUTPUT DATA = "+ o_sdata);
					_logParameter("Ouput (Decrypt)", o_sdata);
					_logLine("-> DECRYPT: [" + o_sdata + "]");
				}
				else {
					String sdata = _byteArrayToB64(data);
					Log.i(_TAG, " OUTPUT DATA = "+ sdata);
					
					_logLine("-> Output data is not in a readable format," +
								" base64: ["+ sdata +"]");
					_logReturnValue("Output (converted to b64)", sdata);
				}
			}
	}
	
	public void doFinal2(Object... args) {
		Log.i(_TAG,"-------------- DEnTRO DOFINAL2-----------");
		if (_thiz != null) {
			_warning = false;
			_out = "";
			//Cipher cipher = (Cipher) _resources;
			Log.i(_TAG,"--------------  DOFINAL2 prima di cipher-----------");
			cipher = (Cipher) _hook.getThiz();
			Log.i(_TAG,"-------------- DEnTRO dopo  cipher-----------");
			
			if(cipher == null){
				Log.i(_TAG, " ERRRORE CIPHER ON CRYPTO EXECUTE");
				return;
			}
			else{
				Log.i(_TAG, "  CIPHER ON CRYPTO EXECUTE");
			}
			//_logBasicInfo();

			// input
			if (args.length != 0 && args[0] != null) {
				try {
					_getInput((byte[]) args[0]);
				}
				catch (Exception e) {
					Log.w(_TAG_ERROR, "Error in _getInput " +
							"(CRYPTO_IMPL->dofinal): " + e);
				}
			}
			
			//output
			try {
				_getOutput(args);
			}
			catch (Exception e) {
				Log.w(_TAG_ERROR, "Error in _getOutput " +
						"(CRYPTO_IMPL->dofinal): " + e);
			}
			
			// algo/IV
			try {
				_getAlgo(cipher);
				_getIV(cipher);
			}
			catch (Exception e) {
				Log.w(_TAG_ERROR, "Error in _getAlgo/_getCipher " +
						"(CRYPTO_IMPL->dofinal): " + e);
			}

			// dump some params
			if (cipher.getParameters() != null)// && ApplicationConfig.g_debug)
				_logLine("Parameters: " + cipher.getParameters());
			
			if (_warning)
				_logFlush_W(_out, this);
			else if (!_out.isEmpty())
				_logFlush_I(_out, this);
		
		}
		else {
			Log.w(_TAG_ERROR, 
					"Error in Intro_CRYPTO: resource is null");
		}
	}
	public void crypto_init(Object... args){
		Log.i(_TAG,"DENTRO CRYPTO INIT ");
		// let's not care about init since we are hooking 
		// the key class already
		// BUT it can be useful to get a state of the mode 
		// if needed later
		if (_thiz != null) {
			Log.i(_TAG,"DENTRO CRYPTO INIT THIZ NON NULLO!!!");			
			try {
				_lastCipher = (Cipher) _thiz;
				
				// get the mode
				Integer mode = _lastMode = (Integer) args[0];
				String smode;
				switch (mode) {
					case Cipher.DECRYPT_MODE: 
						smode = "DECRYPT_MODE";
						break;
					case Cipher.ENCRYPT_MODE: 
						smode = "ENCRYPT_MODE";
						break;
					default: 
						smode = "???";
				}
				
				_logBasicInfo(this);
				
				String out = "-> Mode: " + smode;
				
				// get the key
				Key key = (Key) args[1];
				String skey = "";
				if (key != null) {
					skey = _getReadableByteArr(key.getEncoded());
					out += ", Key format: " + key.getFormat() + 
							", Key: [" + skey + "]";
				}
				_logParameter("Mode", smode);
				_logParameter("Key", skey);
				_logParameter("Key Format", key.getFormat());

				_logFlush_I(out, this);
				
			} catch (Exception e) {
				Log.w(_TAG_ERROR, "Error in Intro_CRYPTO: " + e);
			}
			
		}
	}
}
