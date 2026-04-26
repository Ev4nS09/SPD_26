package binary_search_multi_threaded;

import eventb_prelude.*;
import Util.*;
//@ model import org.jmlspecs.models.JMLObjectSet;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class a76943_bin_m2{
	private int n_events = 3;
	private static final Integer max_integer = Utilities.max_integer;
	private static final Integer min_integer = Utilities.min_integer;
	private Thread[] events;
	public Lock lock = new ReentrantLock(true);


	/******Set definitions******/
	//@ public static constraint MYGAGASET.equals(\old(MYGAGASET)); 
	public static final BSet<Integer> MYGAGASET = new Enumerated(min_integer,max_integer);


	/******Constant definitions******/
	//@ public static constraint f.equals(\old(f)); 
	public static final BRelation<Integer,Integer> f = Test_a76943_bin_m2.random_f;

	//@ public static constraint n.equals(\old(n)); 
	public static final Integer n = Test_a76943_bin_m2.random_n;

	//@ public static constraint v.equals(\old(v)); 
	public static final Integer v = Test_a76943_bin_m2.random_v;



	/******Axiom definitions******/
	/*@ public static invariant  f.domain().equals(new Enumerated(new Integer(1),n)) && f.range().isSubset(NAT.instance) && f.isaFunction() && BRelation.cross(new Enumerated(new Integer(1),n),NAT.instance).has(f); */
	/*@ public static invariant  (\forall Integer i;  (\forall Integer j;((new Enumerated(new Integer(1),n).has(i) && new Enumerated(new Integer(1),n).has(j) && (i).compareTo(j) <= 0) ==> ((f.apply(i)).compareTo(f.apply(j)) <= 0)))); */
	/*@ public static invariant f.range().has(v); */
	/*@ public static invariant_redundantly (n).compareTo(new Integer(1)) >= 0; */


	/******Variable definitions******/
	/*@ spec_public */ private Integer p;

	/*@ spec_public */ private Integer q;

	/*@ spec_public */ private Integer r;




	/******Invariant definition******/
	/*@ public invariant
		NAT.instance.has(r) &&
		new Enumerated(new Integer(1),n).has(p) &&
		new Enumerated(new Integer(1),n).has(q) &&
		new Enumerated(p,q).has(r) &&
		f.image(new Enumerated(p,q)).has(v); */


	/******Getter and Mutator methods definition******/
	/*@ public normal_behavior
	    requires true;
	    assignable \nothing;
	    ensures \result == this.p;*/
	public /*@ pure */ Integer get_p(){
		return this.p;
	}

	/*@ public normal_behavior
	    requires true;
	    assignable this.p;
	    ensures this.p == p;*/
	public void set_p(Integer p){
		this.p = p;
	}

	/*@ public normal_behavior
	    requires true;
	    assignable \nothing;
	    ensures \result == this.q;*/
	public /*@ pure */ Integer get_q(){
		return this.q;
	}

	/*@ public normal_behavior
	    requires true;
	    assignable this.q;
	    ensures this.q == q;*/
	public void set_q(Integer q){
		this.q = q;
	}

	/*@ public normal_behavior
	    requires true;
	    assignable \nothing;
	    ensures \result == this.r;*/
	public /*@ pure */ Integer get_r(){
		return this.r;
	}

	/*@ public normal_behavior
	    requires true;
	    assignable this.r;
	    ensures this.r == r;*/
	public void set_r(Integer r){
		this.r = r;
	}



	/*@ public normal_behavior
	    requires true;
	    assignable \everything;
	    ensures
		p == 1 &&
		q == n &&
		r == new Integer(new Integer(n + 1) / 2);*/
	public a76943_bin_m2(){
		p = 1;
		q = n;
		r = new Integer(new Integer(n + 1) / 2);

		events = new Thread[n_events];
		events[0] = new dec(this);
		events[1] = new found(this);
		events[2] = new inc(this);

		for (int i = 0; i < n_events;i++){
			events[i].start();
		}
	}
}