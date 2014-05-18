package org.tesi.Hooks;

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.sid.addi.core.DEXHook;

import android.util.Log;

public class NetworkHook extends DEXHook{
	public void getbyname(Object...args){
		
		if(_hook == null){
			System.out.println("------------------------------------------------------PORCODIOOOOOO");
			return;
		}
		
		if(_hook.isHooked()){
			System.out.println("--------------CONTROLLO E RIMUOVO PORCODIDIO");
			_hook.myunhook();
			System.out.println("--------------FINE CONTROLLO E RIMUOVO PORCODIDIO");
		}
		
		System.out.println("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
		if(_thiz != null){
			System.out.println("Thiz vale = " + _thiz.toString());
		}
		else{
			System.out.println("THIZ NULL!!!!!");
		}
		String host = (String)args[0];
		Log.i(_TAG, "Connessione ad host: " + host);
		Log.i(_TAG, "Sono: " + _className);
		try {
			InetAddress i = InetAddress.getByName(host);
			System.out.println("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" + i.getHostName());
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		//_logParameter("Ouput (Decrypt)", "cippa");
		//_logLine("-> DECRYPT: [" + host + "]");
	}
	public  void getbyaddress(Object... args){
		Log.i(_TAG, "chiamato  getbyaddress");
	}
}
