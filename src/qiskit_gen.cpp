
#include "../headers/generator.h"

std::vector<std::string> qiskit::all_passes = {"Optimize1qGates", "Optimize1qGatesDecomposition", "Collect1qRuns", "Collect2qBlocks",
    "CollectMultiQBlocks","CollectLinearFunctions","CollectCliffords","ConsolidateBlocks","CXCancellation","InverseCancellation",
    "CommutationAnalysis","CommutativeCancellation","CommutativeInverseCancellation","Optimize1qGatesSimpleCommutation","RemoveDiagonalGatesBeforeMeasure",
    "RemoveResetInZeroState","RemoveFinalReset","HoareOptimizer","TemplateOptimization","ResetAfterMeasureSimplification",
    "OptimizeCliffords","ElidePermutations","NormalizeRXAngle","OptimizeAnnotated"
};

void qiskit::write_imports(std::ofstream& stream){

	stream << "from qiskit import QuantumCircuit, QuantumRegister, ClassicalRegister" << std::endl;
	stream << "from qiskit.circuit import Parameter, ParameterVector" << std::endl;
	stream << "from helpers.qiskit_helpers import compare_statevectors, run_on_simulator, run_routing_simulation, run_pass_on_simulator" << std::endl;
	stream << "from pathlib import Path" << std::endl; 
	stream << "from math import pi" << std::endl;

	stream << "\n";
}

void qiskit::add_resources(std::ofstream& stream, circuit_info& info){
	Resource r;
	std::string qreg_name, creg_name;
	int qreg_size, creg_size, added_resources = 0;
	r.rt = named;

	stream << "# Adding qregs " << std::endl;

	// fill up available qubit resources with qregs
	while(added_resources < info.n_named_qubits){
		qreg_name = "qreg_" + std::to_string(added_resources);
		qreg_size = get_rand(1, info.n_named_qubits - added_resources);

		r._reg = std::make_pair(qreg_name, qreg_size);

		stream << qreg_name << " = QuantumRegister(" << qreg_size << ")" << std::endl;
		stream << info.name << ".add_register(" << qreg_name << ")" << std::endl;

		for(int i = 0; i < qreg_size; ++i){
			r.str = qreg_name + "[" + std::to_string(i) + "]";
			info.add_qubit_resources(r); 
		}

		added_resources += qreg_size;
	}

	added_resources = 0;

	stream << "# Adding creg resources " << std::endl;

	// fill up available bit resources with creg resources
	while(added_resources < info.n_named_bits){
		// add qreg resource
		creg_name = "creg_" + std::to_string(added_resources);
		creg_size = get_rand(1, info.n_named_bits - added_resources);

		r._reg = std::make_pair(creg_name, creg_size);

		stream << creg_name << " = ClassicalRegister(" << creg_size << ")" << std::endl;
		stream << info.name << ".add_register(" << creg_name << ")" << std::endl;

		for(int i = 0; i < creg_size; ++i){
			r.str = creg_name + "[" + std::to_string(i) + "]";
			info.add_bit_resources(r); 
		}

		added_resources += creg_size;
	}
}

void qiskit::write_circuit(std::ofstream& stream, circuit_info& info){

	info.reset_state();

	// init qubit resources (index)
	for(int i = 0; i < info.n_init_qubits; ++i){
		info.add_qubit_resources(Resource{index, std::to_string(i)});
	}

	stream << info.name << " = QuantumCircuit(" << info.n_init_qubits << ")" << std::endl;

	add_resources(stream, info);

	assert(info.available_qubit_resources.size() == (size_t)info.get_total_qubits());
	assert(info.available_bit_resources.size() == (size_t)info.get_total_bits());

	if(info.circ_kind == main_circ){
		add_symbols(stream, info);
	}

	info.nest_depth = get_rand(0, 1) ? get_rand(1, MAX_NEST_DEPTH) : 0;

	apply_gates(stream, info, info.n_gates_to_add, info.nest_depth);

	if((info.circ_kind == main_circ)){
		info.write_param_bindings(stream);	
		stream << info.name << " = " << info.name << ".assign_parameters(bindings)" << std::endl;
	}

	#ifdef DEV
		std::cout << info << std::endl;
	#endif

}

/// Apply a given number of gates in this block
int qiskit::apply_gates(std::ofstream& stream, circuit_info& info, int gates_to_apply, int nest_depth){
	Gate* gate;
	int control_flow_choice;
	int added_gates = 0, latest_application = 0;
	
	while(added_gates < gates_to_apply){
		control_flow_choice = get_rand(0, 3) & -(info.circ_kind != subroutine); // minus because bool true is 1 in int
		
		if(control_flow_choice == 0){
			gate = info.get_random_gate();

			latest_application = apply_gate(stream, info, *gate, get_any_qubits);

			if(!latest_application && (gates_to_apply != info.n_gates_to_add)){
				// couldn't do all nested statements without overflowing n_gates
				// write noop or barrier
				if(get_rand(0, 1)){
					add_noop(stream, info);
				} else {
					add_barrier(stream, info);
				}
				break;
			} 
			
			added_gates += latest_application;

		} else if (control_flow_choice == 1){
		 	added_gates += write_if(stream, info, nest_depth);

		} else if (control_flow_choice == 2){
			added_gates += write_if_else(stream, info, nest_depth);

		} else if (control_flow_choice == 3){
			added_gates += write_switch(stream, info, nest_depth);

		}
	}

	info.unindent();
	
	#ifdef DEV
		std::cout << "Added " << added_gates << " gates (including circbox gates)" << std::endl;
	#endif

	return added_gates;
}

// measure a qubit state into this bit resource
void qiskit::add_measure(std::ofstream& stream, circuit_info& info, Resource* br){
	Resource* qr = info.choose_random_resource(true);

	// make a mid circuit measurement of a qubit into this bit
	stream << info.tab << info.name << ".measure(" << qr->str << ", " << br->str << ")" << std::endl;
}

void qiskit::add_barrier(std::ofstream& stream, circuit_info& info){
	Resource* qr = info.choose_random_resource(true);

	// add a barrier on this qubit
	stream << info.tab << info.name << ".barrier(" << qr->str << ")" << std::endl;
}

void qiskit::add_noop(std::ofstream& stream, circuit_info& info){
	Resource* qr = info.choose_random_resource(true);

	// add a barrier on this qubit
	stream << info.tab << info.name << ".id(" << qr->str << ")" << std::endl;
}

int qiskit::write_if(std::ofstream& stream, circuit_info& info, int depth){
	if(depth == 0){
		return 0;
	} else {
		Gate gate = info.gateset[get_rand(0, info.gateset.size()-1)];
		Resource* r = info.choose_random_resource(false);
		
		add_measure(stream, info, r);

		stream << info.tab << "with " << info.name << ".if_test((" << r->str << ",0)):" << std::endl;

		info.indent();

		return apply_gates(stream, info, get_rand(1, MAX_NEST_STATEMENTS), depth-1);
	}
}

int qiskit::write_if_else(std::ofstream& stream, circuit_info& info, int depth){
	if(depth == 0){
		return 0;
	} else {
		Gate gate = info.gateset[get_rand(0, info.gateset.size()-1)];
		Resource* r = info.choose_random_resource(false);

		add_measure(stream, info, r);

		int added_t = 0, added_f = 0;

		stream << info.tab << "with " << info.name << ".if_test((" << r->str << ",0)) as else_" << depth << ":" << std::endl;

		info.indent();

		added_t = apply_gates(stream, info, get_rand(1, MAX_NEST_STATEMENTS), depth-1);
		
		stream << info.tab << "with else_" << depth << ":" << std::endl;

		info.indent();

		added_f = apply_gates(stream, info, get_rand(1, MAX_NEST_STATEMENTS), depth-1);

		return added_t + added_f;
	}
}

int qiskit::write_switch(std::ofstream& stream, circuit_info& info, int depth){
	if(depth == 0){
		return 0;
	} else {

		Gate gate = info.gateset[get_rand(0, info.gateset.size()-1)];
		Resource* r = info.choose_random_resource(false); // single bit

		int max_value = 1;
		int added_gates = 0;

		add_measure(stream, info, r);

		stream << info.tab << "with " << info.name << ".switch(" << r->str << ") as case_" << depth << ":" << std::endl;
		info.indent();

		for(int i = 0; i <= max_value; ++i){
			stream << info.tab << "with case_" << depth << "(" << i << "):" << std::endl;
			info.indent();
			added_gates += apply_gates(stream, info, 4, depth-1);
		}

		info.unindent();

		return added_gates;
	}
}

void qiskit::write_circboxes(std::ofstream& stream, circuit_info& info){
	circuit_info circ("", subroutine, info.gateset);
	
	for(int i = 0; i < info.n_subroutines; ++i){
		circ.name = "subcirc" + std::to_string(i);

		Gate gate(circ.get_total_qubits(), circ.n_gates_to_add, circbox);
		gate.name = circ.name;	
	
		write_circuit(stream, circ);

		// randomly make some circboxes controlled
		if(get_rand(0, 1)){
			int num_control_qubits = get_rand(1, MAX_CIRCBOX_CONTROL_QUBITS);
			stream << gate.name << " = " << gate.name << ".to_gate().control(" << num_control_qubits << ")" << std::endl; 
			gate.num_of_resources += num_control_qubits;
		}

		gate.num_of_control_gates = circ.n_control_gates_added;
		gate.set_vars();

		stream << std::endl;

		info.gateset.push_back(gate);  // add back to your parent's gateset
	}
}

void qiskit::generate_circuits(int n){
	fs::path circuit_filename;
	int passes_circuits = 0;

	if(!fs::exists(CIRCUITS_DIR)){
		fs::create_directory(CIRCUITS_DIR);
	}

	Circuit_Restrictions cr{"Qiskit_Gateset", qiskit_flags};

	for(int i = 0; i < n; ++i){
		
		#ifdef DEV
			std::cout << "================== circuit" << i+1 << ".py =========================\n" << std::endl;
		#endif

		circuit_filename = CIRCUITS_DIR / ("circuit" + std::to_string(i+1) + ".py");
	
		std::ofstream stream(circuit_filename.c_str());

		circuit_info global_info("main_circ", main_circ, &cr);

		int total_qubits = get_rand(MIN_TOTAL_QUBITS, MAX_TOTAL_QUBITS);

		global_info.n_init_qubits = (get_rand(0, 1) ? MIN_TOTAL_QUBITS : total_qubits-MIN_TOTAL_QUBITS);
		global_info.n_named_qubits = total_qubits - global_info.n_init_qubits;
		global_info.n_named_bits = N_NAMED_BITS;

		write_imports(stream);

		write_circboxes(stream, global_info);

		write_circuit(stream, global_info);

		stream << "\n";
		stream << "print(Path(__file__).name, \" results:\")" << std::endl;

		if(global_info.nest_depth){	
			stream << global_info.name << ".measure_active()" << std::endl;

			// choose whether to apply a single pass or optimisation levels
			if(get_rand(0, 1)){
				stream << "run_on_simulator(main_circ, \"" << i+1 << "\")" << std::endl;
			} else {
				stream << "run_pass_on_simulator(main_circ, \"" << i+1 << "\", \"" << all_passes[passes_circuits++ % all_passes.size()] << "\")" << std::endl;
			}
			
		} else{
			//Randomly choose whether to do a optimisation pass or a routing pass
			if (get_rand(0, 1)) {	
				stream << "compare_statevectors(" << global_info.name << ", \"" << all_passes[passes_circuits++ % all_passes.size()] << "\")" << std::endl;
			}
			else {
				stream << global_info.name << ".measure_active()" << std::endl;
				stream << "run_routing_simulation(main_circ, \"" << i+1 << "\")" << std::endl;
			}
		}

		stream.close();
	}
}
