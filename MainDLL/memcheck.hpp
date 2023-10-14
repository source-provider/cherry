#include "utilities.hpp"
#include <luaconf.h>

namespace cherry {
	class memcheck {
	public:
		int original_hook = 0, last = 0, silent_checkers = 0;
		std::vector<cherry::utilities::hasher_t> populated_hashes;
		cherry::utilities::section_t text, rdata, vmpx, vmp0, vmp1;
		int* region_list = nullptr;
		size_t* region_sizes = nullptr;
	public:
		cherry::utilities* utils = cherry::utilities::get_singleton();
		cherry::console* console = cherry::console::get_singleton();
		cherry::scheduler* scheduler = cherry::scheduler::get_singleton();

		memcheck() {
			this->utils = cherry::utilities::get_singleton();
			this->console = cherry::console::get_singleton();
			this->scheduler = cherry::scheduler::get_singleton();
		}

		static auto get_singleton() -> cherry::memcheck* {
			static cherry::memcheck* _memcheck = nullptr;

			if (_memcheck == nullptr)
				_memcheck = new cherry::memcheck();

			return _memcheck;
		}

		auto bypass() -> void;
	};
}