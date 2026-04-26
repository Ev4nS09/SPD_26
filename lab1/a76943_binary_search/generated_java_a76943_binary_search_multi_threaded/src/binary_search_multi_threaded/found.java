package binary_search_multi_threaded; 

import eventb_prelude.*;
import Util.Utilities;

public class found extends Thread{
	/*@ spec_public */ private bin_m2 machine; // reference to the machine 

	/*@ public normal_behavior
		requires true;
		assignable \everything;
		ensures this.machine == m; */
	public found(bin_m2 m) {
		this.machine = m;
	}

	/*@ public normal_behavior
		requires true;
 		assignable \nothing;
		ensures \result <==> machine.f.apply(machine.get_r()).equals(machine.v); */
	public /*@ pure */ boolean guard_found() {
		return machine.f.apply(machine.get_r()).equals(machine.v);
	}

	/*@ public normal_behavior
		requires guard_found();
		assignable \nothing;
		ensures guard_found() && true; 
	 also
		requires !guard_found();
		assignable \nothing;
		ensures true; */
	public void run_found(){
		if(guard_found()) {
			System.out.println("found executed ");
			System.out.println("result = " +this.machine.get_r());
		}
		
		System.exit(1);
	}

	public void run() {
		while(true) {
			machine.lock.lock(); // start of critical section
			run_found();
			machine.lock.unlock(); // end of critical section
		}
	}
}
