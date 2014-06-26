/*******************************************************************************
 * Advanced Dalvik Dynamic Instrumentation Android Library
 * 
 * (c) 2014, 
 ******************************************************************************/
package org.sid.addi.core;


import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.util.List;

import android.util.Base64;
import android.util.Log;

public class Session {
	//private File req;
	//private File res;
	private String req;
	private String res;
	private BufferedReader input;
	private BufferedWriter output;
	//private BufferedInputStream input;
	//private BufferedOutputStream output;
	public boolean connected;
	private StringBuilder data;
	
	//private String sfifo_name,  cfifo_name;
	public final String _TAG = "Hooks";
	public final String _ETAG = "HooksError";
	
	
	public Session(String request, String response){
		Log.i(_TAG,  "------------ DENTRO SESSION con req = " +request+" e res = "+response);
		
			//req = new File(request);
			//res = new File(response);
			req = request;
			res = response;
			data = new StringBuilder();
			Commands.init();
			
		/**	
			sfifo_name = fifoserver;
			String mess;
			manageADDI.suspendALL();
			File file = new File(sfifo_name);
			input = new BufferedReader(new FileReader(file));
			cfifo_name = receive().replace("\0", "");
			setOutput(cfifo_name);
			sendFullTransmission("BENVENUTO","");
			manageADDI.resumeALL();
			connected = true;
			while(connected){
			handleCommand(receive().replace("\0", ""));
		*/	
			/**
			System.out.println( "-------- CLIENT FIFO RICEVUTO = "+cfifo_name);
			setOutput();
			send("BENVENUTO\n",false);
			manageADDI.resumeALL();
			connected = true;
				mess = receive().replace("\0", "");
				send("BENVENUTO\n",false);
				System.out.println( "-------- LOOP CLIENT FIFO RICEVUTO = "+mess);
				//connected = false;
			
			}
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} */
	}
	public String getData(){
		return data.toString();
	}
	public void start(){
		Log.i(_TAG, "------------DENTRO START");
		//manageADDI.suspendALL();
		String line = null;
		for(;;){
			try {
				//input = new BufferedReader(new InputStreamReader(new FileInputStream(req)));
				//output = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(res)));
				input = new BufferedReader(new FileReader(new File(req)));
				output = new BufferedWriter(new FileWriter(new File(res)));
				for(;;){
					Log.i(_TAG, "============== SERVER IN ASCOLTO ==================");
					line = input.readLine();
					if(line != null)
						data.append(line);
					else
						break;
				}
				
				handleCommand(data.toString());
				data.setLength(0);
				//input.close();
			//	output.close();
			} catch (FileNotFoundException e) {
				System.out.println("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				System.out.println("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
			}
		}
	}
	public void handleCommand(String xmlInput)
	{
		Log.i(_TAG, "-------- CLIENT FIFO RICEVUTO = "+xmlInput);
//		sendFullTransmission("", "Command not found on Mercury server");
//		return;
		
		//s.init();
		
		
		//List<CommandWrapper> tmp = Commands.getCommands();
		//Create an array of commands from xml request received
		List<RequestWrapper> parsedCommands = new XML(xmlInput).getCommands();
		
		//Command has been found on server
		boolean found = false;
	
		//Iterate through received commands
		for (int i = 0; i < parsedCommands.size(); i++)
		{
			//Look for server command and execute
			for (CommandWrapper command : Commands.commandList)
			{
				if (parsedCommands.get(i).section.toUpperCase().equals(command.section.toUpperCase()) && parsedCommands.get(i).function.toUpperCase().equals(command.function.toUpperCase()))
				{
					found = true;
					Object[] args = new Object[2];
					//if(parsedCommands.get(i).argsArray != null)
					args[0] = (Object) parsedCommands.get(i).argsArray;
					args[1] = (Object)this;
					//command.executor.execute(parsedCommands.get(i).argsArray, this);
					try {
						command.meth.invoke(null,(Object)args);
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
					break;
				}
	        }
		}
		
		Log.i(_TAG, "FINE DI HANDLE");
		//Default case if command not found
		if (!found){
			Log.i(_TAG, "----- COMMAND NOT FOUND");
			sendFullTransmission("", "Command not found on Mercury server");
		}
		
  }
	public boolean send(String data, boolean isbase64){
		//Log.i(_TAG, "inivio data "+data+" size = "+data.length());
		
//		byte[] arr = data.getBytes();
		try
		{
			
			//Write to socket base64 encoded with a newline on end
			if (isbase64){
				//output.write(Base64.encode(arr, Base64.DEFAULT), 0, arr.length);
				output.write(new String(Base64.encode(data.getBytes(), Base64.DEFAULT))+"\n");
			}
			else{
			//Or not
				output.write(data, 0, data.length());
			}
			//output.flush();		
			return true;
		}
		catch (Exception e)
		{
			Log.i(_TAG, "EXCEPTION SEEND");
			e.printStackTrace();
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
				output.flush();
				output.close();
				input.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			/**
			try {
				input.close();
				output.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} */
			
		}
		//Send a full transmission without worrying about structure
		//Should only be used for small responses < 50KB
		public void sendFullTransmission(String response, String error)
		{
			/*
			
			Sends the following structure:
			
			<transmission>
				<response>
					<data>response</data>
					<error>error</error>
				</response>
			</transmission>
			
			*/
			
			startTransmission();
			startResponse();
			startData();
			if(response.length() > 0)
				send(response, true);
			endData();
			
			if (error == null)
				error("Null error given");
			else
				if (error.length() > 0)
					error(error);
				else
					noError();
			
			endResponse();
			endTransmission();
		}
}
