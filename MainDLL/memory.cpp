#include "memory.hpp"

cherry::memory::vftable::vftable(void* instance) {
	vfbackup = *reinterpret_cast<int**>(instance);

	while (reinterpret_cast<int*>(*reinterpret_cast<int*>(instance))[this->size])
		this->size++;

	const int vt_size = ((this->size * 0x4) + 0x4);
	vfclone = std::make_unique<int[]>(this->size + 1);

	memcpy(vfclone.get(), &vfbackup[-1], vt_size);
	*reinterpret_cast<int**>(instance) = &vfclone.get()[1];
}

auto cherry::memory::vftable::hook(int idx, void* func) -> int {
	if (idx > this->size)
		return 0;

	int original_func = this->vfclone[idx + 1];
	this->vfclone[idx + 1] = reinterpret_cast<int>(func);

	return original_func;
}

auto cherry::memory::vftable::get_original(int idx) -> int {
	if (idx > this->size)
		return 0;

	return (int)(reinterpret_cast<void*>(this->vfbackup[idx]));
}

auto cherry::memory::vftable::revert(int idx) -> void {
	if (idx > this->size)
		return;

	int original_func = this->vfbackup[idx];
	this->vfclone[idx + 1] = original_func;
}