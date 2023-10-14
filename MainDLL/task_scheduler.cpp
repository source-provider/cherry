#include "task_scheduler.hpp"
#include "memory.hpp"
#include <iostream>
#include <mutex>

int wsj_orginal = 0;

#define div(a1, a2, fallback) ((a1 == 0 ? fallback : (a2 == 0 ? fallback : (a1/a2))))

/* scheduler */
auto cherry::scheduler::get(bool bypass) -> int {
	this->tscheduler = cherry::offsets::taskscheduler::get_taskscheduler();
	return this->tscheduler;
}

cherry::scheduler* TASK_OF_SHCEDULER;

auto cherry::scheduler::load(bool bypass) -> int {
	static bool loaded = false;
	int task_scheduler = this->get();;

	if (bypass || !loaded) {
		loaded = true;
		int waiting_scripts_job = this->get_job("WaitingHybridScriptsJob");
		//auto vf_hook = new cherry::memory::vftable{ reinterpret_cast<void*>(waiting_scripts_job) };
		TASK_OF_SHCEDULER = this;

		/*void** vtable = new void* [6];
		memcpy(vtable, *reinterpret_cast<void**>(waiting_scripts_job), 0x18);
		wsj_orginal = reinterpret_cast<uintptr_t>(vtable[2]);

		vtable[2] = sschedur_hooke;
		*reinterpret_cast<void***>(waiting_scripts_job) = vtable;*/

		/*std::function<int(int, int, int)> scheduler_hook = [&](int self, int _, int a2) -> int {
			this->step_scheduler();
			return reinterpret_cast<int(__thiscall*)(uintptr_t, int)>(wsj_orginal)(self, a2);
		};

		wsj_orginal = vf_hook->hook(1, reinterpret_cast<void*>(cherry::memory::get_fn_ptr<0>(scheduler_hook))); */

		this->tcontext = cherry::offsets::taskscheduler::script_context.get(waiting_scripts_job);
		this->tdatamodel = cherry::offsets::taskscheduler::datamodel.get(waiting_scripts_job);
		this->tstate = cherry::offsets::taskscheduler::get_lua_state(this->tcontext);
	}

	return task_scheduler;
}


auto cherry::scheduler::get_jobs() -> std::vector<job_t*> {
	int task_s = this->get();
	auto itier = cherry::offsets::taskscheduler::start.get(task_s);
	auto end = cherry::offsets::taskscheduler::end.get(task_s);
	std::vector<job_t*> jobs{};
	while (itier != end)
	{
		jobs.push_back(*reinterpret_cast<job_t**>(itier));
		itier += 8;
	}

	return jobs;
}

auto cherry::scheduler::get_job(std::string_view job_name) -> int {
	auto jobs = this->get_jobs();
	auto last_job = 0;

	for (job_t* job : jobs)
		if (job->name == job_name)
			last_job = reinterpret_cast<int>(job);

	return last_job;
}

auto cherry::scheduler::state() -> int {
	return this->tstate;
}

auto cherry::scheduler::context() -> int {
	return this->tcontext;
}

auto cherry::scheduler::datamodel() -> int {
	return this->tdatamodel;
}

auto cherry::scheduler::is_loaded() -> bool {
	int waiting_scripts_job = this->get_job("WaitingHybridScriptsJob");

	if (waiting_scripts_job == NULL)
		return false;

	int datamodel_new = *reinterpret_cast<int*>(waiting_scripts_job + cherry::offsets::taskscheduler::datamodel.get_offset());
	return (this->tdatamodel != datamodel_new);
}

/* fps */
auto cherry::scheduler::setfps(double fps) -> void {
	fps = div(1, fps, 0.001); // 0.001 == 1000 fps
	cherry::offsets::taskscheduler::fps.set(this->tscheduler, fps);
}

auto cherry::scheduler::getfps() -> double {
	return div(1, cherry::offsets::taskscheduler::fps.get(this->tscheduler), 0);
}

/* stack */
auto cherry::scheduler::top() -> scheduler_task {
	scheduler_task data = this->queue.top();
	this->queue.pop();
	return data;
}

auto cherry::scheduler::push(std::string code) -> void {
	std::unique_lock<std::mutex> guard{ this->mutex };
	this->queue.emplace(scheduler_task{ code });
}

auto cherry::scheduler::push(std::function<void(int)> func) -> void {
	std::unique_lock<std::mutex> guard{ this->mutex };
	this->queue.emplace(scheduler_task{ func });
}

auto cherry::scheduler::empty() -> bool {
	return this->queue.empty();
}

auto cherry::scheduler::clear() -> void {
	while (!this->queue.empty())
		this->queue.pop();
}