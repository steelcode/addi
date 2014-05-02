package org.sid.addi.core;

import java.util.ArrayList;
import java.util.List;

import org.sid.addi.core.CommandWrapper;
import android.os.Debug;

public class Commands {
	public static Class<?> c = Commands.class;
	
	public Commands(){
		
	}

	public static void pong(Object... args){
		List<ArgumentWrapper> argsArray = (List<ArgumentWrapper>)args[0];
		Session currentSession = (Session)args[1];
		// TODO Auto-generated method stub
		currentSession.sendFullTransmission("pong", "");
			
	}
	public static void suspendAll(Object... args){
		List<ArgumentWrapper> argsArray = (List<ArgumentWrapper>)args[0];
		Session currentSession = (Session)args[1];
		manageADDI.suspendALL();
		// TODO Auto-generated method stub
		currentSession.sendFullTransmission("suspendok", "");
	}
	public static void resumeAll(Object... args){
		List<ArgumentWrapper> argsArray = (List<ArgumentWrapper>)args[0];
		Session currentSession = (Session)args[1];
		// TODO Auto-generated method stub
		manageADDI.resumeALL();
		currentSession.sendFullTransmission("resumeok", "");
			
	}
	public static void startDump(Object... args){	
		List<ArgumentWrapper> argsArray = (List<ArgumentWrapper>)args[0];
		Session currentSession = (Session)args[1];
		Debug.startMethodTracing("whats", 100000000);
		currentSession.sendFullTransmission("startDebugOk", "");
	}
	public static void stopDump(Object... args){	
		List<ArgumentWrapper> argsArray = (List<ArgumentWrapper>)args[0];
		Session currentSession = (Session)args[1];
		Debug.stopMethodTracing();
		currentSession.sendFullTransmission("stopDebugOk", "");
	}
	public static void unhook(Object... args){	
		List<ArgumentWrapper> argsArray = (List<ArgumentWrapper>)args[0];
		Session currentSession = (Session)args[1];
		String hook = Common.getParamString(argsArray, "hook");
		System.out.println("-------- HO RICEVUTO HOOK "+hook);
		manageADDI.unhook(hook);
		currentSession.sendFullTransmission("unhookOk", "");
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
	 try {
		commandList.add(new CommandWrapper("core", "ping", c.getMethod("pong", Object[].class)));
		commandList.add(new CommandWrapper("core", "resume", c.getMethod("resumeAll", Object[].class)));
		commandList.add(new CommandWrapper("core", "suspend", c.getMethod("suspendAll", Object[].class)));
		commandList.add(new CommandWrapper("core", "startDump", c.getMethod("startDump", Object[].class)));
		commandList.add(new CommandWrapper("core", "stopDump", c.getMethod("stopDump", Object[].class)));
		commandList.add(new CommandWrapper("core", "unhook", c.getMethod("unhook", Object[].class)));
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
