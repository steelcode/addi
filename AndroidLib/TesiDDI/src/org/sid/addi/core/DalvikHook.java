/*******************************************************************************
 * Advanced Dalvik Dynamic Instrumentation Android Library
 * 
 * (c) 2014, 
 ******************************************************************************/
package org.sid.addi.core;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Set;

import org.sid.addi.core.HookT.HookType;


import android.os.Bundle;




public abstract class DalvikHook extends StringHelper{

	protected String _clname = null;
	protected String _clnamep = null;
	protected String _method_name= null;
	protected String _method_sig = null ;
	protected String _dexpath = null;
	protected String _hashvalue = null ;
	protected String _dex_method = null;
	protected String _dex_class = null;
	protected HookType type;
	protected int ns; //args+ ?static
	protected int _myargs;
	protected int dump;
	protected int debugme;
	protected int _error;
	protected int _loaded;
	protected int _skip; 
	protected Object thiz = null;
	protected DEXHook _dexhook;
	protected DalvikHook _mythiz = null;
	
	public int getType(){
		return this.type.getValue();
	}
	public int getNs() {
		return ns;
	}
	public void setError(int value) {
		this._error = value;
	}
	public int get_myargs() {
		return _myargs;
	}
	public void set_myargs(int _myargs) {
		this._myargs = _myargs;
	}	
	public void setThiz(Object t){
		this.thiz = t;
	}
	public Object getThiz(){
		return this.thiz;
	}
	public DEXHook getDexFunc(){
		return _dexhook;
	}	
	public String get_dex_class() {
		return _dex_class;
	}
	public void set_dex_class(String _dex_class) {
		this._dex_class = _dex_class;
	}
	public int isSkip() {
		return _skip;
	}
	public void setSkip(int skip) {
		this._skip = skip;
	}	
	public String get_hashvalue() {
		return _hashvalue;
	}
	public void set_hashvalue(String _hashvalue) {
		this._hashvalue = _hashvalue;
	}
	public String get_dexpath() {
		return _dexpath;
	}
	public void set_dexpath(String _dexpath) {
		this._dexpath = _dexpath;
	}
/**
	private String removeFirstChar(String s){
		return s.substring(1);
	}
*/
	public DalvikHook(){}
	public DalvikHook(String clname, String method_name, String method_sig, String dex_method, String dex_class, DEXHook ih,HookType t){
		this._clname = clname;
		this._method_name = method_name;
		this._method_sig = method_sig;
		this._dex_method = dex_method;
		this._dex_class = dex_class;
		this._dexhook = ih;
		this._hashvalue = clname+method_name+method_sig;
		this._loaded = 0;
		this.type = t;
		this._mythiz = this;		
	}
	public void initFunc(){
		_dexhook.init(this);		
	}
	private void printArrayObj(Object[] o){
		for(Object obj : o){
			if(obj != null){
				System.out.println("OBJ NON NULLO: "+obj.getClass().toString());
				System.out.println(" Oggetto : " + obj.toString());
			
				System.out.println("Obj:  "+obj.getClass().toString());
				if(obj instanceof android.os.Bundle ){
					Bundle b = (Bundle)obj;
					if(b==null) continue;
					System.out.println("TRVATO UN BUDLE, CON VALORI: "+b.size());
					Set<String> s = b.keySet();				
					if(s.size()>0){
			            for(String item : s){
			            	System.out.println("il bundle contiene chiave: "+item+"valore: ");
			                Object o2 = b.get(item);
			                if(o2 != null)
			                	System.out.println(o2.toString());
			            }
					}
				}
			}
		}
	}
	public void myexecute(Object[] args){
		try {
			//System.out.println("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC");
			//System.out.println("DUMP: dex_method = "+get_dex_method()+" DEXCLASS = " + get_dex_class()+" CLSNAME = "+get_clname());
			//System.out.println("DEBUG DEXHOOK,  "+_dexhook._methodName);
			Method m = _dexhook.getClass().getDeclaredMethod(get_dex_method(), Object[].class);
			//System.out.println("RICEVUTO NUM ARGS = "+args.length + args.toString());
			//printArrayObj(args);
			//System.out.println("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC" + _clname + " paranoica " + m.getName());
			Object[] myargs = new Object[]{args};
			//Object[] myargs = new Object[args.length];
			//myargs[0] = args;
			//int i =0;
			//for(Object oo : args){
			//	myargs[i] =  oo;
			//	i++;
			//}
			//System.out.println("AAAAAAAAAAAAAAA STAMPO COPIA ARRAY");
			//printArrayObj(myargs);
			//System.out.println("vediamo un po class: "+myargs.getClass().toString()+" myargs: "+myargs.length + myargs.toString());
			//System.out.println("INVOKO IL METODO SENZA VALORE DI RITORNO!!!");
			m.invoke(_dexhook, myargs);
		} catch (IllegalAccessException e) {
			System.out.println("PUTTANA CENERENTOLA");
			printArrayObj(args);
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalArgumentException e) {
			System.out.println("EXCEPTION ARGUMENTS!!!!");
			System.out.println(args.getClass().toString());
			System.out.println("DUMP: dex_method = "+get_dex_method()+" DEXLASS = " + get_dex_class()+" CLSNAME = "+get_clname());
			System.out.println("DEBUG DEXHOOK,  "+_dexhook._methodName);
			System.out.println("RICEVUTO NUM ARGS = "+args.length + args.toString());
			Object[] myargs2 = new Object[args.length];
			int i =0;
			for (Object o : args){
				if (o == null){
					System.out.println("OBJ NULLO");
					continue;
				}
				System.out.println("OBJ NON NULLO: "+o.getClass().toString());
				//if(o.getClass().equals(int.class))
						//System.out.println("ELEMENTO PRIMITIVO!!!!");
				myargs2[i] = o;
				i++;
				System.out.println(" Oggetto : " + o.toString());
			}
			
			System.out.println("vediamo un po class: "+myargs2.getClass().toString()+" myargs: "+myargs2.length + myargs2.toString());
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (InvocationTargetException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (NoSuchMethodException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
	}
	public void myexecute(){
		try {
			System.out.println("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC");
			Method m = _dexhook.getClass().getDeclaredMethod(get_dex_method());
			System.out.println("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC" + _clname + " paranoica " + m.getName());
			m.invoke(_dexhook);
		} catch (IllegalAccessException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (NoSuchMethodException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalArgumentException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (InvocationTargetException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	public String get_dex_method() {
		return _dex_method;
	}

	public void set_dex_method(String _dex_method) {
		this._dex_method = _dex_method;
	}

	public String get_clname() {
		return _clname;
	}

	public void set_clname(String _clname) {
		this._clname = _clname;
	}

	public String get_clnamep() {
		return _clnamep;
	}

	public void set_clnamep(String _clnamep) {
		this._clnamep = _clnamep;
	}

	public String get_method_name() {
		return _method_name;
	}
	public void set_method_name(String _method_name) {
		this._method_name = _method_name;
	}
	public String get_method_sig() {
		return _method_sig;
	}
	public void set_method_sig(String _method_sig) {
		this._method_sig = _method_sig;
	}
	public int getDump() {
		return dump;
	}
	public void setDump(int dump) {
		this.dump = dump;
	}
	public int getDebugme() {
		return debugme;
	}
	public void setDebugme(int debugme) {
		this.debugme = debugme;
	}
	public void toXML(){
		StringBuilder xml = null;
		Class<?> c = DalvikHook.class;
		List<Field> currentClassFields = new ArrayList<Field>(Arrays.asList(c.getDeclaredFields()));
		for (Field f : currentClassFields){
			System.out.println(f.getName());
		}
		//return xml.toString();
	}
	public boolean isHooked(){
		if(manageADDI.searchHookHash(_hashvalue))
			return true;
		else 
			return false;
	}
	public void myunhook(){
		System.out.println("--------------  JAVA UNHOOK "+_hashvalue);
		manageADDI.unhookWrap(_hashvalue);
	}
	public void myrehook(){
		manageADDI.rehookWrap(_hashvalue);
	}
}
