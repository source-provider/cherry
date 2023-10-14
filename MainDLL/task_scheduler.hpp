#include <Windows.h>
#include <string>
#include <stack>
#include <functional>

#include "offsets.hpp"


struct job_t
{
	uintptr_t* vftable;
	uint8_t pad0[12];
	std::string name;
	uint8_t pad1[16];
	double time;
	uint8_t pad2[16];
	double time_spend;
	uint8_t pad3[8];
	uintptr_t state;
};

namespace cherry
{
	class scheduler_task {
	public:
		bool isc = false;
		std::string script;
		std::function<void(int)> func;

		scheduler_task(std::string script) : script{ script } {}

		scheduler_task(std::function<void(int)> func) : func{ std::move(func) } {
			this->isc = true;
		}
	};

	class scheduler {
	private:
		std::stack<scheduler_task> queue{};
		int tscheduler = 0, tstate = 0, tcontext = 0, tdatamodel = 0;
	public:
		std::mutex mutex;
		static void step_scheduler();
		static scheduler* get_singleton()
		{
			static scheduler* _scheduler = nullptr;

			if (_scheduler == nullptr)
				_scheduler = new scheduler();

			return _scheduler;
		}

		/* scheduler */
		auto get(bool bypass = false) -> int;
		auto load(bool bypass = false) -> int;
		auto get_jobs() -> std::vector<job_t*>;
		auto get_job(std::string_view job_name) -> int;

		auto state() -> int;
		auto context() -> int;
		auto datamodel() -> int;

		auto is_loaded() -> bool;

		/* fps */
		auto setfps(double fps) -> void;
		auto getfps() -> double;

		/* stack */

		auto top() -> scheduler_task;
		auto push(std::string code) -> void;
		auto push(std::function<void(int)> func) -> void;
		auto empty() -> bool;
		auto clear() -> void;
	};
}