package org.sid.addi.core;

import java.util.ArrayList;
import java.util.List;

import android.os.Debug;
import android.util.Log;

public class Commands {
public Commands(){
		
	}
	
	public static void execute(Object...args){
		System.out.println("+++++++++++++++++++++++++++++ EXECUTE COMMANDS");
		//manageADDI.suspendALL();
		//manageADDI.unhook((String) args[0]);
		System.out.println("+++++++++++++++++++++++++++++ EXECUTE COMMANDS FATTO UNHOOK");
		//manageADDI.resumeALL();
	}

	public static void pong(Object... args){
		List<ArgumentWrapper> argsArray = (List<ArgumentWrapper>)args[0];
		PipeServer currentSession = (PipeServer)args[1];
		// TODO Auto-generated method stub
		currentSession.sendFullTransmission("pong", "");
			
	}
	public static void suspendAll(Object... args){
		List<ArgumentWrapper> argsArray = (List<ArgumentWrapper>)args[0];
		PipeServer currentSession = (PipeServer)args[1];
		//manageADDI.suspendALL();
		
		Log.i("Hooks", "++++++++++++++++++++++++++++++++++++++++=CHIAMATO SUSPENDALL");
		
		// TODO Auto-generated method stub
		currentSession.sendFullTransmission("suspendok", "");
		
		Log.i("Hooks", "++++++++++++++++++++++++++++++++++++++++ INVIATA RISPOSTA SUSPENDALL");
	}
	public static void resumeAll(Object... args){
		List<ArgumentWrapper> argsArray = (List<ArgumentWrapper>)args[0];
		PipeServer currentSession = (PipeServer)args[1];
		// TODO Auto-generated method stub
		//manageADDI.resumeALL();
		
		Log.i("Hooks", "++++++++++++++++++++++++++++++++++++++++=CHIAMATO RESUMEALL");
		
		currentSession.sendFullTransmission("resumeok", "");
		
		Log.i("Hooks", "++++++++++++++++++++++++++++++++++++++++ INVIATA RISPOSTA RESUMEALL");
			
	}
	public static void startDump(Object... args){	
		List<ArgumentWrapper> argsArray = (List<ArgumentWrapper>)args[0];
		PipeServer currentSession = (PipeServer)args[1];
		Debug.startMethodTracing("whats", 100000000);
		currentSession.sendFullTransmission("startDebugOk", "");
	}
	public static void stopDump(Object... args){	
		List<ArgumentWrapper> argsArray = (List<ArgumentWrapper>)args[0];
		PipeServer currentSession = (PipeServer)args[1];
		Debug.stopMethodTracing();
		currentSession.sendFullTransmission("stopDebugOk", "");
	}
	public static void unhook(Object... args){	
		List<ArgumentWrapper> argsArray = (List<ArgumentWrapper>)args[0];
		PipeServer currentSession = (PipeServer)args[1];
		String hook = Common.getParamString(argsArray, "hook");
		System.out.println("-------- HO RICEVUTO HOOK "+hook);
		//manageADDI.unhookWrap(hook);
		currentSession.sendFullTransmission("unhookOk", "");
	}
	public static void newThread(Object... args){
		List<ArgumentWrapper> argsArray = (List<ArgumentWrapper>)args[0];
		PipeServer currentSession = (PipeServer)args[1];
		//String dexp = Common.getParamString(argsArray, "dexpath");
		//String clsn = Common.getParamString(argsArray, "clsname");
		String dexp = "/data/local/blabla";
		String clsn = "ClassNameBLaBla";
		//manageADDI.startNewThread(dexp, clsn);
		currentSession.sendFullTransmission("newThreadOK", "");
	}
	public static void dumpJavaClass(Object... args){
		List<ArgumentWrapper> argsArray = (List<ArgumentWrapper>)args[0];
		PipeServer currentSession = (PipeServer)args[1];
		String clsn = Common.getParamString(argsArray, "clsname");
		String sname = Common.getParamString(argsArray, "fieldname");
		//manageADDI.dumpJavaClass(clsn,sname);
		currentSession.sendFullTransmission("newThreadOK", "");
	}
	/**
static CommandWrapper[] commandList = new CommandWrapper[]{
	new CommandWrapper("core", "ping", new Executor()
	{
		
		@Override
		public void execute(List<ArgumentWrapper> argsArray, Session currentSession)
		{
			// TODO Auto-generated method stub
			currentSession.sendFullTransmission("pong", "");
		}
	})
};

*/
 static List<CommandWrapper> commandList = new ArrayList<CommandWrapper>();
		
 static void init(){
	 Class<?> c = Commands.class;
	 try {
		commandList.add(new CommandWrapper("core", "ping", c.getMethod("pong", Object[].class)));
		commandList.add(new CommandWrapper("core", "resume", c.getMethod("resumeAll", Object[].class)));
		commandList.add(new CommandWrapper("core", "suspend", c.getMethod("suspendAll", Object[].class)));
		commandList.add(new CommandWrapper("core", "startDump", c.getMethod("startDump", Object[].class)));
		commandList.add(new CommandWrapper("core", "stopDump", c.getMethod("stopDump", Object[].class)));
		commandList.add(new CommandWrapper("core", "unhook", c.getMethod("unhook", Object[].class)));
		commandList.add(new CommandWrapper("core", "newThread", c.getMethod("newThread", Object[].class)));
		commandList.add(new CommandWrapper("core", "dumpJavaClass", c.getMethod("dumpJavaClass", Object[].class)));
	} catch (NoSuchMethodException e) {
		// TODO Auto-generated catch block
		e.printStackTrace();
	}
	 /**
				commandList.add(new CommandWrapper("core", "ping", new Executor()
				{
					
					@Override
					public void execute(List<ArgumentWrapper> argsArray, Session currentSession)
					{
						// TODO Auto-generated method stub
						currentSession.sendFullTransmission("pong", "");
					}
				}));
	*/				
}
		
 static  List<CommandWrapper> getCommands(){
			return commandList;
}
}
