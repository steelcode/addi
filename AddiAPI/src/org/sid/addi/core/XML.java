package org.sid.addi.core;

import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import android.util.Base64;

public class XML {
private List<RequestWrapper> commands;
private List<ConfigWrapper> config;
	
	//Assign toClient to connected client
	XML(String xmlInput)
	{
		commands = parseXML(xmlInput);
	}
	XML(String xmlInput, int i){
		config = parseConfigXML(xmlInput);
	}
	public static void writeHookXML(String path){
		
	}
	//Parse an XML string and create Document
	private Document getXMLdocument(String xml)
	{
	
		Document doc = null;
	
		DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
		try
		{
			DocumentBuilder db = dbf.newDocumentBuilder();
	
			InputSource is = new InputSource();
		    is.setCharacterStream(new StringReader(xml));
		    doc = db.parse(is);
		} catch (ParserConfigurationException e) {
					System.out.println("XML parse error: " + e.getMessage());
			return null;
		} catch (SAXException e) {
			System.out.println("Wrong XML file structure: " + e.getMessage());
	        return null;
		} catch (IOException e) {
			System.out.println("I/O exeption: " + e.getMessage());
					return null;
		}
	
		return doc;
	
	}
	/*
	   #<?xml version="1.0" ?>
	    #<transmission>
	    #    <config>
	    #        <section>module</section>
	    #		 <type>module</type>
	    #		 <function>module</function>
	    #    </config>
	    #</transmission>

	    # section = type of config, module, variable, ...
	    # function = function to execute
	    # args_dict = a dict object that defines the configs
	    # None is sent for args_dict if there are no var config
	    # Return a structured XML command
	    **/
//Formulate XML into separate commands
private List<ConfigWrapper> parseConfigXML(String xmlInput)
{

	//The variable to put all commands into
	List<ConfigWrapper> configList = new ArrayList<ConfigWrapper>();
	
	//Parse XML from input
	Document document = getXMLdocument(xmlInput);
	
	//Get root element - should always be "transmission"
	Element rootElement = document.getDocumentElement();
	
	//Get all commands within transmission
	NodeList configs = rootElement.getElementsByTagName("config");
	
	//Iterate through commands and perform them
	for (int i = 0; i < configs.getLength(); i++)
	{
		
		ConfigWrapper conf = new ConfigWrapper();
		
		//Get current command
		Node command = configs.item(i);
		
		//Get all nodes of command	
		NodeList nodes = command.getChildNodes();
		
		//Split nodes into their respective fields
		conf.section = nodes.item(0).getTextContent();
		conf.type = nodes.item(1).getTextContent();
		conf.function = nodes.item(2).getTextContent();
		System.out.println("Config debug, section = "+conf.section+" type = "+conf.type+" function = "+conf.function);
		//Node arguments = nodes.item(2);
		//Arguments can be of any type, binary or text - so a byte array will suffice
		//Arguments are also base64 decoded and placed into this list
		/*
		cmd.argsArray = new ArrayList<ArgumentWrapper>();
		//Check if any arguments came with command
	    if (arguments.hasChildNodes())
	    {
	    	//Iterate through arguments and place them in an array
	    	NodeList argumentList = arguments.getChildNodes();
	    	for (int j = 0; j < argumentList.getLength(); j++)
	    	{
	    		ArgumentWrapper tempArg = new ArgumentWrapper();
	    		tempArg.type = argumentList.item(j).getNodeName();
	    		tempArg.value = argumentList.item(j).getTextContent().getBytes();//Base64.decode(argumentList.item(j).getTextContent(), Base64.DEFAULT);
	    		cmd.argsArray.add(tempArg); 
	    	}
	    }

		*/
	    //Add each command to the list of commands returned
		configList.add(conf);
	}
	
	return configList;

}
	/*
	   #<?xml version="1.0" ?>
	    #<transmission>
	    #    <command>
	    #        <section>provider</section>
	    #        <function>query</function>
	    #        <arguments>
	    #            <selectionargs>ZWlzaA==</selectionargs>
	    #            <selection>dzAwdHk=</selection>
	    #            <projection>bG9sb2xsb2xvbG9sb2wgbG9sb2xvbG9s</projection>
	    #            <URI>Y29udGVudDovL3Bldw==</URI>
	    #            <sortorder>YXNj</sortorder>
	    #        </arguments>
	    #    </command>
	    #</transmission>

	    # section = section of program where command was executed from
	    # function = function to execute
	    # args_dict = a dict object that defines the arguments that come with the command
	    # None is sent for args_dict if there are no arguments
	    # Return a structured XML command
	    **/
//Formulate XML into separate commands
private List<RequestWrapper> parseXML(String xmlInput)
{

	//The variable to put all commands into
	List<RequestWrapper> cmdList = new ArrayList<RequestWrapper>();
	
	//Parse XML from input
	Document document = getXMLdocument(xmlInput);
	
	//Get root element - should always be "transmission"
	Element rootElement = document.getDocumentElement();
	
	//Get all commands within transmission
	NodeList commands = rootElement.getElementsByTagName("command");
	
	//Iterate through commands and perform them
	for (int i = 0; i < commands.getLength(); i++)
	{
		
		RequestWrapper cmd = new RequestWrapper();
		
		//Get current command
		Node command = commands.item(i);
		
		//Get all nodes of command
		NodeList nodes = command.getChildNodes();
		
		//Split nodes into their respective fields
		cmd.section = nodes.item(0).getTextContent();
		cmd.function = nodes.item(1).getTextContent();
		Node arguments = nodes.item(2);
		//Arguments can be of any type, binary or text - so a byte array will suffice
		//Arguments are also base64 decoded and placed into this list
		cmd.argsArray = new ArrayList<ArgumentWrapper>();
		//Check if any arguments came with command
	    if (arguments.hasChildNodes())
	    {
	    	//Iterate through arguments and place them in an array
	    	NodeList argumentList = arguments.getChildNodes();
	    	for (int j = 0; j < argumentList.getLength(); j++)
	    	{
	    		ArgumentWrapper tempArg = new ArgumentWrapper();
	    		tempArg.type = argumentList.item(j).getNodeName();
	    		tempArg.value = Base64.decode(argumentList.item(j).getTextContent(), Base64.DEFAULT);
	    		cmd.argsArray.add(tempArg); 
	    	}
	    }
	    //Add each command to the list of commands returned
	    cmdList.add(cmd);

	}
	
	return cmdList;

}

public List<RequestWrapper> getCommands()
{
	return commands;
}
public List<ConfigWrapper> getConfig()
{
	return config;
}
}
