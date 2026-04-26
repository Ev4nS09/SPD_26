package binary_search_multi_threaded; 

import eventb_prelude.*;
import Util.Utilities;

public class dec extends Thread{
	/*@ spec_public */ private a76943_bin_m2 machine; // reference to the machine 

	/*@ public normal_behavior
		requires true;
		assignable \everything;
		ensures this.machine == m; */
	public dec(a76943_bin_m2 m) {
		this.machine = m;
	}

	/*@ public normal_behavior
		requires true;
 		assignable \nothing;
		ensures \result <==> (machine.f.apply(machine.get_r())).compareTo(machine.v) > 0; */
	public /*@ pure */ boolean guard_dec() {
		return (machine.f.apply(machine.get_r())).compareTo(machine.v) > 0;
	}

	/*@ public normal_behavior
		requires guard_dec();
		assignable machine.q, machine.r;
		ensures guard_dec() &&  machine.get_q() == \old(new Integer(machine.get_r() - 1)) &&  machine.get_r() == \old(new Integer(new Integer(new Integer(machine.get_p() + machine.get_r()) - 1) / 2)); 
	 also
		requires !guard_dec();
		assignable \nothing;
		ensures true; */
	public void run_dec(){
		if(guard_dec()) {
			Integer q_tmp = machine.get_q();
			Integer r_tmp = machine.get_r();

			machine.set_q(new Integer(r_tmp - 1));
			machine.set_r(new Integer(new Integer(new Integer(machine.get_p() + r_tmp) - 1) / 2));

			System.out.println("dec executed ");
		}
	}

	public void run() {
		while(true) {
			machine.lock.lock(); // start of critical section
			run_dec();
			machine.lock.unlock(); // end of critical section
		}
	}
}
