package org.tesi.fakecontext;




import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;


import android.app.Application;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;



public class LoggerWrapper extends Thread{
	public Application a;
	private Intent mye;
	private Thread myt;
	public boolean started = false;
	public boolean haveContext = false;
	public boolean flag = false;
	static final String ACTION_W00T = "org.tesi.fakecontext.TIMETOW00T";
	private BlockingQueue<Object> q = null;
	
	
	private static int counter = 0;
	
	public native String  stringFromJNI();
	public native void myhook();
	
	


	public LoggerWrapper(){
		System.out.println("----------------CONSTRUCTOR -----------------------");	
		q = new LinkedBlockingQueue<Object>();
		myt = new Thread(this);		
		myt.start();
		System.out.println("---------------------------------------");		
	}

	public void start_magic(int pid, String time){		
		System.out.println("----------- START MAGIC PID: "+pid+time+" -------------");
		if(!flag){

		/**
		d.set_clname("Landroid/app/Activity;");
		d.set_method_name("startActivity");
		d.set_method_sig("(Landroid/content/Intent;)V");
		d.set_dexpath("/data/local/tmp/classes.dex");
		*/
		
		flag = true;
		}
		//System.out.println("-------------"+stringFromJNI()+"----------");
		Intent sendIntent = new Intent();
		sendIntent.setAction(ACTION_W00T);
		sendIntent.putExtra(Intent.EXTRA_TEXT, "w00tw00t");
		if(!haveContext){
			//a = AppContextConfig.getcon();
			haveContext = true;
		}
		System.out.println("!!! application name: !!!"+a.toString());
		System.out.println("!!! app package name: !!!"+a.getPackageName());
		System.out.println("!!! INTENT DEBUGGER: !!!"+ sendIntent.toString());
		a.sendBroadcast(sendIntent);
		
		System.out.println("INTENT INVIATO!!");
		try {
			q.put(sendIntent);
			System.out.println("INVIATO OGGETTO NELLA CODA!!");
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			System.out.println("START MAGIC QUEUE EXCEPTION" + e.toString());
			e.printStackTrace();
		}
		System.out.println("----------------------------------------");
	}
	
	public void wait_message(){
		try{
			while(true){
				mye = (Intent) q.take();
				System.out.println("----------------------------------------");
				System.out.println("RICEVUTO OGGETTO DALLA CODA!!");
				System.out.println("----------------------------------------");
			}
		} catch(Exception e){
			System.out.println("WAIT MESSAGE EXCEPTION" + e.toString());
		}
	}
	
	public void run(){
		while(true){
			System.out.println("----------- THREAD PARTITO -------------");
			started=true;
			try {
				mye = (Intent) q.take();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				System.out.println("WAIT MESSAGE EXCEPTION" + e.toString());
			}
			System.out.println("RICEVUTO OGGETTO DALLA CODA!!");			
			System.out.println("----------------------------------------");
			System.out.println("RIENTRATO DAL WAIT");
			
			/**
			mye.setAction(ACTION_W00T);
			mye.putExtra(Intent.EXTRA_TEXT, "w00tw00t");
			a.sendBroadcast(mye);
			*/
			
			if(!haveContext){
				//a = AppContextConfig.getcon();
				haveContext = true;
			}		
			PackageManager m = a.getPackageManager();
			String s = a.getPackageName();
			try {
				PackageInfo p = m.getPackageInfo(s, 0);
				System.out.println("app info: "+p.toString());
			} catch (NameNotFoundException e1) {
				// TODO Auto-generated catch block
				System.out.println("EXCEPTION");
				e1.printStackTrace();
			}

			System.out.println("----------------------------------------");
		}
		
	}
	
}
