/*****************************************************************************
* Advanced Dalvik Dynamic Instrumentation Android Library
* 
* (c) 2014, 
******************************************************************************/

package org.sid.addi.core;

import java.lang.reflect.Member;
import java.util.List;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

/**
* @author sid
*
*/
public abstract class manageADDI {	
	static {
		System.load("/data/local/tmp/libdynsec.so");
		System.load("/data/local/tmp/libmstring.so");	
	}	
	public static List<DalvikHook> _myhooks = null;
	
	public static void setmyHooks(List<DalvikHook> d){
		_myhooks =  d;
	}
	
	/**
	 * JNI wrappers to  dalvik's function
	 * 
	 */
	
	/* Inserisce nella lista un DalvikHook*/
	public static native int createStruct(DalvikHook d);
	//public static native void createPTY();
	/* sospende tutti i thread tranne il chiamante*/
	public static native void suspendALL();
	/* risveglia tutti i thread */
	public static native void resumeALL();
	/* Ripristina la chiamata originale di un DalvikHook , ma non lo rimuove dalla struttura dinamica (lista)*/
	public static native void unhook(String hook);
	/* rimpiazza l'hook nativo di un DalvikHook  presente nella struttura dinamica (lista) */
	public static native void rehook(String hook);
	/* Rimuove dalla struttura dinamica (lista) un DalvikHook */
	public static native void delhook(String hook);
	
	public static native void startNewThread(String dexpath, String clsname);
	
	public static native int defineClass(int i ,String c);
	
	public static native int loadDex(String d);
	
	public static native void dumpJavaClass(String cls,String sname);
	
	//public static native int hookMethodNative(Member method, Class<?> declaringClass, int slot, Class<?>[] parameterTypes, Class<?> returnType);
	
	public static native void diosoloW();
	
	
	
	public static void unhookWrap(String s){
		System.out.println("----------- UNHOOK di "+s);
		//Object[] args = new Object[]{ s};
		//Commands.execute(args);
		unhook("Landroid/app/Activity;startActivity(Landroid/content/Intent;)V");
	}
	public static void rehookWrap(String s){
		System.out.println("----------- REHOOK di "+s);
		//suspendALL();
		rehook(s);
		//resumeALL();
	}
	
	public static void addHook(DalvikHook d){
		_myhooks.add(d);
	}
	
	public static void removeHook(String hash){
		for(final DalvikHook elem : _myhooks){
			if(elem.get_hashvalue() == hash)
				_myhooks.remove(elem);
		}
	}
	public static boolean searchHookHash(String hash){
		System.out.println("CERCO: "+hash);
		boolean flag = false;
		for(final DalvikHook elem : _myhooks){
			if(elem.get_hashvalue() == hash)
				flag = true;
		}
		return flag;
	}
	public static void writeXMLHooks(String path){
		try {
			DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
			DocumentBuilder docBuilder = docFactory.newDocumentBuilder();
			Document doc = docBuilder.newDocument();
			Element rootElement = doc.createElement("addi");
			doc.appendChild(rootElement);
			for (final DalvikHook elem : _myhooks){
				Element Hook  = doc.createElement("hook");
				rootElement.appendChild(Hook);
				Element clsname = doc.createElement("clsname");
				clsname.appendChild(doc.createTextNode(elem.get_clname()));
				Hook.appendChild(clsname);
				
				Element mname = doc.createElement("mname");
				mname.appendChild(doc.createTextNode(elem.get_method_name()));
				Hook.appendChild(mname);
				
				Element dexclass = doc.createElement("dexclass");
				dexclass.appendChild(doc.createTextNode(elem.get_dex_class()));
				Hook.appendChild(dexclass);
				
				Element hashsig = doc.createElement("hash");
				hashsig.appendChild(doc.createTextNode(elem.get_hashvalue()));
				Hook.appendChild(hashsig);
			}	
			TransformerFactory transformerFactory = TransformerFactory.newInstance();
			Transformer transformer = transformerFactory.newTransformer();
			DOMSource source = new DOMSource(doc);
			StreamResult result = new StreamResult(System.out);
			transformer.transform(source, result);
		} catch (ParserConfigurationException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (TransformerConfigurationException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (TransformerException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
	}
	
	public static int _init(DalvikHook d){
		int i = createStruct(d); //1 is error, 0 ok
		return i; 
	}
	
}
