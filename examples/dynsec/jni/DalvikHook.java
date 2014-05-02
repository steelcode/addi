package org.tesi.fakecontext;

public class DalvikHook {
	protected final static String _TAG ="Hooks";
	protected final static String _ETAG = "HooksErrors";

	protected String _clname = null;
	protected String _clnamep = null;
	protected String _method_name= null;
	protected String _method_sig = null ;
	protected String _dexpath = null;
	protected String _hashvalue = null ;
	protected String _dex_method = null;
	protected String _dex_class = null;
	public String get_dex_class() {
		return _dex_class;
	}

	public void set_dex_class(String _dex_class) {
		this._dex_class = _dex_class;
	}

	protected int ns; //args+ ?static
	protected int dump;
	protected int debugme;
	protected int _loaded;
	protected int _skip; 
	
	public int isSkip() {
		return _skip;
	}

	public void setSkip(int skip) {
		this._skip = skip;
	}

	private int _myargs;
	
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


	
	public DalvikHook(){}
	public DalvikHook(String clname, String method_name, String method_sig, String dex_method, String dex_class,int num_args, int mya, int skip){
		this._clname = clname;
		this._method_name = method_name;
		this._method_sig = method_sig;
		this._dex_method = dex_method;
		this._dex_class = dex_class;
		this.ns = num_args;
		this._hashvalue = method_name+method_sig;
		this._loaded = 0;
		this._myargs = mya;
		this._skip = skip;
	}
	public static boolean checkContext(){
		if(AppContextConfig.haveContext())
			return true;
		else return false;
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
	
/**
 * struct dalvik_hook_t
{
	char clname[256];
	char clnamep[256];
	char method_name[256];
	char method_sig[256];

	Method *method;
	int sm; // static method

	// original values, saved before patching
	int iss;
	int rss;
	int oss;	
	int access_flags;
	void *insns; // dalvik code

	// native values
	int n_iss; // == n_rss
	int n_rss; // num argument (+ 1, if non-static method) 
	int n_oss; // 0
	void *native_func;
	
	int af; // access flags modifier
	
	int resolvm;

	// for the call
	jclass cls;
	jmethodID mid;

	// debug stuff
	int dump;      // call dvmDumpClass() while patching
	int debug_me;  // print debug while operating on this method
};
*/
	

}
