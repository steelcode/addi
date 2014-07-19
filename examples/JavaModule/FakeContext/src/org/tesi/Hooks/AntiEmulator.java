/**
 * @author bellone
 *
 */
package org.tesi.Hooks;

import java.math.BigInteger;
import java.security.SecureRandom;


/**
 * @author bellone
 *
 */

class Property {
	public String name;
	public String seek_value;
	
	public Property(String name, String seek_value) {
		this.name = name;
		this.seek_value = seek_value;
	}
}

public class AntiEmulator {
	private static SecureRandom random = new SecureRandom();
	private static String[] blackFiles = {
		"/dev/socket/qemud",
		"/dev/qemu_pipe",
		"/system/lib/libc_malloc_debug_qemu.so",
		"/sys/qemu_trace", 
		"/system/bin/qemu-props",
		
	};
	private static Property[] known_props = {
		new Property( "init.svc.qemud", null ),
		new Property( "init.svc.qemu-props", null ),
		new Property( "qemu.hw.mainkeys", null ),
		new Property( "qemu.sf.fake_camera", null ),
		new Property( "qemu.sf.lcd_density", null ),
		new Property( "ro.bootloader", "unknown" ),
		new Property( "ro.bootmode", "unknown" ),
		new Property( "ro.hardware", "goldfish" ),
		new Property( "ro.kernel.android.qemud", null ),
		new Property( "ro.kernel.qemu.gles", null ),
		new Property( "ro.kernel.qemu", "1" ),
		new Property( "ro.product.device", "generic" ),
		new Property( "ro.product.model", "sdk" ),
		new Property( "ro.product.name", "sdk" ),
		 // Need to double check that an "empty" string ("") returns null
		new Property( "ro.serialno", null)
	};
	
	public static String randomDeviceId(){
		//return new BigInteger(130, random).toString(32);
		return "111111111111111";
	}
}
