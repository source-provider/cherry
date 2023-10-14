#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <functional>

namespace cherry {
	class execution {
	private:
		bool init = false;
	public:
		static auto get_singleton() -> execution* {
			static execution* _execution = nullptr;

			if (_execution == nullptr)
				_execution = new execution();

			return _execution;
		}

		auto send(int state, std::string script, bool push_to_scheduler = true) -> int;
		auto launch_pipe() -> void;
	};
}