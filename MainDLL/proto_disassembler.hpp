#include <Windows.h>
#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <winternl.h>
#include <memory>
#include "environment.hpp"

extern __forceinline auto proto_disassemble(Proto* p, std::string proto_name = "") -> std::string;