package org.sid.addi.core;

public class HookT {
	public enum HookType{
		NORMAL_HOOK(0), 
		TRUE_HOOK(1),
		FALSE_HOOK(2),
		NOX_HOOK(3),
		AFTER_HOOK(4),
		UNUSED_HOOK(5);
		private final int value;
		private HookType(final int i){
			value=i;
		}
		public int getValue(){
			return value;
		}
	}
}
