#include "memcheck.hpp"
//#include <luau/vm/include/luaconf.h>
#include "Luau/Compiler.h"
#include "Luau/BytecodeBuilder.h"
//
//#include "console.hpp"
cherry::memcheck* memcheck_bypass;

#define memcheck_assert(chck, msg) \
if (!chck) \
	throw std::runtime_error(msg);

#define memcheck_goto(chck, bl, label) \
bounds = old_b;\
if (!chck && bl == false)\
{\
	bl = true;\
	bounds = bounds_text;\
	goto label;\
}

#define cast_array(t, v) reinterpret_cast<t*>(v)
#define cast_int(v) reinterpret_cast<int>(v)
#define defer_item(v) *reinterpret_cast<int*>(v)
using silent_hasher_t = size_t(__cdecl*)(int, int, int, int);

auto add_to_list(int entry) -> void {
	cherry::utilities::hasher_t hasher{};
	hasher.entry = entry;

	for (size_t i = 0; i < 30; i++)
	{
		int hash_start = cherry::offsets::memcheck::hasher + cherry::offsets::memcheck::deciper_region(memcheck_bypass->region_list[i]);
		int hash_size = cherry::offsets::memcheck::deciper_region(memcheck_bypass->region_sizes[i]);
		size_t hash = reinterpret_cast<silent_hasher_t>(entry)(hash_start, hash_size, 0, 0);

		if (hash_size > 0 && hasher.enc == (int)ENC::NONE)
			hasher.enc = memcheck_bypass->utils->get_encryption(entry, hash, hash_start, hash_size);

		if (hasher.entry != memcheck_bypass->last) {
			//memcheck_bypass->console->dbgprintf("HASHER", "%i 0x%X\n", memcheck_bypass->silent_checkers, hasher.entry);
			memcheck_bypass->last = hasher.entry;
		}

		hasher.hashes[hash_start] = hash;
	}

	memcheck_assert((hasher.enc != (int)ENC::NONE), "Failed to grab the hash type");

	memcheck_bypass->populated_hashes.push_back(hasher);
}

auto spoofed_address(int address) -> int {
	if (address >= memcheck_bypass->text.start && address <= memcheck_bypass->text.start + memcheck_bypass->text.size)
		return address - memcheck_bypass->text.start + memcheck_bypass->text.clone;
	else if (address >= memcheck_bypass->rdata.start && address <= memcheck_bypass->rdata.start + memcheck_bypass->rdata.size)
		return address - memcheck_bypass->rdata.start + memcheck_bypass->rdata.clone;
	else if (address >= memcheck_bypass->vmpx.start && address <= memcheck_bypass->vmpx.start + memcheck_bypass->vmpx.size)
		return address - memcheck_bypass->vmpx.start + memcheck_bypass->vmpx.clone;
	else if (address >= memcheck_bypass->vmp1.start && address <= memcheck_bypass->vmp1.start + memcheck_bypass->vmp1.size)
		return address - memcheck_bypass->vmp1.start + memcheck_bypass->vmp1.clone;

	return address;
}

size_t __stdcall silent_hasher_spoof(size_t hasher, int start, int size, int unk, int seed)
{
	auto& hasher_list = memcheck_bypass->populated_hashes[hasher];
	auto cache = hasher_list.hashes.find(start);

	memcheck_assert((cache != hasher_list.hashes.end()), "This should not happen");

	switch ((ENC)hasher_list.enc)
	{
	case ENC::ADD:
		return cache->second + seed;
	case ENC::SUB1:
		return cache->second - seed;
	case ENC::XOR:
		return cache->second ^ seed;
	default:
		break;
	}

	return 0;
}

void __declspec(naked) silent_sub()
{
	__asm
	{
		push[esp + 20];
		push[esp + 20];
		push[esp + 20];
		push[esp + 20];

		push[esp + 16];
		call silent_hasher_spoof;
		add esp, 4;

		ret;
	}
}

int esp_backup = 0, spoofed = 0;
__declspec(naked) void hasher_stub()
{
	__asm
	{
		mov esp_backup, esp;
		sub esp, 0x300;

		pushad;

		push ebx;
		call spoofed_address;
		add esp, 4;

		mov spoofed, eax;

		popad;

		add esp, 0x300;
		mov eax, spoofed;
		mov esp, eax;

	hasher_start:

		mov eax, [esp];
		add eax, ebx;
		imul eax, 0x1594FE2D; // loop1_enc
		add eax, edi;
		rol eax, 0x13;
		imul edi, eax, 0x0CBB4ABF7; // cant seem to find this

		lea eax, [ebx + 4];

		sub eax, [esp + 4];
		add ebx, 8;
		add esp, 8;

		imul eax, 0x344B5409; // loop2_enc
		add eax, [ebp - 0x10]; // 0x10 can update
		rol eax, 0x11;
		imul eax, 0x1594FE2D; // loop1_enc
		mov[ebp - 0x10], eax; // 0x10 can update

		mov eax, [esp];
		xor eax, ebx;
		add ebx, 4;
		add esp, 4;
		imul eax, 0x1594FE2D; // loop1_enc
		add eax, [ebp - 0xC];
		rol eax, 0xD;
		imul eax, 3417615351; // cant seem to find this (0x0CBB4ABF7)
		mov[ebp - 0xC], eax;

		mov eax, [esp];
		sub eax, ebx;
		add ebx, 4;
		add esp, 4;
		imul eax, 0x344B5409; // loop2_enc
		add eax, esi;
		rol eax, 0xF;
		imul esi, eax, 0x1594FE2D; // loop1_enc
		cmp ebx, ecx;
		jb hasher_start;

		mov esp, esp_backup;
		jmp[cherry::offsets::memcheck::core_end];
	}

}

auto cherry::memcheck::bypass() -> void {
	memcheck_bypass = this;

	this->utils = cherry::utilities::get_singleton();

	this->text = this->utils->get_section(".text");
	this->rdata = this->utils->get_section(".rdata");
	this->vmpx = this->utils->get_section(".vmpx");
	this->vmp0 = this->utils->get_section(".vmp0", false); // Not cloned, nont scanned, needed in checks
	this->vmp1 = this->utils->get_section(".vmp1");

	/* regions */
	auto bounds = std::pair<int, int>(this->vmp0.start, this->vmp0.start + this->vmp0.size);
	auto bounds_text = std::pair<int, int>(this->text.start, this->text.start + this->text.size);
	std::pair<int, int> old_b = bounds;

	/* regions */
	bool scanned0 = false, scanned1 = false;

	//memcheck_bypass->console->debug_write("STARTING", "MEMCHECK");
	bounds = std::pair<int, int>(this->vmp0.start, this->vmp0.start + this->vmp0.size);
	bounds_text = std::pair<int, int>(this->text.start, this->text.start + this->text.size);
region_list_scan:
	for (auto& res : this->utils->sig_scan(offsets::memcheck::sigs::regions_sig, offsets::memcheck::sigs::regions_pattern, bounds, 0)) {
		int region = defer_item(res + 3);
		if (region >= this->vmpx.start && region <= this->vmpx.start + this->vmpx.size) {
			auto result1 = offsets::memcheck::hasher + offsets::memcheck::deciper_region(defer_item(region));
			if (result1 == cast_int(GetModuleHandle(NULL)) + 0x1000)
				this->region_list = cast_array(int, region);
		}
	}

	memcheck_goto(region_list, scanned0, region_list_scan);
	memcheck_assert(this->region_list, "Failed to grab the region list");


	//memcheck_bypass->console->dbgprintf("REGION", "0x%X\n", region_list, 0);

region_size_scan:
	for (auto& res : this->utils->sig_scan(offsets::memcheck::sigs::regions_sig1, offsets::memcheck::sigs::regions_pattern, bounds, 0)) {
		int region = defer_item(res + 3);
		if (region >= this->vmpx.start && region <= this->vmpx.start + this->vmpx.size) {
			auto result1 = offsets::memcheck::deciper_region(defer_item(region));

			auto first = offsets::memcheck::hasher + offsets::memcheck::deciper_region(this->region_list[0]);
			auto second = offsets::memcheck::hasher + offsets::memcheck::deciper_region(this->region_list[1]);

			if (result1 == second - first)
				this->region_sizes = cast_array(size_t, region);
		}
	}
	memcheck_goto(region_sizes, scanned1, region_size_scan);
	memcheck_assert(this->region_sizes, "Failed to grab the region size");

	//memcheck_bypass->console->dbgprintf("REGION", "0x%X\n", region_sizes, 0);

	/* secondary hashers */

	for (const auto& res : this->utils->sig_scan(cherry::offsets::memcheck::sigs::secondary_hashers_sig, cherry::offsets::memcheck::sigs::secondary_hashers_pattern, bounds_text, 0)) {
		bool possible = false;

		if (this->utils->sig_scan(cherry::offsets::memcheck::sigs::secondary_hashers_b1_sig, cherry::offsets::memcheck::sigs::secondary_hashers_pattern, { res, res + 50 }, 2).size() == 1 || this->utils->sig_scan(cherry::offsets::memcheck::sigs::secondary_hashers_b2_sig, cherry::offsets::memcheck::sigs::secondary_hashers_pattern, { res, res + 50 }, 2).size() == 1)
			possible = true;

		if (possible) {
			auto entry = res - (res % 16);

			for (size_t i = 0; i < 5; i++) {
				if (*reinterpret_cast<std::uint8_t*>(entry) == 0x55 && *reinterpret_cast<std::uint8_t*>(entry + 1) == 0x8B && *reinterpret_cast<std::uint8_t*>(entry + 2) == 0xEC) {
					if (this->utils->sig_scan(cherry::offsets::memcheck::sigs::secondary_hashers_b3_sig, cherry::offsets::memcheck::sigs::secondary_hashers_pattern1, { entry, res }, 1).size() && this->utils->sig_scan(cherry::offsets::memcheck::sigs::secondary_hashers_b4_sig, cherry::offsets::memcheck::sigs::secondary_hashers_pattern1, { entry, res }, 1).size() && this->utils->sig_scan("\x8B\x00\x08", cherry::offsets::memcheck::sigs::secondary_hashers_b5_sig, { entry, res }, 1).size()) {
						this->silent_checkers++;
						add_to_list(entry);
						break;
					}
				}

				entry -= 16;
			}
		}
	}
	memcheck_assert((silent_checkers == 16), "Failed to grab the correct amount of silent checks");

	/* job hooking */
	this->scheduler->push([](int state) {
		memcheck_bypass->utils->place_jmp(cherry::offsets::memcheck::core_start, hasher_stub);

		for (size_t i = 0; i < memcheck_bypass->populated_hashes.size(); i++) {
			const auto& hasher_info = memcheck_bypass->populated_hashes[i];

			DWORD old_protection{ 0 };
			VirtualProtect(reinterpret_cast<void*>(hasher_info.entry), 2, PAGE_EXECUTE_READWRITE, &old_protection);

			*reinterpret_cast<std::uint8_t*>(hasher_info.entry) = 0x6A;
			*reinterpret_cast<std::uint8_t*>(hasher_info.entry + 1) = i;

			VirtualProtect(reinterpret_cast<void*>(hasher_info.entry), 2, old_protection, &old_protection);

			memcheck_bypass->utils->place_jmp(hasher_info.entry + 2, silent_sub);
		}

		memcheck_bypass->console->debug_write("BYPASSED!", "MEMCHECK");
	});
}