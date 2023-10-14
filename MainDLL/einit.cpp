#include "environment.hpp"
using namespace cherry::global;

std::vector<std::string> queue_on_tp_{};

auto tp_handler_thread() -> void {
	while (true) {
		Sleep(100);
		if (task_scheduler->is_loaded())
			break;
	}

	Sleep(1500);

	task_scheduler->load(true);

	console->clear();
	console->debug_write("TELEPORTED!");

	int state = task_scheduler->state();

	console->debug_write(utilities->tostring_hex(state), "STATE");
	console->debug_write(utilities->tostring_hex(task_scheduler->context()), "CONTEXT");
	console->debug_write(utilities->tostring_hex(task_scheduler->datamodel()), "DATAMODEL");

	cherry::environment::luauopen_register(reinterpret_cast<lua_State*>(state));
}

auto tp_handler(lua_State* L) -> int {
	std::thread(tp_handler_thread).detach();
	return 0;
}

auto cherry::environment::queue_on_tp(std::string code) -> void {
	queue_on_tp_.push_back(code);
}

std::vector<const char*> dangerous_functions = {
	"OpenVideosFolder", "OpenScreenshotsFolder",
	"GetRobuxBalance", "PerformPurchase", "PromptBundlePurchase", "PromptNativePurchase", "PromptProductPurchase", "PromptPurchase", "PromptThirdPartyPurchase",
	"Publish", "GetMessageId", "OpenBrowserWindow", "RequestInternal", "ExecuteJavaScript",

};

auto dangerous_function(lua_State* L) -> int {
	luaL_error(L, "Dangerous function is not supported for security reasons.");
	return 0;
}

int old_namecall;
auto namecall_hook(lua_State* L) -> int {
	if (L->namecall) {
		const char* data = L->namecall->data;

		if (!strcmp(data, "HttpGet") || !strcmp(data, "HttpGetAsync")) {
			return cherry::environment::httpget(L);
		}

		if (!strcmp(data, "HttpPost") || !strcmp(data, "HttpPostAsync")) {
			return cherry::environment::httppost(L);
		}

		if (!strcmp(data, "GetObjects") || !strcmp(data, "GetObjectsAsync")) {
			return cherry::environment::getobjects(L);
		}

		for (unsigned int i = 0; i < dangerous_functions.size(); i++) {
			if (!strcmp(data, dangerous_functions[i])) {
				luaL_error(L, "Dangerous function is not supported for security reasons.");
				return 0;
			}
		}
	}

	return reinterpret_cast<int(__cdecl*)(int)>(old_namecall)((int)L);
}

int old_index;
auto index_hook(lua_State* L) -> int {
	if (lua_isstring(L, 2))
	{
		const char* data = lua_tostring(L, 2);

		if (!strcmp(data, "HttpGet") || !strcmp(data, "HttpGetAsync")) {
			lua_getglobal(L, "httpget");
			return 1;
		}

		if (!strcmp(data, "HttpPost") || !strcmp(data, "HttpPostAsync")) {
			lua_getglobal(L, "httppost");
			return 1;
		}

		if (!strcmp(data, "GetObjects") || !strcmp(data, "GetObjectsAsync")) {
			lua_getglobal(L, "getobjects");
			return 1;
		}

		for (unsigned int i = 0; i < dangerous_functions.size(); i++) {
			if (!strcmp(data, dangerous_functions[i])) {
				lua_pushcclosure(L, dangerous_function, NULL, 0);
				return 1;
			}
		}
	}

	return reinterpret_cast<int(__cdecl*)(int)>(old_index)((int)L);
}

auto cherry::environment::luauopen_initscript(lua_State* L) -> void {
	std::string init_script = R"(
--< variables >--
local getinstancelist = clonefunction(getinstancelist)
local getgenv = clonefunction(getgenv)
local getrawmetatable = clonefunction(getrawmetatable)
local rawget = clonefunction(rawget)
local rawset = clonefunction(rawset)
local setreadonly = clonefunction(setreadonly)
local assert = clonefunction(assert)
local typeof = clonefunction(typeof)
local islclosure = clonefunction(islclosure)
local o_require = clonefunction(require)
local getidentity = clonefunction(getidentity)
local setidentity = clonefunction(setidentity)
local error = clonefunction(error)
local warn = clonefunction(warn)
local iscclosure = clonefunction(iscclosure)
local rawequal = clonefunction(rawequal)

getgenv().getinstancelist = nil
getgenv().run_on_corescript = nil
--< source >--
getgenv().balls = newcclosure(function() -- this is just a throwaway function due to newcclosure breaking on its first closure
	print("balls")
	return "balls"
end)

local function format_type_error(func_name, arg, type_wanted, val)
	return ([[bad argument %s to '%s' (expected %s, got %s)]]):format(arg, func_name, type_wanted, typeof(val))
end

local gfenv = getfenv
getgenv().getfenv = newcclosure(function(lvl)
	lvl = lvl or 0
	if (rawequal(lvl, 0)) then 
		return getgenv()
	end

	return gfenv(lvl)
end)

getgenv().newlclosure = newcclosure(function(closure)
	assert(typeof(closure) == "function", format_type_error("newlclosure", 1, "function", closure))

	if (islclosure(closure)) then 
		return closure
	end

	return function(...) 
		return closure(...)
	end
end)

getgenv().secure_call = newcclosure(function(func, fake_env, ...) 
    local olds = getfenv().script
    getfenv().script = fakeEnv

    local oldi = getidentity()
    setidentity(2)

    local data = { pcall(func, ...) }

    getfenv().script = olds
    setidentity(oldi)

    if not data[1] then
        warn(data[2])
    else
        data[1] = nil
        return unpack(data)
    end
end)

local hookf = hookfunction;
getgenv().hookfunction = newcclosure(function(hookto, hookwith)
	if (iscclosure(hookto)) then 
		if (islclosure(hookwith)) then 
			hookwith = newcclosure(hookwith)
		end
	else 
		if (iscclosure(hookwith)) then 
			hookwith = newlclosure(hookwith)
		end
	end

	return hookf(hookto, hookwith);
end)

getgenv().require = newcclosure(function(module)
	local identity = getidentity()
	setidentity(2)

	local data = { pcall(o_require, module) }
	setidentity(identity)

	if (data[1] == false) then 
		error(data[2], 0)
		return nil;
	end
	
	return data[2]
end)

getgenv().getinstances = newcclosure(function()
    local instances = {}

	local idx = 1
    for i,v in getinstancelist() do
        if (typeof(v) == "Instance") then 
            rawset(instances, idx, v)
			idx += 1
        end
    end

    return instances
end)

getgenv().getnilinstances = newcclosure(function()
    local instances = {}

	local idx = 1
    for i,v in getinstancelist() do
        if (typeof(v) == "Instance" and v.Parent == nil) then 
            rawset(instances, idx, v)
			idx += 1
        end
    end

    return instances
end)

getgenv().getscripts = newcclosure(function()
    local instances = {}

	local idx = 1
    for i, v in getinstancelist() do
        if (typeof(v) == "Instance" and (v:IsA("ModuleScript") or v:IsA("LocalScript"))) then 
            rawset(instances, idx, v)
			idx += 1
        end
    end

    return instances
end)
getgenv().getrunningscripts = getgenv().getscripts

getgenv().getloadedmodules = newcclosure(function()
    local instances = {}

	local idx = 1
    for i,v : Instance in getinstancelist() do
        if (typeof(v) == "Instance" and v:IsA("ModuleScript")) then 
            rawset(instances, idx, v)
			idx += 1
        end
    end

    return instances
end)

local function lookupify(t)
    local _t = {}
    for k, v in t do
        _t[v] = true
    end
    return _t
end

getgenv().gethiddenproperty = newcclosure(function(inst, idx) 
	assert(typeof(script) == "Instance", "invalid argument #1 to 'gethiddenproperty' [Instance expected]", 2);
	local was = isscriptable(inst, idx);
	setscriptable(inst, idx, true)

	local value = inst[idx]
	setscriptable(inst, idx, was)

	return value, not was
end)

getgenv().sethiddenproperty = newcclosure(function(inst, idx, value) 
	assert(typeof(script) == "Instance", "invalid argument #1 to 'sethiddenproperty' [Instance expected]", 2);
	local was = isscriptable(inst, idx);
	setscriptable(inst, idx, true)

	inst[idx] = value

	setscriptable(inst, idx, was)

	return not was
end)

getgenv().getsenv = newcclosure(function(script) 
	assert(typeof(script) == "Instance", "invalid argument #1 to 'getsenv' [ModuleScript or LocalScript expected]", 2);
	assert((script:IsA("LocalScript") or script:IsA("ModuleScript")), "invalid argument #1 to 'getsenv' [ModuleScript or LocalScript expected]", 2)
	if (script:IsA("LocalScript") == true) then 
		for i,v in getreg() do
			if (type(v) == "function") then
				if getfenv(v).script then
					if getfenv(v).script == script then
						return getfenv(v)
					end
				end
			end
		end
	else
		local A = getreg()
		local B = {}

		if #A == 0 then
			return require(script)
		end

		for C, D in next, A do
			if type(D) == "function" and islclosure(D) then
				local E = getfenv(D)
				local F = rawget(E, "script")
				if F and F == script then
					for G, H in next, E do
						if G ~= "script" then
							rawset(B, G, H)
						end
					end
				end
			end
		end
		return B
	end
end)
getgenv().getmenv = getgenv().getsenv;

local allowed_methods = lookupify{ "__index", "__namecall", "__newindex", "__call", "__concat", "__unm", "__add", "__sub", "__mul", "__div", "__pow", "__mod", "__tostring", "__eq", "__lt", "__le", "__gc", "__len" }
getgenv().hookmetamethod = newcclosure(function(ud, method, fn)
    assert(ud ~= nil, 'invalid argument #1 (object expected)', 0)
    assert(typeof(method) == "string", 'invalid argument #2 (string expected)', 0)
    assert(typeof(fn) == "function", 'invalid argument #3 (function expected)', 0)
    assert(allowed_methods[method], 'invalid argument #2 (function mode expected)', 0)

    local gmt = getrawmetatable(ud)
    local old_fn = rawget(gmt, method)

    if (old_fn == nil and type(old_fn) ~= "function") then 
		return 
	end

    setreadonly(gmt, false)
    rawset(gmt, method, (islclosure(fn) and newcclosure(fn) or fn))
    setreadonly(gmt, true)
    return old_fn
end)


)";

	std::string disassembler = R"(
local function dissect_import(id, k) 
    -- Import IDs have the top two bits as the length of the chain, and then 3 10-bit fields of constant string indices 
    local count = bit32.extract(id, 30, 2) 

    local k0 = k[bit32.extract(id, 20, 10) + 1] 
    local k1 = count > 1 and k[bit32.extract(id, 10, 10) + 1] 
    local k2 = count > 2 and k[bit32.extract(id, 0, 10) + 1] 

    local displayString = k0 
    if k1 then 
            displayString ..= "." .. k1 
            if k2 then 
                    displayString ..= "." .. k2 
            end 
    end 

    return { 
            ["count"] = count, 
            ["displayString"] = displayString, 
    } 
end

local rnd = Random.new()
local function random_string(length)
    local characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890_"
    local out = ""

    for i=1, length, 1 do 
        local num = rnd:NextInteger(1, string.len(characters))
        out = out .. string.sub(characters, num, num)
    end

    return out
end

local function deserialize(bytecode)
    -- LBC_VERSION_FUTURE includes linedefined for protos
    local LBC_VERSION_FUTURE = 3

    -- Current read position in the bytecode string 
    local offset = 1

    local function readByte()
            local number = string.unpack("B", bytecode, offset)
            offset += 1
            return number
    end

    local function readLEB128()
            local l_readByte = readByte -- Stored in a local so the upvalue doesn't need to be grabbed multiple times

            local result = 0
            local b = 0 -- amount of bits to shift
            local c

            repeat
                    c = l_readByte() 
                    local c2 = bit32.band(c, 0x7F)
                    result = bit32.bor(result, bit32.lshift(c2, b))
                    b += 7
            until not bit32.btest(c, 0x80)

            return result
    end

    local function readUInt32()
            local number = string.unpack("<I", bytecode, offset)
            offset += 4
            return number
    end

    local function readFloat64() 
            local number = string.unpack("<d", bytecode, offset)
            offset += 8
            return number
    end

    local function readLengthPrefixedString()
            local length = readLEB128()
            local str = string.unpack("c" .. length, bytecode, offset)
            offset += length
            return str
    end

	local OR, XOR, AND = 1, 3, 4

	local function bitoper(a, b, oper)
		local r, m, s = 0, 2^31
		repeat
			s,a,b = a+b+m, a%m, b%m
			r,m = r + m*oper%(s-a-b), m/2
		until m < 1
		return r
	end

    local bytecodeVersion = readByte()
    assert(bytecodeVersion ~= 0, "Cannot deserialize bytecode that is a compilation error")
    assert(
            bytecodeVersion <= LBC_VERSION_FUTURE,
            "Invalid bytecode version (must be 1 - 3, got " .. bytecodeVersion ..  ')'
    )

    local stringCount = readLEB128()
    local stringTable = table.create(stringCount)
    if stringCount > 0 then
            for stringIdx = 1, stringCount do
                    stringTable[stringIdx] = readLengthPrefixedString()
            end
    end

    local protoCount = readLEB128()
    local protoTable = table.create(protoCount)
    for protoIdx = 1, protoCount do
            local proto = {}

            proto.maxstacksize = readByte()
            proto.numparams = readByte()
            proto.nups = readByte()
            proto.is_vararg = readByte()

            proto.sizecode = readLEB128()
            proto.code = table.create(proto.sizecode)
            for codeIdx = 1, proto.sizecode do
                proto.code[codeIdx] = readUInt32()
            end

            proto.sizek = readLEB128()
            proto.k = table.create(proto.sizek)
            for kIdx = 1, proto.sizek do
                    local kType = readByte()

                    if kType == 0 then -- nil
                            proto.k[kIdx] = nil
                    elseif kType == 1 then -- boolean
                            proto.k[kIdx] = readByte() ~= 0
                    elseif kType == 2 then -- number
                            proto.k[kIdx] = readFloat64()
                    elseif kType == 3 then -- string
                            proto.k[kIdx] = stringTable[readLEB128()]
                    elseif kType == 4 then -- import
                            proto.k[kIdx] = dissect_import(readUInt32(), proto.k)
                    elseif kType == 5 then -- table
                            for _ = 1, readLEB128() do
                                    readLEB128()
                            end 
                    elseif kType == 6 then -- closure
                            proto.k[kIdx] = readLEB128() -- proto id
                    else
                            error("Unexpected constant type: " .. kType .. " is not a recognized type")
                    end
            end

            proto.sizep = readLEB128()
            proto.p = table.create(proto.sizep)
            for innerProtoIdx = 1, proto.sizep do
                    proto.p[innerProtoIdx] = readLEB128()
            end

            if bytecodeVersion == LBC_VERSION_FUTURE then
                    proto.linedefined = readLEB128()
            end

            local debugNameId = readLEB128()
            if debugNameId ~= 0 then
                    proto.debugname = stringTable[debugNameId]
            end

            if readByte() ~= 0 then -- lineinfo?
                    proto.linegaplog2 = readByte()


                    local intervals = bit32.rshift(proto.sizecode - 1, proto.linegaplog2) + 1
					local absoffset = bitoper((proto.sizecode + 3), bit32.bnot(intervals), AND)
					proto.sizelineinfo = absoffset + intervals * 4; -- 4 is the sizeof(int)
					proto.lineinfo = table.create(proto.sizelineinfo)
					proto.abslineinfo = {}


					local last_offset = 0;
                    for j = 1, proto.sizecode do
                        last_offset += readByte()
						proto.lineinfo[j] = last_offset;
                    end

					local last_line = 0;
                    for j = 1, intervals do
						last_line += readByte()
						proto.abslineinfo[j] = last_line
						--print("last_line", last_line)
                        readByte()
                        readByte()
                        readByte()
                    end
            end

            if readByte() ~= 0 then -- debuginfo? 
                    proto.sizelocvars = readLEB128() 
                    for _ = 1, proto.sizelocvars do 
                        readLEB128()
                        readLEB128()
                        readLEB128()
                        readByte()
                    end

                    proto.sizeupvalues = readLEB128()
                    for _ = 1, proto.sizeupvalues do
                        readLEB128()
                    end
            end

			proto.pc = 1
            protoTable[protoIdx] = proto
    end

    local mainId = readLEB128()

    return protoTable, mainId
end

local function uint16_to_signed(n) 
    local sign = bit32.btest(n, 0x8000)
    n = bit32.band(n, 0x7FFF)

    if sign then
            return n - 0x8000
    else
            return n
	end
end

local function uint24_to_signed(n) 
    local sign = bit32.btest(n, 0x800000)
    n = bit32.band(n, 0x7FFFFF)

    if sign then
            return n - 0x800000
    else
            return n
    end
end

local function get_opcode(insn)
    return bit32.band(insn, 0xFF)
end

local function get_arga(insn)
    local s1, e1 =  pcall(bit32.rshift, insn, 8)

    if s1 then
        local s, e = pcall(bit32.band, e1, 0xFF)
        if s then
            return e;
        end
    end

    return 0;
end
local function get_argb(insn)
    local s1, e1 =  pcall(bit32.rshift, insn, 16)

    if s1 then
        local s, e = pcall(bit32.band, e1, 0xFF)
        if s then
            return e;
        end
    end

    return 0;
end

local function get_argc(insn)
    local s1, e1 =  pcall(bit32.rshift, insn, 24)
    return (s1 and e1 or 0)
end

local function get_argd(insn)
    return uint16_to_signed(bit32.rshift(insn, 16))
end

local function get_arge(insn) 
    return uint24_to_signed(bit32.rshift(insn, 8))
end

local function getConstantString(k) 
        if (type(k) == "string") then
		local res = "\""

		if (k == "\"") then 
			res ..= "\\\""
		elseif (k == "\\") then
			res ..= "\\\\"
		elseif (k == "\a") then
			res ..= "\\a"
		elseif (k == "\b") then
			res ..= "\\b"
		elseif (k == "\f") then
			res ..= "\\f"
		elseif (k == "\n") then
			res ..= "\\n"
		elseif (k == "\r") then
			res ..= "\\r"
		elseif (k == "\t") then
			res ..= "\\t"
		elseif (k == "\v") then
			res ..= "\\v"
		else
			k = k:gsub("\"", "\\\"")
			k = k:gsub("\\", "\\\\")
			k = k:gsub("\a", "\\a")
			k = k:gsub("\b", "\\b")
			k = k:gsub("\f", "\\f")
			k = k:gsub("\n", "\\n")
			k = k:gsub("\r", "\\r")
			k = k:gsub("\t", "\\t")
			k = k:gsub("\v", "\\v")

			res ..= k
		end

		return (res .. "\"")
	elseif (type(k) == "number") then 
		k = string.format("%4.3f", k)
	elseif (type(k) == "boolean") then
		return (k == true and "true" or "false")
	end

	return k
end 

local dissassemble_t = (function(bytecodeString)
    bytecodeString = getscriptbytecode(bytecodeString)
    --assert(type(bytecodeString) == "string", "Argument #1 to `disassemble` must be a string") 

    local CAPTURE_TYPES = { 
            [0] = "VAL", 
            [1] = "REF", 
            [2] = "UPVAL", 
    } 

    --local output = table.create(#bytecodeString * 6) 
    local output = {} 

    local protoTable = deserialize(bytecodeString) 

    for protoId, proto in ipairs(protoTable) do 
            table.insert(output, "; bytecode id: " .. (protoId - 1) .. "\n")

            if (proto.debugname) then 
                table.insert(output, "; proto name: " .. proto.debugname .. (proto.linedefined and "\n" or "\n;\n"))
            else
                table.insert(output, "; proto random name: " .. random_string(16) .. (proto.linedefined and "\n" or "\n;\n"))
            end

            if proto.linedefined then 
                table.insert(output, "; line defined: " .. proto.linedefined .. "\n;\n") 
            end 

            table.insert(output, "; maxstacksize: " .. proto.maxstacksize .. "\n")
            table.insert(output, "; numparams: " .. proto.numparams .. "\n")
            table.insert(output, "; nups: " .. proto.nups .. "\n")
            table.insert(output, "; is_vararg: " .. proto.is_vararg .. "\n;\n")

            if #proto.p > 0 then 
                    table.insert(output, "; child protos: " .. table.concat(proto.p, ", ") .. "\n;\n") 
            end 

            table.insert(output, "; sizecode: " .. proto.sizecode .. "\n") 
            table.insert(output, "; sizek: " .. proto.sizek .. "\n;\n") 

            local pc = 1 
            while pc <= proto.sizecode do 
                    table.insert(output, string.format("[%03i] ", pc - 1)) 

                    local insn = proto.code[pc] 
                    local opcode = get_opcode(insn) 

                    if opcode == 0x00 then -- NOOP 
                            table.insert(output, string.format("NOOP (%#010x)\n", insn)) 
                    elseif opcode == 0xE3 then -- BREAK 
                            table.insert(output, "BREAK\n") 
                    elseif opcode == 0xC6 then -- LOADNIL 
                            table.insert(output, "LOADNIL " .. get_arga(insn) .. "\n") 
                    elseif opcode == 0xA9 then -- LOADB
                            local targetRegister = get_arga(insn) 
                            local boolValue = get_argb(insn) 
                            local jumpOffset = get_argc(insn) 

                            if jumpOffset > 0 then 
                                    table.insert(output, string.format( 
                                            "LOADB %i %i %i ; %s, jump to %i\n", 
                                            targetRegister, 
                                            boolValue, 
                                            jumpOffset, 
                                            boolValue ~= 0 and "true" or "false", 
                                            pc + jumpOffset 
                                    )) 
                            else 
                                    table.insert(output, string.format( 
                                            "LOADB %i %i ; %s\n", 
                                            targetRegister, 
                                            boolValue, 
                                            boolValue ~= 0 and "true" or "false" 
                                    )) 
                            end 
                    elseif opcode == 0x8C then -- LOADN 
                            table.insert(output, string.format( 
                                    "LOADN %i %i\n", 
                                    get_arga(insn), 
                                    get_argd(insn) 
                            )) 
                    elseif opcode == 0x6F then -- LOADK 
                            local constantIndex = get_argd(insn) 
                            local constant = proto.k[constantIndex + 1] 
                            local constantString = getConstantString(constant)

                            table.insert(output, string.format( 
                                    "LOADK %i %i ; K(%i) = %s\n", 
                                    tostring(get_arga(insn)), 
                                    tostring(constantIndex), 
                                    tostring(constantIndex), 
                                    tostring(constantString) 
                            )) 
                    elseif opcode == 0x52 then -- MOVE 
                            table.insert(output, string.format( 
                                    "MOVE %i %i\n", 
                                    get_arga(insn), 
                                    get_argd(insn) 
                            )) 
                    elseif opcode == 0x35 then -- GETGLOBAL 
                            local target = get_arga(insn)
                            pc += 1 
                            local aux = proto.code[pc]
                            table.insert(output, string.format( 
                                    "GETGLOBAL %i %i ; K(%i) = '%s'\n", 
                                    target, 
                                    aux, 
                                    aux, 
                                    proto.k[aux + 1]
                            )) 
                    elseif opcode == 0x18 then -- SETGLOBAL 
                            local source = get_arga(insn)
                            pc += 1
                            local aux = proto.code[pc]
                            table.insert(output, string.format( 
                                    "SETGLOBAL %i %i ; K(%i) = '%s'\n", 
                                    source, 
                                    aux, 
                                    aux, 
                                    proto.k[aux + 1] 
                            )) 
                    elseif opcode == 0xFB then -- GETUPVAL 
                            table.insert(output, string.format( 
                                    "GETUPVAL %i %i\n", 
                                    get_arga(insn),
                                    get_argb(insn)
                            )) 
                    elseif opcode == 0xDE then -- SETUPVAL 
                            table.insert(output, string.format( 
                                    "SETUPVAL %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn) 
                            )) 
                    elseif opcode == 0xC1 then -- CLOSEUPVALS 
                            table.insert(output, "CLOSEUPVALS " .. get_arga(insn) .. "\n") 
)";

            disassembler += R"(
                   elseif opcode == 0xA4 then -- GETIMPORT 
                            local target = get_arga(insn)
                            local importId = get_argd(insn)
                            pc += 1 -- skip aux
                            local import = proto.k[importId + 1]
                            table.insert(output, string.format( 
                                    "GETIMPORT %i %i ; count = %i: %s\n", 
                                    target,
                                    importId,
                                    import.count,
                                    import.displayString
                            )) 
                    elseif opcode == 0x87 then -- GETTABLE 
                            table.insert(output, string.format( 
                                    "GETTABLE %i %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    get_argc(insn) 
                            )) 
                    elseif opcode == 0x6A then -- SETTABLE 
                            table.insert(output, string.format( 
                                    "SETTABLE %i %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    get_argc(insn) 
                            )) 
                    elseif opcode == 0x4D then -- GETTABLEKS 
                            local targetRegister = get_arga(insn)
                            local tableRegister = get_argb(insn)
                            pc += 1
                            local aux = proto.code[pc]
                            table.insert(output, string.format( 
                                    "GETTABLEKS %i %i %i ; K(%i) : %s\n", 
                                    targetRegister,
                                    tableRegister,
                                    aux,
                                    aux,
                                    proto.k[aux + 1]
                            ))
                    elseif opcode == 0x30 then -- SETTABLEKS 
                            local sourceRegister = get_arga(insn)
                            local tableRegister = get_argb(insn)
                            pc += 1
                            local aux = proto.code[pc]
                            table.insert(output, string.format( 
                                    "SETTABLEKS %i %i %i ; K(%i) = '%s'\n", 
                                    sourceRegister, 
                                    tableRegister, 
                                    aux, 
                                    aux, 
                                    proto.k[aux + 1] 
                            )) 
                    elseif opcode == 0x13 then -- GETTABLEN 
                            local argc = get_argc(insn) 
                            table.insert(output, string.format( 
                                    "GETTABLEN %i %i %i ; index = %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    argc, 
                                    argc + 1 
                            )) 
                    elseif opcode == 0xF6 then -- SETTABLEN 
                            local argc = get_argc(insn) 
                            table.insert(output, string.format( 
                                    "SETTABLEN %i %i %i ; index = %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    argc, 
                                    argc + 1 
                            )) 
                    elseif opcode == 0xD9 then -- NEWCLOSURE 
                            local childProtoId = get_argd(insn) 
                            table.insert(output, string.format( 
                                    "NEWCLOSURE %i %i ; global id = %i\n", 
                                    get_arga(insn),
                                    childProtoId,
                                    proto.p[childProtoId + 1]
                            )) 
                    elseif opcode == 0xBC then -- NAMECALL 
                            local targetRegister = get_arga(insn) 
                            local sourceRegister = get_argb(insn) 
                            pc += 1 
                            local aux = proto.code[pc] 
                            table.insert(output, string.format( 
                                    "NAMECALL %i %i %i ; K(%i) = '%s'\n", 
                                    targetRegister, 
                                    sourceRegister, 
                                    aux,
                                    aux,
                                    proto.k[aux + 1] 
                            )) 
                    elseif opcode == 0x9F then -- CALL 
                            local nargs = get_argb(insn) 
                            local nresults = get_argc(insn) 

                            table.insert(output, string.format( 
                                    "CALL %i %i %i ; %s arguments, %s results\n", 
                                    get_arga(insn), 
                                    nargs, 
                                    nresults, 
                                    nargs ~= 0 and tostring(nargs - 1) or "MULTRET", 
                                    nresults ~= 0 and tostring(nresults - 1) or "MULTRET" 
                            )) 
                    elseif opcode == 0x82 then -- RETURN 
                            local arga = get_arga(insn) 
                            local argb = get_argb(insn) 
                            table.insert(output, string.format( 
                                    "RETURN %i %i ; values start at %i, num returned values = %s\n", 
                                    arga, 
                                    argb, 
                                    arga, 
                                    argb ~= 0 and tostring(argb - 1) or "MULTRET" 
                            )) 
                    elseif opcode == 0x65 then -- JUMP 
                            local offset = get_argd(insn)
                            table.insert(output, string.format( 
                                    "JUMP %i ; to %i\n", 
                                    offset, 
                                    pc + offset
                            )) 
                    elseif opcode == 0x48 then -- JUMPBACK 
                            local offset = get_argd(insn) 
                            table.insert(output, string.format( 
                                    "JUMPBACK %i ; to %i\n", 
                                    offset, 
                                    pc + offset 
                            )) 
                    elseif opcode == 0x2B then -- JUMPIF 
                            local sourceRegister = get_arga(insn) 
                            local offset = get_argd(insn) 
                            table.insert(output, string.format( 
                                    "JUMPIF %i %i ; to %i\n", 
                                    sourceRegister, 
                                    offset, 
                                    pc + offset 
                            )) 
                    elseif opcode == 0x0E then -- JUMPIFNOT 
                            local sourceRegister = get_arga(insn) 
                            local offset = get_argd(insn) 
                            table.insert(output, string.format( 
                                    "JUMPIFNOT %i %i ; to %i\n", 
                                    sourceRegister, 
                                    offset, 
                                    pc + offset 
                            )) 
                    elseif opcode == 0xF1 then -- JUMPIFEQ 
                            local register1 = get_arga(insn) 
                            local offset = get_argd(insn) 
                            local jumpTo = pc + offset 
                            pc += 1 
                            local aux = proto.code[pc] 
                            table.insert(output, string.format( 
                                    "JUMPIFEQ %i %i %i ; to %i\n", 
                                    register1, 
                                    aux, 
                                    offset, 
                                    jumpTo 
                            )) 
                    elseif opcode == 0xD4 then -- JUMPIFLE 
                            local register1 = get_arga(insn) 
                            local offset = get_argd(insn) 
                            local jumpTo = pc + offset 
                            pc += 1 
                            local aux = proto.code[pc] 
                            table.insert(output, string.format( 
                                    "JUMPIFLE %i %i %i ; to %i\n", 
                                    register1, 
                                    aux, 
                                    offset, 
                                    jumpTo 
                            )) 
                    elseif opcode == 0xB7 then -- JUMPIFLT 
                            local register1 = get_arga(insn) 
                            local offset = get_argd(insn) 
                            local jumpTo = pc + offset 
                            pc += 1 
                            local aux = proto.code[pc] 
                            table.insert(output, string.format( 
                                    "JUMPIFLT %i %i %i ; to %i\n", 
                                    register1, 
                                    aux, 
                                    offset, 
                                    jumpTo 
                            )) 
                    elseif opcode == 0x9A then -- JUMPIFNOTEQ 
                            local register1 = get_arga(insn) 
                            local offset = get_argd(insn) 
                            local jumpTo = pc + offset 
                            pc += 1 
                            local aux = proto.code[pc] 
                            table.insert(output, string.format( 
                                    "JUMPIFNOTEQ %i %i %i ; to %i\n", 
                                    register1, 
                                    aux, 
                                    offset, 
                                    jumpTo 
                            )) 
                    elseif opcode == 0x7D then -- JUMPIFNOTLE 
                            local register1 = get_arga(insn) 
                            local offset = get_argd(insn) 
                            local jumpTo = pc + offset 
                            pc += 1 
                            local aux = proto.code[pc] 
                            table.insert(output, string.format( 
                                    "JUMPIFNOTLE %i %i %i ; to %i\n", 
                                    register1, 
                                    aux, 
                                    offset, 
                                    jumpTo 
                            )) 
                    elseif opcode == 0x60 then -- JUMPIFNOTLT 
                            local register1 = get_arga(insn) 
                            local offset = get_argd(insn) 
                            local jumpTo = pc + offset 
                            pc += 1 
                            local aux = proto.code[pc] 
                            table.insert(output, string.format( 
                                    "JUMPIFNOTLT %i %i %i ; to %i\n", 
                                    register1, 
                                    aux, 
                                    offset, 
                                    jumpTo 
                            )) 
                    elseif opcode == 0x43 then -- ADD 
                            table.insert(output, string.format( 
                                    "ADD %i %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    get_argc(insn) 
                            )) 
                    elseif opcode == 0x26 then -- SUB 
                            table.insert(output, string.format( 
                                    "SUB %i %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    get_argc(insn) 
                            )) 
                    elseif opcode == 0x09 then -- MUL 
                            table.insert(output, string.format( 
                                    "MUL %i %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    get_argc(insn) 
                            )) 
                    elseif opcode == 0xEC then -- DIV 
                            table.insert(output, string.format( 
                                    "DIV %i %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    get_argc(insn) 
                            )) 
                    elseif opcode == 0xCF then -- MOD 
                            table.insert(output, string.format( 
                                    "MOD %i %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    get_argc(insn) 
                            )) 
                    elseif opcode == 0xB2 then -- POW 
                            table.insert(output, string.format( 
                                    "POW %i %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    get_argc(insn) 
                            )) 
                    elseif opcode == 0x95 then -- ADDK 
                            local constantIndex = get_argc(insn) 
                            local constantValue = proto.k[constantIndex + 1] 
                            table.insert(output, string.format( 
                                    "ADDK %i %i %i ; K(%i) = %4.3f\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    constantIndex, 
                                    constantIndex, 
                                    constantValue 
                            )) 
                    elseif opcode == 0x78 then -- SUBK 
                            local constantIndex = get_argc(insn) 
                            local constantValue = proto.k[constantIndex + 1] 
                            table.insert(output, string.format( 
                                    "SUBK %i %i %i ; K(%i) = %4.3f\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    constantIndex, 
                                    constantIndex, 
                                    constantValue 
                            )) 
                    elseif opcode == 0x5B then -- MULK 
                            local constantIndex = get_argc(insn) 
                            local constantValue = proto.k[constantIndex + 1] 
                            table.insert(output, string.format( 
                                    "MULK %i %i %i ; K(%i) = %4.3f\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    constantIndex, 
                                    constantIndex, 
                                    constantValue 
                            )) 
                    elseif opcode == 0x3E then -- DIVK 
                            local constantIndex = get_argc(insn) 
                            local constantValue = proto.k[constantIndex + 1] 
                            table.insert(output, string.format( 
)";

                                    disassembler += R"(
                                    "DIVK %i %i %i ; K(%i) = %4.3f\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    constantIndex, 
                                    constantIndex, 
                                    constantValue 
                            )) 
                    elseif opcode == 0x21 then -- MODK 
                            local constantIndex = get_argc(insn) 
                            local constantValue = proto.k[constantIndex + 1] 
                            table.insert(output, string.format( 
                                    "MODK %i %i %i ; K(%i) = %4.3f\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    constantIndex, 
                                    constantIndex, 
                                    constantValue 
                            )) 
                    elseif opcode == 0x04 then -- POWK 
                            local constantIndex = get_argc(insn) 
                            local constantValue = (tonumber(proto.k[constantIndex + 1]) or 0)
                            table.insert(output, string.format( 
                                    "POWK %i %i %i ; K(%i) = %4.3f\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    constantIndex, 
                                    constantIndex, 
                                    constantValue 
                            )) 
                    elseif opcode == 0xE7 then -- AND 
                            table.insert(output, string.format( 
                                    "AND %i %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    get_argc(insn) 
                            )) 
                    elseif opcode == 0xCA then -- OR 
                            table.insert(output, string.format( 
                                    "OR %i %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    get_argc(insn) 
                            )) 
                    elseif opcode == 0xAD then -- ANDK 
                            local constantIndex = get_argc(insn) 
                            table.insert(output, string.format( 
                                    "ANDK %i %i %i ; K(%i) = %s\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    constantIndex, 
                                    constantIndex, 
                                    getConstantString(proto.k[constantIndex + 1]) 
                            )) 
                    elseif opcode == 0x90 then -- ORK 
                            local constantIndex = get_argc(insn) 
                            table.insert(output, string.format( 
                                    "ORK %i %i %i ; K(%i) = %s\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    constantIndex, 
                                    constantIndex, 
                                    getConstantString(proto.k[constantIndex + 1]) 
                            )) 
                    elseif opcode == 0x73 then -- CONCAT 
                            table.insert(output, string.format( 
                                    "CONCAT %i %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    get_argc(insn) 
                            )) 
                    elseif opcode == 0x56 then -- NOT 
                            table.insert(output, string.format( 
                                    "NOT %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn) 
                            )) 
                    elseif opcode == 0x39 then -- MINUS 
                            table.insert(output, string.format( 
                                    "MINUS %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn) 
                            )) 
                    elseif opcode == 0x1C then -- LENGTH 
                            table.insert(output, string.format( 
                                    "LENGTH %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn) 
                            )) 
                    elseif opcode == 0xFF then -- NEWTABLE 
                            pc += 1 
                            local aux = proto.code[pc] 
                            table.insert(output, string.format( 
                                    "NEWTABLE %i %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    aux 
                            )) 
                    elseif opcode == 0xE2 then -- DUPTABLE 
                            table.insert(output, string.format( 
                                    "DUPTABLE %i %i\n", 
                                    get_arga(insn), 
                                    get_argd(insn) 
                            )) 
                    elseif opcode == 0xC5 then -- SETLIST 
                            local sourceStart = get_argb(insn) 
                            local argc = get_argc(insn) 
                            pc += 1 
                            local aux = proto.code[pc] 
                            table.insert(output, string.format( 
                                    "SETLIST %i %i %i %i ; start at register %i, fill %s values, start at table index %i\n", 
                                    get_arga(insn), 
                                    sourceStart, 
                                    argc, 
                                    aux, 
                                    sourceStart, 
                                    argc ~= 0 and tostring(argc - 1) or "MULTRET", 
                                    aux 
                            )) 
                    elseif opcode == 0xA8 then -- FORNPREP 
                            local jumpOffset = get_argd(insn) 
                            table.insert(output, string.format( 
                                    "FORNPREP %i %i ; to %i\n", 
                                    get_arga(insn), 
                                    jumpOffset, 
                                    pc + jumpOffset 
                            )) 
                    elseif opcode == 0x8B then -- FORNLOOP 
                            local jumpOffset = get_argd(insn) 
                            table.insert(output, string.format( 
                                    "FORNLOOP %i %i ; to %i\n", 
                                    get_arga(insn), 
                                    jumpOffset, 
                                    pc + jumpOffset 
                            )) 
                    elseif opcode == 0x51 then -- FORGPREP_INEXT 
                            local jumpOffset = get_argd(insn) 
                            table.insert(output, string.format( 
                                    "FORGPREP_INEXT %i %i ; to %i\n", 
                                    get_arga(insn), 
                                    jumpOffset, 
                                    pc + jumpOffset 
                            )) 
                    elseif opcode == 0x34 then -- FORGLOOP_INEXT 
                            local jumpOffset = get_argd(insn) 
                            table.insert(output, string.format( 
                                    "FORGLOOP_INEXT %i %i ; to %i\n", 
                                    get_arga(insn), 
                                    jumpOffset, 
                                    pc + jumpOffset 
                            )) 
                    elseif opcode == 0x17 then -- FORGPREP_NEXT 
                            local jumpOffset = get_argd(insn) 
                            table.insert(output, string.format( 
                                    "FORGPREP_NEXT %i %i ; to %i\n", 
                                    get_arga(insn), 
                                    jumpOffset, 
                                    pc + jumpOffset 
                            )) 
                    elseif opcode == 0xFA then -- FORGLOOP_NEXT 
                            local jumpOffset = get_argd(insn) 
                            table.insert(output, string.format( 
                                    "FORGLOOP_NEXT %i %i ; to %i\n", 
                                    get_arga(insn), 
                                    jumpOffset, 
                                    pc + jumpOffset 
                            )) 
                    elseif opcode == 0xDD then -- GETVARARGS 
                            table.insert(output, string.format( 
                                    "GETVARARGS %i %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn) 
                            )) 
                    elseif opcode == 0xC0 then -- DUPCLOSURE 
                            local childProtoId = get_argd(insn) 
                            table.insert(output, string.format( 
                                    "DUPCLOSURE %i %i ; global id = %i\n", 
                                    get_arga(insn), 
                                    childProtoId, 
                                    proto.k[childProtoId + 1] 
                            )) 
                    elseif opcode == 0xA3 then -- PREPVARARGS
                            table.insert(output, "PREPVARARGS " .. get_arga(insn) .. " " .. get_argb(insn) .. " " .. get_argc(insn) .. "\n")
                    elseif opcode == 0x86 then -- LOADKX 
                            pc += 1 
                            local aux = proto.code[pc] 
                            table.insert(output, string.format( 
                                    "LOADKX %i %i\n", 
                                    get_arga(insn), 
                                    aux 
                            )) 
                    elseif opcode == 0x69 then -- JUMPX 
                            local offset = get_arge(insn) 
                            table.insert(output, string.format( 
                                    "JUMPX %i ; to %i\n", 
                                    offset, 
                                    pc + offset 
                            )) 
                    elseif opcode == 0x4C then -- FASTCALL 
                            table.insert(output, string.format( 
                                    "FASTCALL %i %i\n", 
                                    get_arga(insn), 
                                    get_argc(insn) 
                            )) 
                    elseif opcode == 0x2F then -- COVERAGE 
                            table.insert(output, "COVERAGE\n") 
                    elseif opcode == 0x12 then -- CAPTURE 
                            local captureTypeId = get_arga(insn) 
                            table.insert(output, string.format( 
                                    "CAPTURE %i %i ; %s capture\n", 
                                    captureTypeId, 
                                    get_argb(insn), 
                                    CAPTURE_TYPES[captureTypeId] or "unknown" 
                            )) 
                    elseif opcode == 0xF5 then -- JUMPIFEQK 
                            local offset = get_argd(insn) 
                            local jumpTo = pc + offset 
                            pc += 1 
                            local aux = proto.code[pc] 
                            table.insert(output, string.format( 
                                    "JUMPIFEQK %i %i %i ; K(%i) = %s, to %i\n", 
                                    get_arga(insn), 
                                    aux, 
                                    offset, 
                                    aux, 
                                    getConstantString(proto.k[aux + 1]), 
                                    jumpTo 
                            )) 
                    elseif opcode == 0xD8 then -- JUMPIFNOTEQK 
                            local offset = get_argd(insn) 
                            local jumpTo = pc + offset 
                            pc += 1 
                            local aux = proto.code[pc] 
                            table.insert(output, string.format( 
                                    "JUMPIFNOTEQK %i %i %i ; K(%i) = %s, to %i\n", 
                                    get_arga(insn), 
                                    aux, 
                                    offset, 
                                    aux, 
                                    getConstantString(proto.k[aux + 1]), 
                                    jumpTo 
                            )) 
                    elseif opcode == 0xBB then -- FASTCALL1 
                            local offset = get_argc(insn) 
                            table.insert(output, string.format( 
                                    "FASTCALL1 %i %i %i ; to %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    offset, 
                                    pc + offset 
                            )) 
                    elseif opcode == 0x9E then -- FASTCALL2 
                            local offset = get_argc(insn) 
                            pc += 1 
                            local aux = proto.code[pc] 
                            table.insert(output, string.format( 
                                    "FASTCALL2 %i %i %i %i ; to %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    aux, 
                                    offset, 
                                    pc + offset 
                            )) 
                    elseif opcode == 0x81 then -- FASTCALL2K 
                            local offset = get_argc(insn) 
                            pc += 1 
                            local aux = proto.code[pc] 
                            table.insert(output, string.format( 
                                    "FASTCALL2K %i %i %i %i ; to %i\n", 
                                    get_arga(insn), 
                                    get_argb(insn), 
                                    aux, 
                                    offset, 
                                    pc + offset 
                            )) 
                    else -- Unknown opcode 
                            table.insert(output, string.format( 
                                    "UNKNOWN (%#010x)\n", 
                                    insn 
                            )) 
                    end 

                    pc += 1 
            end 
    end 

    return table.concat(output, "") 
end)

getgenv().disassemble = newcclosure(function(script) 
    local s, e = pcall(dissassemble_t, script)
    if (s == false) then
        return ("; script failed to disassemble or ignored")
    end

    return (("; script was disassembled by %s\n\n"):format(identifyexecutor()) .. e)
end)

local success, e = pcall(function() 
    if (isfile("cherry_decompiler.lua")) then
        loadstring(readfile("cherry_decompiler.lua"))()
        warn("Running Decompiler On File!")
    else 
        loadstring(game:HttpGet("https://raw.githubusercontent.com/auxopen/libraries/main/DEEEEEEEEEEECOMAPILAR.lua", true))()
    end
end)

if (success == false) then
    warn(e)
    pcall(function() 
        loadstring(game:HttpGet("https://raw.githubusercontent.com/auxopen/libraries/main/DEEEEEEEEEEECOMAPILAR.lua", true))()
    end)
end

)";

init_script += R"(
local on_actor_created_callback = {}
getgenv().getactors = newcclosure(function() 
    local t = {}

	for i, scr in game:GetDescendants() do 
		if (scr:IsA("Actor")) then 
			table.insert(t, scr);
		end
	end

	return t
end)

getgenv().isactor = newcclosure(function() 
    return false
end)

local isactor_true = newcclosure(function() 
    return true
end)

local on_actor_created_event = Instance.new("BindableEvent")
game.DescendantAdded:Connect(function(v)
	if v:IsA("Actor") then
		on_actor_created_event:Fire(v)
	end
end)

getgenv().on_actor_created = on_actor_created_event.Event;

getgenv().run_on_actor = newcclosure(function(actor, code, argument)
    assert(typeof(actor) == "Instance", ('bad argument #1 to \'run_on_actor\' (Instance expected, got %s)'):format(typeof(actor)))
    assert(actor.ClassName == "Actor", ('bad argument #1 to \'run_on_actor\' (Actor expected, got %s)') : format(actor.ClassName))
    assert(typeof(code) == "string", ('bad argument #2 to \'run_on_actor\' (string expected, got %s)') : format(typeof(code)))

    local custom_env = {
        isactor = isactor_true
    };
    local env = setmetatable({}, {
        __index = newcclosure(function(self, idx)
            if (custom_env[idx]) then return custom_env[idx] end
            return getgenv()[idx]
        end),
        __newindex = newcclosure(function(self, idx, val)
            custom_env[idx] = val
        end)
    })

    setfenv(loadstring(code), env)(argument)
end)

getgenv().checkparallel = newcclosure(function()
    local suc = pcall(task.synchronize)
	if suc then
		task.desynchronize()
		print("desynced")
		return true
	end
	return false
end)

local comm_channels = {}
getgenv().create_comm_channel = newcclosure(function()
	local id = game:GetService("HttpService"):GenerateGUID(false)
	local bindable = Instance.new("BindableEvent")
	local object = newproxy(true)
	getmetatable(object).__index = function(_, i)
		if i == "bro" then
			return bindable
		end
	end
	local event = setmetatable({
			__OBJECT = object
	}, {
		__type = "SynSignal",
		__index = function(self, i)
			if i == "Connect" then
				return function(_, callback)
					return self.__OBJECT.bro.Event:Connect(callback)
				end
			elseif i == "Fire" then
				return function(_, ...)
					return self.__OBJECT.bro:Fire(...)
				end
			end
		end,
		__newindex = function()
			error("SynSignal table is readonly.")
		end
	})
	comm_channels[id] = event
	return id, event
end)

getgenv().get_comm_channel = newcclosure(function(id)
	local channel = comm_channels[id]
    if not channel then
		error('bad argument #1 to \'get_comm_channel\' (invalid communication channel)')
    end
    return channel
end)

local specialinfo = {
	MeshPart = {
		"PhysicsData",
		"InitialSize"
	},
	UnionOperation = {
		"AssetId",
		"ChildData",
		"FormFactor",
		"InitialSize",
		"MeshData",
		"PhysicsData"
	},
	Terrain = {
		"SmoothGrid",
		"MaterialColors"
	}
}

getgenv().getspecialinfo = newcclosure(function(obj)
	assert(typeof(obj) == "Instance", string.format('bad argument #1 to \'getspecialinfo\' (Instance expected, got %s)', typeof(obj)))
    local info = specialinfo[obj.ClassName]
    local props = {}
    if info then
        for _, v in next, info do
            props[v] = gethiddenproperty(obj, v)
        end
    end
    return props
end)

getgenv().syn = table.freeze{
    cache_replace = cache.replace,
    cache_invalidate = cache.invalidate,
    set_thread_identity = setidentity,
    get_thread_identity = getidentity,
    is_cached = cache.is_cached,
    write_clipboard = setclipboard,
    queue_on_teleport = queue_on_teleport,
    protect_gui = newcclosure(function() 
        return
    end),
    unprotect_gui = newcclosure(function() 
        return
    end),
    is_beta = newcclosure(function() 
        return false
    end),
    crypto = crypt,
    crypt = crypt,
    request = request,
    secure_call = secure_call,

    on_actor_created = on_actor_created,
    run_on_actor = run_on_actor,
    getactors = getactors,
    create_comm_channel = create_comm_channel,
    get_comm_channel = get_comm_channel,
    checkparallel = checkparallel,
    isactor = isactor,
    
    oth = table.freeze{
        hook = hookfunction,
    }
}
)";

	lua_getglobal(L, "game");
	lua_getmetatable(L, -1);
	lua_getfield(L, -1, "__namecall");

	Closure* namecall = (Closure*)lua_topointer(L, -1);
	lua_CFunction namecall_f = namecall->c.f;
	old_namecall = (int)namecall_f;
	namecall->c.f = namecall_hook;

	lua_settop(L, 0);

	lua_getglobal(L, "game");
	lua_getmetatable(L, -1);
	lua_getfield(L, -1, "__index");

	Closure* index = (Closure*)lua_topointer(L, -1);
	lua_CFunction index_f = index->c.f;
	old_index = (int)index_f;
	index->c.f = index_hook;

	global::console->debug_write("HOOKED!", "LUA BRIDGE");


	/* tp handler */
	lua_pushcclosurek(L, tp_handler, NULL, NULL, NULL);
	lua_setfield(L, -10002, "reattach");

	global::execution->send(NULL, R"(
local re_att = clonefunction(reattach)
game:GetService("Players").LocalPlayer.OnTeleport:Connect(newcclosure(function(teleport_state) 
	if teleport_state == Enum.TeleportState.InProgress then
		re_att()
	end
end))
getgenv().reattach = nil
	)", true);

	global::console->debug_write("DONE!", "TELEPORT HANDLER");

	lua_settop(L, 0);

	global::execution->send(NULL, init_script.c_str(), true); /* pushes initscript to the task scheduler */
    global::execution->send(NULL, disassembler.c_str(), true); /* pushes disassembler to the task scheduler */

	auto exe = global::execution;
	std::thread([exe]() { /* I hate this */
			Sleep(1000);

			
			task_scheduler->push([exe](int)
				{
					for (int i = 0; i < queue_on_tp_.size(); i++)
						exe->send(NULL, queue_on_tp_[i], true);

					queue_on_tp_.clear();
				}
			);
		}
	).detach();

}