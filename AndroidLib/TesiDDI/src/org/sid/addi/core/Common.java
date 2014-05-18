/*******************************************************************************
 * Advanced Dalvik Dynamic Instrumentation Android Library
 * 
 * (c) 2014, 
 ******************************************************************************/
package org.sid.addi.core;

import java.io.Serializable;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.util.Log;



//A class to wrap arguments in
class ArgumentWrapper
{
	public String type;
	public byte[] value;
}

//An interface to use in CommandWrapper to better define Commands
//interface Executor { public void execute(List<ArgumentWrapper> argsArray, Session currentSession);}

//A class to wrap commands and their implementations inside
class CommandWrapper
{
	public String section;
	public String function;
	public Method meth;
	//public Executor executor;
	
	//public CommandWrapper(String inputSection, String inputFunction, Executor inputExecutor)
	public CommandWrapper(String inputSection, String inputFunction, Method inputExecutor)
	{
		section = inputSection;
		function = inputFunction;
		meth = inputExecutor;
		//executor = inputExecutor;
	}
}

//A class to wrap requests that come in
class RequestWrapper
{
	public String section;
	public String function;
	public List<ArgumentWrapper> argsArray;
}


public class Common {
	
	
	//Get parameter from a List<ArgumentWrapper> in byte[] format
	public static byte[] getParam(List<ArgumentWrapper> argWrapper, String type)
	{
		
		for (int i = 0; i < argWrapper.size(); i++)
		{
			if (argWrapper.get(i).type.toUpperCase().equals(type.toUpperCase()))
				return argWrapper.get(i).value;
		}
		
		return null;
	}
	
	//Get parameter from a List<ArgumentWrapper> in String format
	public static String getParamString(List<ArgumentWrapper> argWrapper, String type)
	{
		
		for (int i = 0; i < argWrapper.size(); i++)
		{
			if (argWrapper.get(i).type.toUpperCase().equals(type.toUpperCase()))
				return new String(argWrapper.get(i).value);
		}
		
		return "";
	}
	
	//Get parameter from a List<ArgumentWrapper> in List<String> format
	public static List<String> getParamStringList(List<ArgumentWrapper> argWrapper, String type)
	{
		List<String> returnValues = new ArrayList<String>();
		
		for (int i = 0; i < argWrapper.size(); i++)
		{
			if (argWrapper.get(i).type.toUpperCase().equals(type.toUpperCase()))
				returnValues.add(new String(argWrapper.get(i).value));
		}
		
		return returnValues;
	}

	
	//Convert a List to a contentvalues structure by splitting by =
	public static ContentValues listToContentValues(List<String> values, String type)
	{
		ContentValues contentvalues = new ContentValues();
	    
	    //Place values into contentvalue structure
	    for (int i = 0; i < values.size(); i++)
	    {
	    	String current = values.get(i);
	    	
	    	try
	    	{    	
		    	//Separate the value by = in order to get key:value
		    	Integer indexOfEquals = current.indexOf("=");
		    	String key = current.substring(0, indexOfEquals);
		    	String value = current.substring(indexOfEquals + 1);
		
		    	if (type.toUpperCase().equals("STRING"))
		    		contentvalues.put(key, value);
		    	
		    	if (type.toUpperCase().equals("BOOLEAN"))
		    		contentvalues.put(key, Boolean.valueOf(value));
	
		    	if (type.toUpperCase().equals("INTEGER"))
		    		contentvalues.put(key, new Integer(value));
		    	
		    	if (type.toUpperCase().equals("DOUBLE"))
		    		contentvalues.put(key, new Double(value));
		    	
		    	if (type.toUpperCase().equals("FLOAT"))
		    		contentvalues.put(key, new Float(value));
		    	
		    	if (type.toUpperCase().equals("LONG"))
		    		contentvalues.put(key, new Long(value));
		    	
		    	if (type.toUpperCase().equals("SHORT"))
		    		contentvalues.put(key, new Short(value));
	    	}
	    	catch (Exception e) 
	    	{
	    		Log.e("mercury", "Error with argument " + current);
	    	}
	    	
	    }
	    
	    return contentvalues;
	}
	
	//Get the columns of a content provider
	public static ArrayList<String> getColumns (ContentResolver resolver, String uri, String[] projectionArray)
	{
		//String returnValue = "";
		ArrayList<String> columns = new ArrayList<String>();
		
		try
		{				
	        //Issue query
	        Cursor c = resolver.query(Uri.parse(uri), projectionArray, null, null, null);
	                    		        	
	        //Get all column names and display
	        if (c != null)
	        {
	        	String [] colNames = c.getColumnNames();
	        	
	        	//Close the cursor
	        	c.close();
	        	
	        	//String columnNamesOutput = "";
	        	for (int k = 0; k < colNames.length; k++)
	        		columns.add(colNames[k]);
	        }
		}
		catch (Throwable t) {}
		
		return columns;
		

	}
	

	private static native String native_strings(String path);
	
	public static ArrayList<String> strings(String path) {
		ArrayList<String> lines = new ArrayList<String>();
		
		String nativeList = native_strings(path);
		
		if (nativeList != null) {		
			String[] stringList = nativeList.split("\n");
				
			if (stringList != null) {		
				for (String uri : stringList) {
					lines.add(uri);
				}
			}
		}
		
		return lines;
	}
			
	//Parse a generic intent and add to given intent
	public static Intent parseIntentGeneric(List<ArgumentWrapper> argsArray, Intent intent)
	{		
		Intent localIntent = intent;
		Iterator<ArgumentWrapper> it = argsArray.iterator();
		
		//Iterate through arguments
		while (it.hasNext())
		{
			ArgumentWrapper arg = it.next();
			
			String key = "";
			String value = "";
			
			try
			{
			
				//Try split value into key:value pair
				try
				{
					String[] split = new String(arg.value).split("=");
					key = split[0];
					value = split[1];
				}
				catch (Exception e) {}
				
				//Parse arguments into Intent
				if (arg.type.toUpperCase().equals("ACTION"))
					localIntent.setAction(new String(arg.value));
				
				if (arg.type.toUpperCase().equals("DATA"))
					localIntent.setData(Uri.parse(new String(arg.value)));
					
				if (arg.type.toUpperCase().equals("MIMETYPE"))
					localIntent.setType(new String(arg.value));

				if (arg.type.toUpperCase().equals("CATEGORY"))
					localIntent.addCategory(new String(arg.value));
					
				if (arg.type.toUpperCase().equals("COMPONENT"))
					localIntent.setComponent(new ComponentName(key, value));
					
				if (arg.type.toUpperCase().equals("FLAGS"))
					localIntent.setFlags(Integer.parseInt(new String(arg.value)));
					
				if (arg.type.toUpperCase().equals("EXTRABOOLEAN"))
					localIntent.putExtra(key, Boolean.parseBoolean(value));
					
				if (arg.type.toUpperCase().equals("EXTRABYTE"))
					localIntent.putExtra(key, Byte.parseByte(value));
					
				if (arg.type.toUpperCase().equals("EXTRADOUBLE"))
					localIntent.putExtra(key, Double.parseDouble(value));
					
				if (arg.type.toUpperCase().equals("EXTRAFLOAT"))
					localIntent.putExtra(key, Float.parseFloat(value));
					
				if (arg.type.toUpperCase().equals("EXTRAINTEGER"))
					localIntent.putExtra(key, Integer.parseInt(value));
					
				if (arg.type.toUpperCase().equals("EXTRALONG"))
					localIntent.putExtra(key, Long.parseLong(value));
					
				if (arg.type.toUpperCase().equals("EXTRASERIALIZABLE"))
					localIntent.putExtra(key, Serializable.class.cast(value));
					
				if (arg.type.toUpperCase().equals("EXTRASHORT"))
					localIntent.putExtra(key, Short.parseShort(value));
					
				if (arg.type.toUpperCase().equals("EXTRASTRING"))
					localIntent.putExtra(key, value);
					
			}
			catch (Exception e)
			{
				Log.e("mercury", "Error with argument " + arg.type + "--" + new String(arg.value));
			}
			
			
		}
		
		return localIntent;
	}
	}
