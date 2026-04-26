package a76943_binary_search_multi_threaded; 

import eventb_prelude.*;
import Util.Utilities;

public class inc extends Thread{
	/*@ spec_public */ private a76943_bin_m2 machine; // reference to the machine 

	/*@ public normal_behavior
		requires true;
		assignable \everything;
		ensures this.machine == m; */
	public inc(a76943_bin_m2 m) {
		this.machine = m;
	}

	/*@ public normal_behavior
		requires true;
 		assignable \nothing;
		ensures \result <==> (machine.f.apply(machine.get_r())).compareTo(machine.v) < 0; */
	public /*@ pure */ boolean guard_inc() {
		return (machine.f.apply(machine.get_r())).compareTo(machine.v) < 0;
	}

	/*@ public normal_behavior
		requires guard_inc();
		assignable machine.p, machine.r;
		ensures guard_inc() &&  machine.get_p() == \old(new Integer(machine.get_r() + 1)) &&  machine.get_r() == \old(new Integer(new Integer(machine.get_r() + 1 + machine.get_q()) / 2)); 
	 also
		requires !guard_inc();
		assignable \nothing;
		ensures true; */
	public void run_inc(){
		if(guard_inc()) {
			Integer p_tmp = machine.get_p();
			Integer r_tmp = machine.get_r();

			machine.set_p(new Integer(r_tmp + 1));
			machine.set_r(new Integer(new Integer(r_tmp + 1 + machine.get_q()) / 2));

			System.out.println("inc executed ");
		}
	}

	public void run() {
		while(true) {
			machine.lock.lock(); // start of critical section
			run_inc();
			machine.lock.unlock(); // end of critical section
		}
	}
}
