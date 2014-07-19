package org.tesi.Hooks;

import java.net.InetAddress;
import java.net.SocketAddress;
import java.net.UnknownHostException;

import org.sid.addi.core.DEXHook;

import android.util.Log;

public class NetworkHook extends DEXHook{
	public void getbyname(Object...args){
		
		String host = (String)args[0];
		String out = " -> "+host;
		Log.i(_TAG, "Connessione ad host: " + host);
		Log.i(_TAG, "Sono: " + _className);
		try {
			InetAddress i = InetAddress.getByName(host);
			System.out.println("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" + i.getHostName());
			_logParameter("InetAddress", host);
			out += i.getHostName();
			System.out.println("AAAAAAAAAA SALVO NEL DB!");
			_logFlush_I(out, this);
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		//_logParameter("Ouput (Decrypt)", "cippa");
		//_logLine("-> DECRYPT: [" + host + "]");
	}
	public void connect(Object...args){
		Log.i(_TAG,"dentro connect!!!");
		SocketAddress sa = (SocketAddress) args[0];
		Log.i(_TAG,"CONNECT ADDRESS = "+sa.toString());
	}
	public void bind(Object...args){
		Log.i(_TAG,"dentro bind!!!");
		SocketAddress sa = (SocketAddress) args[0];
		Log.i(_TAG,"BIND ADDRESS = "+sa.toString());	
	}
	public  void getbyaddress(Object... args){
		Log.i(_TAG, "chiamato  getbyaddress");
	}
	public void initInetSocketAddress(Object... args){
		Log.i(_TAG,"---------CHIAMATO initInetSocketAddress");
		
	}
}
