package org.sid.addi.core;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Set;

import org.sid.addi.core.HookT.HookType;

import android.os.Bundle;


public abstract class DalvikHook {
	protected String _clname = null;
	protected String _clnamep = null;
	protected String _method_name= null;
	protected String _method_sig = null ;
	protected String _dexpath = null;
	protected String _hashvalue = null ;
	protected String _dex_method = null;
	protected String _dex_class = null;
	protected HookType type;
	protected int _skip;

	

	public int getType(){
		return this.type.getValue();
	}
	public String get_dex_class() {
		return _dex_class;
	}
	public int isSkip() {
		return _skip;
	}
	public String get_hashvalue() {
		return _hashvalue;
	}
	public String get_dexpath() {
		return _dexpath;
	}
	public String get_dex_method() {
		return _dex_method;
	}
	public String get_clname() {
		return _clname;
	}
	public String get_clnamep() {
		return _clnamep;
	}
	public String get_method_name() {
		return _method_name;
	}
	public String get_method_sig() {
		return _method_sig;
	}


	
	public DalvikHook(){}
	private void printArrayObj(Object[] o){
		if(o == null) return;
		for(Object obj : o){
			if(obj != null){
				System.out.println("OBJ NON NULLO: "+obj.getClass().toString());
				System.out.println(" Oggetto : " + obj.toString());
				if(obj instanceof android.os.Bundle ){
					Bundle b = (Bundle)obj;
					if(b==null) break;
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
			else{
				System.out.println("OBJ NULLO!!!");
			}
		}
	}
	public DalvikHook(String clname, String method_name, String method_sig, String dex_method, String dex_class, HookType t){
		this._clname = clname;
		this._method_name = method_name;
		this._method_sig = method_sig;
		this._dex_method = dex_method;
		this._dex_class = dex_class;
		this._hashvalue = clname+method_name+method_sig;
		this.type = t;		
	}
	
	public void myexecute(Object[] args){
		System.out.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		System.out.println("chiamo metodo: "+this._dex_method+" con num args: "+args.length);
		try {
			Class c = Class.forName(this._dex_class);
			Method m = c.getMethod(this._dex_method, Object[].class);
			Object[] myargs = new Object[]{args};
			m.invoke(null, myargs);
		} catch (ClassNotFoundException | NoSuchMethodException | IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
			// TODO Auto-generated catch block
			//e.printStackTrace();
			;
		}
		//printArrayObj(args);
		return;

	}
}
