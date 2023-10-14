#include "scheduler.h"
#include <roblox/offsets/addresses.h>
#include <roblox/classes/vfhook/vfhook.h>

auto getTaskScheduler = reinterpret_cast<cherry::addressTypes::taskScheduler>(cherry::addresses::taskScheduler);
auto getState = reinterpret_cast<cherry::addressTypes::getState>(cherry::addresses::getState);
namespace cherry {
	scheduler* scheduler::singleton{ nullptr };
	scheduler::scheduler() {
		this->taskScheduler = getTaskScheduler();
		schedulerHook = new vfhook();

		std::intptr_t jobStart{ *reinterpret_cast<std::intptr_t*>(this->taskScheduler + offsets::schedulerJobStart) };
		std::intptr_t jobEnd{ *reinterpret_cast<std::intptr_t*>(this->taskScheduler + offsets::schedulerJobEnd) };

		while (jobStart != jobEnd) {
			this->jobVector.push_back(*(std::intptr_t*)(jobStart));
			jobStart += 8;
		}
	};

	auto scheduler::getSingleton() -> scheduler* {
		if (singleton == nullptr)
			singleton = new scheduler();

		return singleton;
	};

	auto scheduler::reInitialize() -> void {
		this->taskScheduler = getTaskScheduler();

		std::intptr_t jobStart{ *reinterpret_cast<std::intptr_t*>(this->taskScheduler + offsets::schedulerJobStart) };
		std::intptr_t jobEnd{ *reinterpret_cast<std::intptr_t*>(this->taskScheduler + offsets::schedulerJobEnd) };

		while (jobStart != jobEnd) {
			this->jobVector.push_back(*(std::intptr_t*)(jobStart));
			jobStart += 8;
		}
	};

	auto scheduler::getJobName(std::intptr_t job) -> std::string {
		return *reinterpret_cast<std::string*>(job + offsets::schedulerJobName);
	};

	auto scheduler::getJob(std::string_view name) -> std::intptr_t {
		std::intptr_t jobRet{ NULL };

		for (std::intptr_t job : this->jobVector) {
			if (this->getJobName(job) == name)
				jobRet = job;
		}

		return jobRet;
	};

	auto scheduler::jobHook(void* hookFunc) -> std::intptr_t {
		this->schedulerHook->evictHook((void*)this->getJob("WaitingHybridScriptsJob"), 6);
		return this->schedulerHook->hook(2, hookFunc);
	};

	auto scheduler::getScriptContext() -> std::intptr_t {
		auto waitingHybridScriptsJob{ this->getJob("WaitingHybridScriptsJob") };
		return *reinterpret_cast<std::intptr_t*>(waitingHybridScriptsJob + offsets::scriptJobScriptContext);
	};

	auto scheduler::getDataModel() -> std::intptr_t {
		auto waitingHybridScriptsJob{ this->getJob("WaitingHybridScriptsJob") };
		return *reinterpret_cast<std::intptr_t*>(waitingHybridScriptsJob + offsets::scriptJobDataModel);
	};

	auto scheduler::getLuaState() -> lua_State* {
		std::intptr_t script[] = { 0, 0 };
		std::intptr_t identity{ 0 };
		return getState(this->getScriptContext(), &identity, script);
	};
}