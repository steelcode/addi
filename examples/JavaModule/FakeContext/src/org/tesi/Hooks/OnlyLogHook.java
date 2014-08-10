/**
 * @author Valerio Costamagna
 *
 */
package org.tesi.Hooks;

import org.sid.addi.core.DEXHook;


/**
 * @author Valerio Costamagna
 *
 */
public class OnlyLogHook extends DEXHook {

	public void myCompare(Object... args){
		if(_thiz == null){
			System.out.println("MYCOMPARE THIZ NULLO!!!!!");
			return;
		}
		String s1 = (String)args[0];
		System.out.println("COMPARE: "+_thiz+" con: "+s1);
	}
	public void myToString(Object... args){
		//System.out.println("DENTRO MYTOSTRING");
		try{
		if(_thiz == null){
			System.out.println("MYTOSTRING THIZ NULLO!!!!!");
			return;
		}
		String s = _thiz.toString();
		System.out.println("myToString: "+s);
		}  catch(Exception e){
			System.out.println("TO STRING EXCEPTION");
			e.printStackTrace();
		}
		
	}
	public String getFullTraces() {
		StackTraceElement[] ste = Thread.currentThread().getStackTrace();
	    if (ste == null)
	        return "";
	    StringBuilder stringBuilder = new StringBuilder();
	    for (StackTraceElement element : ste)
	        stringBuilder.append(element.toString()).append("\n");
	    return stringBuilder.toString();
	}
	public void cippalippa(Object...args){
		System.out.println("CIPPALIPPA JAVA");
		
	}
	public void myLoadClass(Object...args){
		System.out.println("+-+-+-+-+-+-+-+--+-+-+-+-+-aaaa-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-++-");
		/*
		//String s = getFullTraces();
		//System.out.println(s);
		Object thiz = null;
		if(_thiz != null){
			thiz = _thiz;
			if(thiz instanceof String)
				
				System.out.println("----- DUMP STRING MYLOADCLASS, THIZ:  "+(String)thiz);
			else{	
				System.out.println("----- DUMP MYLOADCLASS, THIZ:  "+thiz.toString());
				//da qui si entra nel sifone
				//System.out.println("----- DUMP MYLOADCLASS, THIZ PARENT:  "+((PathClassLoader)_thiz).getParent().toString());
			}
		}
		else{
			;
			//System.out.println("----- DUMP MYLOADCLASS, THIZ NULL!!!!");
		}
		int i;
		for(i=0;i<args.length;i++){
			if(args[i] == null)
				continue;
			if(args[i] instanceof String)
				System.out.println("-----STRING MYLOADCLASS ARGS["+i+"]:  "+(String)args[i]);
			else
				System.out.println("----- DUMP MYLOADCLASS ARGS["+i+"]:  "+args[i].toString());
		}	
		//String c = (String) args[0];
		//System.out.println("MYLOADCLASSS CERCO CLASSE: "+c);
		//manageADDI.dumpJavaClass(c,"");
		//String c1 = c.replace(".", "/");
		//c1 = "L" + c1 + ";";		
		//System.out.println("CHIAMO SEARCH FAILED CON: "+c1);
		//MyInit.searchFailedHookWithClsName(c1);
		
		*/
		 
	}
	public void dumpArgs(Object...args){
		Object thiz = null;
		if(_thiz != null){
			thiz = _thiz;
			if(thiz instanceof String)
				;
				//System.out.println("----- DUMP ARGS, THIZ:  "+(String)thiz);
			else{
				;
				//System.out.println("----- DUMP ARGS, THIZ:  "+thiz.toString());
				//System.out.println("----- DUMP ARGS, THIZ PARENT:  "+((PathClassLoader)_thiz).getParent().toString());
			}
		}
		else{
			;
			//System.out.println("----- DUMP ARGS, THIZ NULL!!!!");
		}
		int i;
		for(i=0;i<args.length;i++){
			if(args[i] == null)
				continue;
			if(args[i] instanceof String)
				;
				//System.out.println("-----STRING DUMP ARGS["+i+"]:  "+(String)args[i]);
			else
				;
				//System.out.println("----- DUMP ARGS["+i+"]:  "+args[i].toString());
		}
		//PathClassLoader pcl = (PathClassLoader)_thiz;
		//System.out.println("----- FINE CLASSLOADER1: "+pcl.getClass());
		//System.out.println("----- FINE CLASSLOADER2: "+pcl.getClass().getClassLoader());
		//System.out.println("----- FINE SYSTEM CLASSLOADER: "+PathClassLoader.getSystemClassLoader().toString());
		//manageADDI.diosoloW();
		/**
		try {
			ClassLoader mcl = pcl.getClass().getClassLoader();
			Class c = pcl.loadClass("appinventor.ai_garikoitzmartinez.crackme01.Screen1");
			System.out.println("CLASSLOADER PCL = "+c.getClassLoader());
			//perche non trova findloadedclass??
			java.lang.reflect.Method m = pcl.getClass().getDeclaredMethod("findLoadedClass", String.class);
			m.setAccessible(true);
			Object o = m.invoke(mcl, "appinventor.ai_garikoitzmartinez.crackme01.Screen1");
			if(o==null)
			{
				System.out.println("CLASSE NON CARICATA!!!!!");
			}
			else{
				System.out.println("CLASSE CARICATA!!!!!");
			}
				//Class c = pcl.loadClass("appinventor.ai_garikoitzmartinez.crackme01.Screen1");
			//System.out.println("CARICATA CLASSE: "+c);
		} catch (NoSuchMethodException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalAccessException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalArgumentException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (InvocationTargetException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (ClassNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		**/
		
		
	}
}
