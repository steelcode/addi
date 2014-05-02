package org.tesi.Controller;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

import org.sid.addi.core.manageADDI;

import android.util.Base64;
import android.util.Log;


public class Sessionold {
	private BufferedReader input;
	private BufferedWriter output; 
	public boolean connected;
	private String sfifo_name,  cfifo_name;
	public final String _TAG = "Hooks";
	public final String _ETAG = "HooksError";
	
	public Sessionold(String fifoserver){
		Log.i(_TAG,  "------------ DENTRO SESSION");
		sfifo_name = fifoserver;
		String mess;
		manageADDI.suspendALL();
		try {
			File file = new File(sfifo_name);
			input = new BufferedReader(new FileReader(file));
			cfifo_name = receive().replace("\0", "");
			System.out.println( "-------- CLIENT FIFO RICEVUTO = "+cfifo_name);
			/**
			byte []bytes= cfifo_name.getBytes();
			for(byte b:bytes)
			  System.out.println(b);
			  */
			setOutput();
			send("BENVENUTO\n",false);
			manageADDI.resumeALL();
			connected = true;
			while(connected){
				mess = receive().replace("\0", "");
				send("BENVENUTO\n",false);
				System.out.println( "-------- LOOP CLIENT FIFO RICEVUTO = "+mess);
				//connected = false;
			}
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public void setOutput(){
		try {
			File file = new File(cfifo_name);
			output = new BufferedWriter(new FileWriter(file));
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	public boolean send(String data, boolean isbase64){
		try
		{
			//Write to socket base64 encoded with a newline on end
			if (isbase64)
				output.write(new String(Base64.encode(data.getBytes(), Base64.DEFAULT)) + "\n");
			else
			//Or not
				output.write(data);
			
			output.flush();
			
			return true;
		}
		catch (Exception e)
		{
			return false;
		}
	}
	
	public String receive(){
		String content = null;
		try {
			while(!input.ready());
			content = input.readLine();
			if(content == null)
				connected = false;
			else
				connected = true;
			return content;			
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return e.toString();
		}
	}
	
	//Send start of transmission tag
		public void startTransmission()
		{
			send("<?xml version=\"1.0\" ?><transmission>", false);	  
		}
		
		//Send start of response tag
		public void startResponse()
		{
			send("<response>", false);
		}
		
		//Send end of response tag
		public void endResponse()
		{
			send("</response>", false);
		}
		
		//Send error tag with contents
		public void error(String error)
		{
			send("<error>", false);
			send(error, true);
			send("</error>", false);
		}
		
		//Send error tag with no contents
		public void noError()
		{
			send("<error />", false);
		}
		
		//Send start of data tag
		public void startData()
		{
			send("<data>", false);
		}
		
		//Send end of data tag
		public void endData()
		{
			send("</data>", false);
		}
		
		//Send end of transmission tag and close socket
		public void endTransmission()
		{
			//Send close of transmission
			send("</transmission>", false);
			
			//Set connected to false so that server does not keep listening on this conn
			connected = false;
			try {
				output.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
}
