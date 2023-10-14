#include "environment.h"

namespace cherry {
	environment* environment::singleton{ nullptr };
	auto environment::getSingleton() -> environment* {
		if (singleton == nullptr)
			singleton = new environment();

		return singleton;
	};

	auto environment::createEnvironment(lua_State* L) -> void {
		/* regular */
		this->createEnv(L);
		this->createClosure(L);
		this->createConsole(L);
		this->createFileSystem(L);
		this->createInput(L);
		this->createMetatable(L);
		this->createHttp(L);
		this->createOthers(L);

		/* libraries */
		this->createCache(L);
		this->createDebug(L);
		this->createDrawing(L);
		this->createCrypt(L);
		this->createWebSockets(L);
		this->createCherry(L);

		/* After register */
		this->createInit(L);
	};

	auto environment::setRobloxTable(Table* tab) -> void {
		this->robloxEnv = tab;
	};

	auto environment::getRobloxTable() -> Table* {
		return this->robloxEnv;
	};
}