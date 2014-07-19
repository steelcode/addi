/**
 * @author bellone
 *
 */
package org.tesi.Hooks;

import java.util.ArrayList;
import java.util.Set;

import org.sid.addi.core.DEXHook;

import android.os.Bundle;

/**
 * @author bellone
 *
 */
public class BillingHook extends DEXHook{
    // Keys for the responses from InAppBillingService
    public static final String RESPONSE_CODE = "RESPONSE_CODE";
    public static final String RESPONSE_GET_SKU_DETAILS_LIST = "DETAILS_LIST";
    public static final String RESPONSE_BUY_INTENT = "BUY_INTENT";
    public static final String RESPONSE_INAPP_PURCHASE_DATA = "INAPP_PURCHASE_DATA";
    public static final String RESPONSE_INAPP_SIGNATURE = "INAPP_DATA_SIGNATURE";
    public static final String RESPONSE_INAPP_ITEM_LIST = "INAPP_PURCHASE_ITEM_LIST";
    public static final String RESPONSE_INAPP_PURCHASE_DATA_LIST = "INAPP_PURCHASE_DATA_LIST";
    public static final String RESPONSE_INAPP_SIGNATURE_LIST = "INAPP_DATA_SIGNATURE_LIST";
    public static final String INAPP_CONTINUATION_TOKEN = "INAPP_CONTINUATION_TOKEN";
	
    
    // Billing response codes
    public static final int BILLING_RESPONSE_RESULT_OK = 0;
    public static final int BILLING_RESPONSE_RESULT_USER_CANCELED = 1;
    public static final int BILLING_RESPONSE_RESULT_BILLING_UNAVAILABLE = 3;
    public static final int BILLING_RESPONSE_RESULT_ITEM_UNAVAILABLE = 4;
    public static final int BILLING_RESPONSE_RESULT_DEVELOPER_ERROR = 5;
    public static final int BILLING_RESPONSE_RESULT_ERROR = 6;
    public static final int BILLING_RESPONSE_RESULT_ITEM_ALREADY_OWNED = 7;
    public static final int BILLING_RESPONSE_RESULT_ITEM_NOT_OWNED = 8;
    
    	private void printStack(){
    		StackTraceElement[] ste = Thread.currentThread().getStackTrace();
    		for(StackTraceElement elem : ste){
    			System.out.println("!!!!!!!!!!!!!!!!!! STACKTRACE: "+elem.toString());
    		}
    	}
    	public void printArgs(Object... args){
    		System.out.println("!!!!!!!!!!!! DENTRO PRINTARGS");
    		for(Object obj : args){
    			if(obj != null){
    				if(obj instanceof String)
    					System.out.println("Valore String: "+obj);
    				if(obj instanceof Integer)
    					System.out.println("Valore integet: "+obj);
    				System.out.println("Obj vale: "+obj.toString());
    			}else{
    				System.out.println("!!!!!!!!!!!!! RICEVUTO OBJ NULL");
    			}
    		}
    		printStack();
    		System.out.println("!!!!!!!!!!!! FINE PRINTARGS");
    	}
    	public void myMakePurchaseIntent(Object... args){
    		System.out.println("!!!!!!!!!!!!! DENTRO MAKE PURCHASE INTENT");
    		printArgs(args);
    		System.out.println("!!!!!!!!!!!!! FINE MAKE PURCHASE INTENT");
    	}
    	public void myGetBuyIntent(Object... args){
    		System.out.println("!!!!!!!!!!!!! DENTRO GET BUY INTENT");
    		printArgs(args);
    		System.out.println("!!!!!!!!!!!!! FINE GET BUY INTENT");
    	}
		public void myGetPurchases(Object... args){
			printStack();
			Bundle b = null;
			if(args[3] == null){
				System.out.println("!!!!!!!!!!!!!!!! ERRORE args3 NULL");
				return;
			}
			b = (Bundle)args[3];
			Set<String> s = b.keySet();
			
			if(s.size()>0){
	            for(String item : s){
	            	System.out.println("il bundle contiene chiave: "+item+"valore: ");
	                Object o = b.get(item);
	                System.out.println(o.toString());
	            }
			}
			else{
				System.out.println("!!!!!!!!!!!!!!!!!!!!!! IL BUNDLE NON CONTIENE VALORI!");
				
			}
			/*
			Integer k = BILLING_RESPONSE_RESULT_OK;
			b.putInt(RESPONSE_CODE,k);
			ArrayList<String> ownedSkus = new ArrayList<String>();
			ArrayList<String> purchaseDataList = new ArrayList<String>();
			ArrayList<String> signatureList = new ArrayList<String>();
			
			ownedSkus.add("cippa");
			purchaseDataList.add("cippadata");
			signatureList.add("cippasignature");
			
			b.putStringArrayList(RESPONSE_INAPP_ITEM_LIST, ownedSkus);
			b.putStringArrayList(RESPONSE_INAPP_PURCHASE_DATA_LIST, purchaseDataList);
			b.putStringArrayList(RESPONSE_INAPP_SIGNATURE_LIST, signatureList);
			
			s = b.keySet();
            for(String item : s){
            	System.out.println("il bundle contiene chiave: "+item+"valore: ");
                Object o = b.get(item);
                System.out.println(o.toString());
            }
			*/
			System.out.println("AAAAAAAAAAAAAAAAAAAAAAA:"+b+" VALORE: "+b.toString());
		}
}
