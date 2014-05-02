package org.tesi.Hooks;

import java.security.cert.X509Certificate;

import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import org.apache.http.conn.ssl.SSLSocketFactory;
import org.tesi.fakecontext.DalvikHookImpl;

import android.util.Log;

public class SSLHook extends DalvikHookImpl{

	public static void check_trust_manager(Object... args){
		Log.i(_TAG," -------- CHECK TRUST MANAGER CHIAMATO");
		TrustManager[] tm_arr = (TrustManager[]) args[1];
		// check the trust manager
		if (tm_arr != null && tm_arr[0] != null) {
			X509TrustManager tm = (X509TrustManager) tm_arr[0];
			X509Certificate[] chain = new X509Certificate[]{};
			boolean check = false;
			try {
				tm.checkClientTrusted(chain, "");
				tm.checkServerTrusted(chain, "");
			} catch (Exception e) { // should change to CertificateException
				// if it goes here with an invalid cert
				// the app may verify certs
				check = true;
			}
			if (check)
				Log.i(_TAG,"The app does not verify SSL certs");
			else
				Log.i(_TAG,"Use of a custom Trust Manager, " +
						"the app may do cert. pinning OR validate any cert");
		}
	}

	public static void check_hostname_verifier(Object... args){
		Log.i(_TAG," -------- CHECK HOSTNAME VERIFIER CHIAMATO");
		if ((org.apache.http.conn.ssl.X509HostnameVerifier)args[0] == 
				SSLSocketFactory.ALLOW_ALL_HOSTNAME_VERIFIER) {
			Log.i(_TAG,"HostNameVerifier set to accept ANY hostname");
		}		
	}
	
	public static void check_uri(Object... args){
		Log.i(_TAG," -------- CHECK URI CHIAMATO");
		String uri = (String) args[0];
		if (uri.contains("http:")) {
			Log.i(_TAG,"No SSL: ["+uri+"]");
		}
	}
}
