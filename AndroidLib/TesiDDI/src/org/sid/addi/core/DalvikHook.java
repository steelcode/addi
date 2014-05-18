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


public abstract class DalvikHook extends StringHelper{


	protected String _clname = null;
	protected String _clnamep = null;
	protected String _method_name= null;
	protected String _method_sig = null ;
	protected String _dexpath = null;
	protected String _hashvalue = null ;
	protected String _dex_method = null;
	protected String _dex_class = null;
	protected int ns; //args+ ?static
	protected int dump;
	protected int debugme;
	protected int _loaded;
	protected int _skip; 
	protected int dexAfter;
	protected int _myargs;
	protected Object thiz = null;
	protected DEXHook _dexhook;
	protected DalvikHook _mythiz = null;
	
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
	
	public int get_myargs() {
		return _myargs;
	}

	public void set_myargs(int _myargs) {
		this._myargs = _myargs;
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
	public DalvikHook(String clname, String method_name, String method_sig, String dex_method, String dex_class, DEXHook ih, int num_args, int mya, int skip){
		this._clname = clname;
		this._method_name = method_name;
		this._method_sig = method_sig;
		this._dex_method = dex_method;
		this._dex_class = dex_class;
		this.ns = num_args;
		this._dexhook = ih;
		//the hash value is: clname+method_name+method_descriptor clname is without L e ;
		//this._hashvalue = removeFirstChar(clname.replace(";", ""))+method_name+method_sig;
		this._hashvalue = clname+method_name+method_sig;
		this._loaded = 0;
		this._myargs = mya;
		this._skip = skip;
		this._mythiz = this;
		
	}
	public void initFunc(){
		_dexhook.init(this);
	}
	public void myexecute(Object... args){
		try {
			System.out.println("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC");
			System.out.println("DUMP: dex_method = "+get_dex_method()+" DEXLASS = " + get_dex_class()+" CLSNAME = "+get_clname());
			Method m = _dexhook.getClass().getDeclaredMethod(get_dex_method(), Object[].class);
			System.out.println("CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC" + _clname + " paranoica " + m.getName());
			Object[] myargs = new Object[]{
					args
			};
			m.invoke(_dexhook, myargs);
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
	public int getNs() {
		return ns;
	}
	public void setNs(int rss) {
		this.ns = rss;
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
