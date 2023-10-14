#pragma once
#include <iostream>
#include <memory>
#include <windows.h>
#include <string>

//#include <unistd.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <functional>
#include <iostream>
#include <cmath>

namespace cherry {
	namespace memory {



		template <const size_t _UniqueId, typename _Res, typename... _ArgTypes>
		struct fun_ptr_helper
		{
		public:
			typedef std::function<_Res(_ArgTypes...)> function_type;

			static void bind(function_type&& f)
			{
				instance().fn_.swap(f);
			}

			static void bind(const function_type& f)
			{
				instance().fn_ = f;
			}

			static _Res invoke(_ArgTypes... args)
			{
				return instance().fn_(args...);
			}

			typedef decltype(&fun_ptr_helper::invoke) pointer_type;
			static pointer_type ptr()
			{
				return &invoke;
			}

		private:
			static fun_ptr_helper& instance()
			{
				static fun_ptr_helper inst_;
				return inst_;
			}

			fun_ptr_helper() {}

			function_type fn_;
		};

		template <const size_t _UniqueId, typename _Res, typename... _ArgTypes>
		typename fun_ptr_helper<_UniqueId, _Res, _ArgTypes...>::pointer_type
			get_fn_ptr(const std::function<_Res(_ArgTypes...)>& f)
		{
			fun_ptr_helper<_UniqueId, _Res, _ArgTypes...>::bind(f);
			return fun_ptr_helper<_UniqueId, _Res, _ArgTypes...>::ptr();
		}

		template<typename T>
		std::function<typename std::enable_if<std::is_function<T>::value, T>::type>
			make_function(T* t)
		{
			return { t };
		}


		template <typename T = std::uintptr_t> class editor {
		private:
			std::uintptr_t address{ 0 };
			std::size_t size{ 0 };
			std::uint8_t* old_mem{ nullptr };
			T new_mem;
			DWORD old_prot{ 0 };
		public:
			editor(std::uintptr_t address, std::size_t size, T new_mem)
				: address(address), size(size), new_mem(new_mem) {
				this->old_mem = new std::uint8_t[size];
			}

			~editor() {
				delete[] this->old_mem;
			}
		public:
			auto write() -> void {
				std::memcpy(old_mem, reinterpret_cast<void*>(this->address), this->size);

				VirtualProtect(reinterpret_cast<void*>(this->address), this->size, PAGE_EXECUTE_READWRITE, &this->old_prot);
				std::memcpy(reinterpret_cast<void*>(this->address), std::bit_cast<void*>(this->new_mem), this->size);
				VirtualProtect(reinterpret_cast<void*>(this->address), this->size, this->old_prot, &this->old_prot);
			}

			auto restore() -> void {
				VirtualProtect(reinterpret_cast<void*>(this->address), this->size, PAGE_EXECUTE_READWRITE, &this->old_prot);
				std::memcpy(reinterpret_cast<void*>(this->address), this->old_mem, this->size);
				VirtualProtect(reinterpret_cast<void*>(this->address), this->size, this->old_prot, &this->old_prot);
			}

			auto place_jump() -> void {
				std::memcpy(this->old_mem, reinterpret_cast<void*>(this->address), this->size);

				VirtualProtect(reinterpret_cast<void*>(this->address), this->size, PAGE_EXECUTE_READWRITE, &this->old_prot);

				*reinterpret_cast<std::uint8_t*>(this->address) = 0xE9;

				*reinterpret_cast<std::uintptr_t*>(this->address + 1) = std::bit_cast<std::uintptr_t>(this->new_mem) - this->address - 5;

				VirtualProtect(reinterpret_cast<void*>(this->address), this->size, this->old_prot, &this->old_prot);
			}

			auto place_call() -> void {
				std::memcpy(this->old_mem, reinterpret_cast<void*>(this->address), this->size);

				VirtualProtect(reinterpret_cast<void*>(this->address), this->size, PAGE_EXECUTE_READWRITE, &this->old_prot);

				*reinterpret_cast<std::uint8_t*>(this->address) = 0xE8;

				*reinterpret_cast<std::uintptr_t*>(this->address + 1) = std::bit_cast<std::uintptr_t>(this->new_mem) - this->address - 5;

				VirtualProtect(reinterpret_cast<void*>(this->address), this->size, this->old_prot, &this->old_prot);
			}

			static __forceinline auto get_bytes_of_naked_func() -> std::uint8_t* {

			}

			/* mainly used for changing args of pseudo2addr :p (i just wanted a function for it) */
			static __forceinline auto split_and_add_byte(std::intptr_t integer, std::uint8_t byte) -> std::uint8_t* {
				auto split = new std::uint8_t[5];
				split[0] = byte;
				split[1] = integer & 0xFF;
				split[2] = (integer >> 8) & 0xFF;
				split[3] = (integer >> 16) & 0xFF;
				split[4] = (integer >> 24) & 0xFF;

				return split;
			}

			__forceinline auto edit_byte(std::intptr_t memory) -> void {
				auto old_protect = DWORD{ 0 };

				VirtualProtect(reinterpret_cast<void*>(this->address), this->size, PAGE_EXECUTE_READWRITE, &old_protect);
				*reinterpret_cast<std::uint8_t*>(this->address) = memory;
				VirtualProtect(reinterpret_cast<void*>(this->address), this->size, old_protect, &old_protect);
			}
		};

		extern class vftable {
		private:
			std::unique_ptr<int[]> vfclone = nullptr;
			int* vfbackup = nullptr;
			int size = 0;
		public:
			vftable() {}
			vftable(void* instance);

			auto hook(int idx, void* func) -> int;
			auto get_original(int idx) -> int;
			auto revert(int idx) -> void;
		};

	}
}