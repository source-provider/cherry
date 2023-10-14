#include "vfhook.h"

namespace cherry {
	vfhook::vfhook() {
		return;
	};

	vfhook::vfhook(void* instance, std::optional<std::intptr_t> size) {
		this->vfBackup = *reinterpret_cast<std::intptr_t**>(instance);

		if (size.value() != -1) {
			while (reinterpret_cast<std::intptr_t*>(*reinterpret_cast<std::intptr_t*>(instance))[this->size])
				this->size++;
		}
		else {
			this->size = size.value();
		}

		const std::intptr_t vtSize = ((this->size * 0x4) + 0x4);
		this->vfClone = std::make_unique<std::intptr_t[]>(this->size + 1);

		memcpy(this->vfClone.get(), &this->vfBackup[-1], vtSize);
		*reinterpret_cast<std::intptr_t**>(instance) = &this->vfClone.get()[1];
	};

	auto vfhook::evictHook(void* instance, std::optional<std::intptr_t> size) -> void {
		this->vfBackup = *reinterpret_cast<std::intptr_t**>(instance);

		if (size.value() != -1) {
			while (reinterpret_cast<std::intptr_t*>(*reinterpret_cast<std::intptr_t*>(instance))[this->size])
				this->size++;
		}
		else {
			this->size = size.value();
		}

		const std::intptr_t vtSize = ((this->size * 0x4) + 0x4);
		this->vfClone = std::make_unique<std::intptr_t[]>(this->size + 1);

		memcpy(this->vfClone.get(), &this->vfBackup[-1], vtSize);
		*reinterpret_cast<std::intptr_t**>(instance) = &this->vfClone.get()[1];
	};

	auto vfhook::getOriginal(std::intptr_t idx) -> std::intptr_t {
		if (idx > this->size)
			return 0;

		return (std::intptr_t)(reinterpret_cast<void*>(this->vfBackup[idx]));
	};

	auto vfhook::hook(std::intptr_t idx, void* func) -> std::intptr_t {
		if (idx > this->size)
			return 0;

		std::intptr_t originalFunc = this->vfClone[idx + 1];
		this->vfClone[idx + 1] = reinterpret_cast<std::intptr_t>(func);

		return originalFunc;
	};

	auto vfhook::revert(std::intptr_t idx) -> void {
		if (idx > this->size)
			return;

		std::intptr_t originalFunc = this->vfBackup[idx];
		this->vfClone[idx + 1] = originalFunc;
	};
}