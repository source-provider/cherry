#include <Windows.h>
#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <winternl.h>
#include <memory>

struct property_descriptor {
	uintptr_t* vftable; // 0
	std::string* name; // 4
	char pad1[24]; // 8
	int isscriptable; // 32
};

struct class_descriptor_t
{
	char padding2[0x4]; // 0
	std::string* class_name; // 4
	char padding1[0x10]; // 8
	std::vector<property_descriptor*> properties; // 24 (adds 12) next is 36
};

struct instance_t
{
	int* vftable; // 0
	std::shared_ptr<instance_t> self; // 4
	class_descriptor_t* class_descriptor; // 12

	int padding0[0x6]; // 16

	std::string* name; // 22
	std::vector<instance_t*>* children; // 26

	int* padding2; // 30

	instance_t* parent; // 34
};

namespace cherry {


	class instance {
	private:
		instance_t* inst = nullptr;
	public:
		instance(int data): inst{ reinterpret_cast<instance_t*>(data) } {}
		instance(instance_t* data) : inst{ data } {}

		__forceinline auto is_empty() -> bool {
			return (this->inst == nullptr);
		}

		__forceinline auto find_first_child_of_class(std::string target_class_name, bool recursive, instance_t* parent = (instance_t*)nullptr) -> cherry::instance {
			instance_t* _instance = parent;

			if (_instance == nullptr)
				_instance = this->inst;

			if (!_instance->children) {
				return instance{ 0 };
			}

			std::vector<instance_t*> children = *(_instance->children);
			int child_num = 0;
			for (instance_t* instance_ : children) {
				if (++child_num % 2) {
					std::string name = *(instance_->class_descriptor->class_name);

					if (name == target_class_name) {
						return instance_;
					}
					else if (recursive) {
						auto child = this->find_first_child_of_class(target_class_name, recursive, instance_);
						return child;
					}
				}
			}

			return instance{ 0 };
		}

		__forceinline auto find_first_child(std::string target_child_name, bool recursive, instance_t* parent = (instance_t*)nullptr) -> cherry::instance {
			instance_t* _instance = parent;

			if (_instance == nullptr)
				_instance = this->inst;

			if (!_instance->children) {
				return instance{ 0 };
			}

			std::vector<instance_t*> children = *(_instance->children);
			int child_num = 0;
			for (instance_t* instance_ : children) {
				if (++child_num % 2) {
					std::string name = *(instance_->name);

					if (name == target_child_name)
						return instance{ instance_ };
					else if (recursive) {
						auto child = this->find_first_child(target_child_name, recursive, instance_);
						return child;
					}
				}
			}

			return instance{ 0 };
		}

		__forceinline auto get_children() -> std::vector<cherry::instance> {
			std::vector<cherry::instance> instt;

			
			if (!this->inst->children)
				return instt;

			std::vector<instance_t*> children = *(this->inst->children);

			int child_num = 0;
			for (instance_t* instance_ : children)
				if (++child_num % 2)
					instt.push_back(cherry::instance{ instance_ });

			return instt;
		}

		__forceinline auto get_full_name() -> std::string {
			instance_t* insta = this->inst->parent;
			std::string output = *(insta->name);

			while (insta->parent) {
				output = std::string(".").append(output.c_str());
				output = std::string(inst->name->c_str()).append(output.c_str());
				insta = insta->parent;
			}

			return output;
		}

	};
}