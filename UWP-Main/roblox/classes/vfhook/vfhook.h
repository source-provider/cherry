#pragma once
#include <Windows.h>
#include <cstdint>
#include <optional>
#include <memory>

namespace cherry {
	class vfhook {
	private:
		std::unique_ptr<std::intptr_t[]> vfClone = nullptr;
		std::intptr_t* vfBackup = nullptr;
		std::intptr_t size = 0;
	public:
		explicit vfhook();
		explicit vfhook(void* instance, std::optional<std::intptr_t> size);
		auto evictHook(void* instance, std::optional<std::intptr_t> size) -> void;
		auto getOriginal(std::intptr_t idx) -> std::intptr_t;
		auto hook(std::intptr_t idx, void* func) -> std::intptr_t;
		auto revert(std::intptr_t idx) -> void;
	};
}