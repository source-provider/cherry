#include "proto_disassembler.hpp"

auto proto_disassemble(Proto* p, std::string proto_name) -> std::string {
	std::string output = "";

#pragma region functions
	std::function<void(std::string, std::string)> add_comment = [&](std::string data, std::string value) {
		output += ("; " + data + ": " + value + "\n");
	};

	std::function<void(std::string, int)> add_comment_int = [&](std::string data, int value) {
		output += ("; " + data + ": " + std::to_string(value) + "\n");
	};

	std::function<void(std::string, double)> add_comment_double = [&](std::string data, double value) {
		output += ("; " + data + ": " + std::to_string(value) + "\n");
	};

	std::function<void(std::string, TString*)> add_comment_tstring = [&](std::string data, TString* value) {
		const char* value1 = getstr(value);
		output += ("; " + data + ": " + value1 + "\n");
	};
#pragma endregion

	

	/* proto data */
	add_comment_int("global id", p->bytecodeid);

	TString* debug_name_t = p->debugname;
	const char* debug_name = cherry::global::utilities->random_string(16).c_str();

	if (debug_name != nullptr) {
		debug_name = getstr(debug_name_t);
	}
	add_comment("proto name", debug_name);

	if (p->linedefined != NULL) {
		add_comment_int("line defined", p->linedefined);
	}

	add_comment_int("maxstacksize", p->maxstacksize);
	add_comment_int("numparams", p->numparams);
	add_comment_int("nups", p->nups);
	add_comment_int("is_vararg", p->is_vararg);
	
	/*if (p->sizep > 0) {
		std::string value = "";
		for (int i = 0; i < p->sizep; i++) {
			Proto* p_data = p->p[i];
			value += std::to_string(p_data->bytecodeid) + ", ";
		}
		value = value.substr(0, value.size() - 2);

		add_comment("child protos", value);
	}*/

	/* disassembler */
	/*int pc = 1;

	while (pc <= p->sizecode) {
		Instruction insn = p->code[pc];

	}*/

	return output;
}