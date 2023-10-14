#include "environment.hpp"
using namespace cherry::global;

#pragma region stuff
enum CryptModes
{
	//AES
	AES_CBC,
	AES_CFB,
	AES_CTR,
	AES_OFB,
	AES_GCM,
	AES_EAX,

	//Blowfish
	BF_CBC,
	BF_CFB,
	BF_OFB
};

enum HashModes
{
	//MD5
	MD5,

	//SHA1
	SHA1,

	//SHA2
	SHA224,
	SHA256,
	SHA384,
	SHA512,

	//SHA3
	SHA3_256,
	SHA3_384,
	SHA3_512,
};


std::map<std::string, CryptModes> CryptTranslationMap =
{
	//AES
	{ "aes-cbc", AES_CBC },
	{ "aes_cbc", AES_CBC },

	{ "aes-cfb", AES_CFB },
	{ "aes_cfb", AES_CFB },

	{ "aes-ctr", AES_CTR },
	{ "aes_ctr", AES_CTR },

	{ "aes-ofb", AES_OFB },
	{ "aes_ofb", AES_OFB },

	{ "aes-gcm", AES_GCM },
	{ "aes_gcm", AES_GCM },

	{ "aes-eax", AES_EAX },
	{ "aes_eax", AES_EAX },

	//Blowfish
	{ "blowfish-cbc", BF_CBC },
	{ "blowfish_cbc", BF_CBC },
	{ "bf-cbc", BF_CBC },
	{ "bf_cbc", BF_CBC },

	{ "blowfish-cfb", BF_CFB },
	{ "blowfish_cfb", BF_CFB },
	{ "bf-cfb", BF_CFB },
	{ "bf_cfb", BF_CFB },

	{ "blowfish-ofb", BF_OFB },
	{ "blowfish_ofb", BF_OFB },
	{ "bf-ofb", BF_OFB },
	{ "bf_ofb", BF_OFB },
};

std::map<std::string, HashModes> HashTranslationMap =
{
	//MD5
	{ "md5", MD5 },

	//SHA1
	{ "sha1", SHA1 },

	//SHA2
	{ "sha224", SHA224 },
	{ "sha256", SHA256 },
	{ "sha384", SHA384 },
	{ "sha512", SHA512 },

	//SHA3
	{ "sha3-256", SHA3_256 },
	{ "sha3_256", SHA3_256 },
	{ "sha3-384", SHA3_384 },
	{ "sha3_384", SHA3_384 },
	{ "sha3-512", SHA3_512 },
	{ "sha3_512", SHA3_512 },
};

std::string Base64Decode(const std::string& encoded_string)
{
	std::string decoded;
	CryptoPP::StringSource ss(encoded_string, true,
		new CryptoPP::Base64Decoder(
			new CryptoPP::StringSink(decoded)
		));

	return decoded;
}

std::string Base64Encode(const byte* bytes_to_encode, size_t in_len)
{
	std::string encoded;
	CryptoPP::StringSource ss(bytes_to_encode, in_len, true,
		new CryptoPP::Base64Encoder(
			new CryptoPP::StringSink(encoded),
			false
		));

	return encoded;
}

template<typename T>
__forceinline std::string EncryptWithAlgo(lua_State* L, const std::string& data, const std::string& key_size, const std::string& ivc_str)
{
	try
	{
		std::string Encrypted;

		T Encryptor;
		Encryptor.SetKeyWithIV((byte*)key_size.c_str(), key_size.size(), (byte*)ivc_str.c_str(), ivc_str.length());

		CryptoPP::StringSource ss(data, true,
			new CryptoPP::StreamTransformationFilter(Encryptor,
				new CryptoPP::StringSink(Encrypted)
			)
		);

		return Base64Encode((unsigned char*)Encrypted.c_str(), Encrypted.size());
	}
	catch (CryptoPP::Exception& e)
	{
		luaL_error(L, e.what());
		return "";
	}
}

template<typename T>
__forceinline std::string EncryptAuthenticatedWithAlgo(lua_State* L, const std::string& data, const std::string& key_size, const std::string& ivc_str)
{
	try
	{
		std::string Encrypted;

		T Encryptor;
		Encryptor.SetKeyWithIV((byte*)key_size.c_str(), key_size.size(), (byte*)ivc_str.c_str(), ivc_str.size());

		CryptoPP::AuthenticatedEncryptionFilter Aef(Encryptor,
			new CryptoPP::StringSink(Encrypted)
		);

		Aef.Put((const byte*)data.data(), data.size());
		Aef.MessageEnd();

		return Base64Encode((unsigned char*)Encrypted.c_str(), Encrypted.size());
	}
	catch (CryptoPP::Exception& e)
	{
		luaL_error(L, e.what());
		return "";
	}
}

template<typename T>
__forceinline std::string DecryptWithAlgo(lua_State* L, const std::string& Ciphertext, const std::string& Key, const std::string& IV)
{
	try
	{
		std::string Decrypted;

		T Decryptor;
		Decryptor.SetKeyWithIV((byte*)Key.c_str(), Key.size(), (byte*)IV.c_str(), IV.length());

		const auto Base = Base64Decode(Ciphertext);

		CryptoPP::StringSource ss(Base, true,
			new CryptoPP::StreamTransformationFilter(Decryptor,
				new CryptoPP::StringSink(Decrypted)
			)
		);

		return Decrypted;
	}
	catch (CryptoPP::Exception& e)
	{
		luaL_error(L, e.what());
		return "";
	}
}

template<typename T>
__forceinline std::string DecryptAuthenticatedWithAlgo(lua_State* L, const std::string& Ciphertext, const std::string& Key, const std::string& IV)
{
	try
	{
		std::string Decrypted;

		T Decryptor;
		Decryptor.SetKeyWithIV((byte*)Key.c_str(), Key.size(), (byte*)IV.c_str(), IV.size());

		const auto Base = Base64Decode(Ciphertext);

		CryptoPP::AuthenticatedDecryptionFilter Adf(Decryptor,
			new CryptoPP::StringSink(Decrypted)
		);

		Adf.Put((const byte*)Base.data(), Base.size());
		Adf.MessageEnd();

		return Decrypted;
	}
	catch (CryptoPP::Exception& e)
	{
		luaL_error(L, e.what());
		return "";
	}
}

void SplitString(std::string Str, std::string By, std::vector<std::string>& Tokens)
{
	Tokens.push_back(Str);
	const auto splitLen = By.size();
	while (true)
	{
		auto frag = Tokens.back();
		const auto splitAt = frag.find(By);
		if (splitAt == std::string::npos)
			break;
		Tokens.back() = frag.substr(0, splitAt);
		Tokens.push_back(frag.substr(splitAt + splitLen, frag.size() - (splitAt + splitLen)));
	}
}

#pragma endregion

auto encryptstringcustom(lua_State* L) -> int {
	ARG_CHECK(L, 4, 4);
	std::string algo = luaL_checklstring(L, 1, NULL);
	std::string data = luaL_checklstring(L, 2, NULL);
	std::string key_size = luaL_checklstring(L, 3, NULL);
	std::string ivc_str = luaL_checklstring(L, 4, NULL);

	std::transform(algo.begin(), algo.end(), algo.begin(), tolower);

	if (!CryptTranslationMap.count(algo)) {
		luaL_argerror(L, 1, "non-existant algorithm");
		return 0;
	}

	const auto ralgo = CryptTranslationMap[algo];
	std::string result;

	if (ralgo == AES_CBC) {
		result = EncryptWithAlgo<CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == AES_CFB) {
		result = EncryptWithAlgo<CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == AES_CTR) {
		result = EncryptWithAlgo<CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == AES_OFB) {
		result = EncryptWithAlgo<CryptoPP::OFB_Mode<CryptoPP::AES>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == AES_GCM) {
		result = EncryptAuthenticatedWithAlgo<CryptoPP::GCM<CryptoPP::AES>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == AES_EAX) {
		result = EncryptAuthenticatedWithAlgo<CryptoPP::EAX<CryptoPP::AES>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == BF_CBC) {
		result = EncryptWithAlgo<CryptoPP::CBC_Mode<CryptoPP::Blowfish>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == BF_CFB) {
		result = EncryptWithAlgo<CryptoPP::CFB_Mode<CryptoPP::Blowfish>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == BF_OFB) {
		result = EncryptWithAlgo<CryptoPP::OFB_Mode<CryptoPP::Blowfish>::Encryption>(L, data, key_size, ivc_str);
	}
	else {
		luaL_argerror(L, 1, "non-existant algorithm");
		return 0;
	}

	lua_pushlstring(L, result.c_str(), result.size());
	return 1;
};

auto encryptstring(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2);
	std::string data = luaL_checklstring(L, 1, NULL);
	std::string key = luaL_checklstring(L, 2, NULL);

	CryptoPP::AutoSeededRandomPool Prng;
	byte IV[12];
	Prng.GenerateBlock(IV, 12);

	byte derived_key[32];
	CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA384> KDF;
	KDF.DeriveKey(derived_key, 32, 0, (byte*)key.c_str(), key.size(), NULL, 0, 10000);

	auto encrypted = EncryptAuthenticatedWithAlgo<CryptoPP::GCM<CryptoPP::AES>::Encryption>(L,
		std::string(data.c_str(), data.size()),
		std::string((const char*)derived_key, 32),
		std::string((const char*)IV, 12));

	encrypted += "|" + Base64Encode(IV, 12);
	encrypted = Base64Encode((byte*)encrypted.data(), encrypted.size());


	lua_pushlstring(L, encrypted.c_str(), encrypted.size());
	return 1;
}

auto decryptstringcustom(lua_State* L) -> int {
	ARG_CHECK(L, 4, 4);
	std::string algo = luaL_checklstring(L, 1, NULL);
	std::string data = luaL_checklstring(L, 2, NULL);
	std::string key_size = luaL_checklstring(L, 3, NULL);
	std::string ivc_str = luaL_checklstring(L, 4, NULL);

	std::transform(algo.begin(), algo.end(), algo.begin(), tolower);

	if (!CryptTranslationMap.count(algo)) {
		luaL_argerror(L, 1, "non-existant algorithm");
		return 0;
	}

	const auto ralgo = CryptTranslationMap[algo];
	std::string result;

	if (ralgo == AES_CBC) {
		result = DecryptWithAlgo<CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == AES_CFB) {
		result = DecryptWithAlgo<CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == AES_CTR) {
		result = DecryptWithAlgo<CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == AES_OFB) {
		result = DecryptWithAlgo<CryptoPP::OFB_Mode<CryptoPP::AES>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == AES_GCM) {
		result = DecryptAuthenticatedWithAlgo<CryptoPP::GCM<CryptoPP::AES>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == AES_EAX) {
		result = DecryptAuthenticatedWithAlgo<CryptoPP::EAX<CryptoPP::AES>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == BF_CBC) {
		result = EncryptWithAlgo<CryptoPP::CBC_Mode<CryptoPP::Blowfish>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == BF_CFB) {
		result = DecryptWithAlgo<CryptoPP::CFB_Mode<CryptoPP::Blowfish>::Encryption>(L, data, key_size, ivc_str);
	}
	else if (ralgo == BF_OFB) {
		result = DecryptWithAlgo<CryptoPP::OFB_Mode<CryptoPP::Blowfish>::Encryption>(L, data, key_size, ivc_str);
	}
	else {
		luaL_argerror(L, 1, "non-existant algorithm");
		return 0;
	}

	lua_pushlstring(L, result.c_str(), result.size());
	return 1;
};

auto decryptstring(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2);
	std::string data = luaL_checklstring(L, 1, NULL);
	std::string key = luaL_checklstring(L, 2, NULL);

	byte DerivedKey[32];
	CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA384> KDF;
	KDF.DeriveKey(DerivedKey, 32, 0, (byte*)key.c_str(), key.size(), NULL, 0, 10000);

	std::vector<std::string> Split;
	SplitString(Base64Decode(data), "|", Split);

	if (Split.size() != 2) {
		luaL_argerror(L, 1, "Invalid encrypted string specified");
		return 0;
	}

	auto decrypted = DecryptAuthenticatedWithAlgo<CryptoPP::GCM<CryptoPP::AES>::Decryption>(L,
		Split.at(0),
		std::string((const char*)DerivedKey, 32),
		Base64Decode(Split.at(1)));


	lua_pushlstring(L, decrypted.c_str(), decrypted.size());
	return 1;
}

auto hashstring(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1);
	std::string data = luaL_checklstring(L, 1, NULL);

	auto hash = hash_with_algo<CryptoPP::SHA384>(data);
	lua_pushlstring(L, hash.c_str(), hash.size());
	return 1;
}

auto hashstringcustom(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2);
	std::string algo = luaL_checklstring(L, 1, NULL);
	std::string data = luaL_checklstring(L, 2, NULL);

	std::transform(algo.begin(), algo.end(), algo.begin(), tolower);

	if (!HashTranslationMap.count(algo))
	{
		luaL_argerror(L, 1, "non-existant hash algorithm");
		return 0;
	}

	const auto ralgo = HashTranslationMap[algo];

	std::string hash;

	//This is intentional - blame Themida not supporting jump tables.
	if (ralgo == MD5) {
		hash = hash_with_algo<CryptoPP::Weak::MD5>(data);
	}
	else if (ralgo == SHA1) {
		hash = hash_with_algo<CryptoPP::SHA1>(data);
	}
	else if (ralgo == SHA224) {
		hash = hash_with_algo<CryptoPP::SHA224>(data);
	}
	else if (ralgo == SHA256) {
		hash = hash_with_algo<CryptoPP::SHA256>(data);
	}
	else if (ralgo == SHA384) {
		hash = hash_with_algo<CryptoPP::SHA384>(data);
	}
	else if (ralgo == SHA512) {
		hash = hash_with_algo<CryptoPP::SHA512>(data);
	}
	else if (ralgo == SHA3_256) {
		hash = hash_with_algo<CryptoPP::SHA3_256>(data);
	}
	else if (ralgo == SHA3_384) {
		hash = hash_with_algo<CryptoPP::SHA3_384>(data);
	}
	else if (ralgo == SHA3_512) {
		hash = hash_with_algo<CryptoPP::SHA3_512>(data);
	}
	else {
		luaL_argerror(L, 1, "non-existant hash algorithm");
		return 0;
	}

	lua_pushlstring(L, hash.c_str(), hash.size());

	return 1;
}

auto randomstring(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1);
	const auto Size = luaL_checkinteger(L, 1);

	if (Size > 1024)
	{
		luaL_argerror(L, 1, "exceeded maximum size (1024)");
		return 0;
	}

	if (Size < 0)
	{
		luaL_argerror(L, 1, "negative size specified");
		return 0;
	}

	CryptoPP::AutoSeededRandomPool Prng;
	auto Alloc = (byte*) operator new(Size);
	Prng.GenerateBlock(Alloc, Size);

	lua_pushlstring(L, (const char*)Alloc, Size);

	delete Alloc;

	return 1;
}

auto derivestring(lua_State* L) -> int {
	ARG_CHECK(L, 2, 2);
	std::string data = luaL_checklstring(L, 1, NULL);
	const auto Size = luaL_checkinteger(L, 2);

	if (Size > 1024)
	{
		luaL_argerror(L, 1, "exceeded maximum size (1024)");
		return 0;
	}

	if (Size < 0)
	{
		luaL_argerror(L, 1, "negative size specified");
		return 0;
	}

	auto Alloc = (byte*) operator new(Size);
	CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA384> KDF;
	KDF.DeriveKey(Alloc, Size, 0, (byte*)data.c_str(), data.size(), NULL, 0, 10000);

	lua_pushlstring(L, (const char*)Alloc, Size);

	delete Alloc;

	return 1;
}

auto generatebytes(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1);
	int len = luaL_checkinteger(L, 1);

	if (len > 1024)
	{
		luaL_argerror(L, 1, "exceeded maximum size (1024)");
		return 0;
	}

	if (len < 0)
	{
		luaL_argerror(L, 1, "negative size specified");
		return 0;
	}

	std::string data = utilities->random_string(len);

	auto encoded = Base64Encode((unsigned char*)data.c_str(), data.size());
	lua_pushlstring(L, encoded.c_str(), encoded.size());

	return 1;
}

auto generatekey(lua_State* L) -> int {
	ARG_CHECK(L, 0, 0);
	std::string data = utilities->random_string(32);

	auto encoded = Base64Encode((unsigned char*)data.c_str(), data.size());
	lua_pushlstring(L, encoded.c_str(), encoded.size());

	return 1;
}


auto base64encode(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1);
	std::string data = luaL_checklstring(L, 1, NULL);

	auto encoded = Base64Encode((unsigned char*)data.c_str(), data.size());
	lua_pushlstring(L, encoded.c_str(), encoded.size());

	return 1;
}

auto base64decode(lua_State* L) -> int {
	ARG_CHECK(L, 1, 1);
	const auto data = luaL_checklstring(L, 1, NULL);

	auto decoded = Base64Decode(data);

	lua_pushlstring(L, decoded.c_str(), decoded.size());

	return 1;
}

static const luaL_Reg crypt_funcs[] = { // {"NAME", FUNC},
	{"encrypt", encryptstring},
	{"decrypt", decryptstring},
	{"hash", hashstring},
	{"random", randomstring},
	{"derive", derivestring},
	{"generatebytes", generatebytes},
	{"generatekey", generatekey},

	{"base64encode", base64encode},
	{"base64decode", base64decode},
	{"base64_encode", base64encode},
	{"base64_decode", base64decode},

	{NULL, NULL},
};

static const luaL_Reg base64_funcs[] = { // {"NAME", FUNC},
	{"encode", base64encode},
	{"decode", base64decode},
	{NULL, NULL},
};

#define add_to_list(L, fn, fn_name) lua_pushcclosure(L, fn, fn_name, 0); \
lua_setfield(L, -2, fn_name);

auto cherry::environment::luauopen_crypt(lua_State* L) -> void {
	/* base64 */
	lua_createtable(L, 0, 0);
	lua_setglobal(L, "base64");

	luaL_register(L, "base64", base64_funcs);

	lua_getglobal(L, "base64");
	lua_setreadonly(L, -1, true);
	lua_settop(L, 0);

	/* crypt */
	lua_createtable(L, 0, 0);
	lua_setglobal(L, "crypt");

	luaL_register(L, "crypt", crypt_funcs);

	/* custom table */
	lua_getglobal(L, "crypt");

	lua_createtable(L, 0, 0);
	add_to_list(L, encryptstringcustom, "encrypt");
	add_to_list(L, decryptstringcustom, "decrypt");
	add_to_list(L, hashstringcustom, "hash");

	lua_setreadonly(L, -1, true);
	lua_setfield(L, -2, "custom");

	lua_getglobal(L, "base64");
	lua_setfield(L, -2, "base64");

	/* redonly for crypt */
	lua_setreadonly(L, -1, true);

	lua_pushvalue(L, LUA_GLOBALSINDEX);
	add_to_list(L, base64encode, "base64encode");
	add_to_list(L, base64decode, "base64decode");
	add_to_list(L, base64encode, "base64_encode");
	add_to_list(L, base64decode, "base64_decode");

	lua_settop(L, 0);
}