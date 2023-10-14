#include <Windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <map>

std::map< std::string, std::vector<std::function<void(std::string)>> > read_callbacks;

auto callback_init(std::string pipe_name, HANDLE handle) -> void {
	char buffer[999999];
	std::string script = "";
	DWORD dwRead;

	while (handle != INVALID_HANDLE_VALUE) {
		if (ConnectNamedPipe(handle, NULL) != FALSE) {
			while (ReadFile(handle, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE) {
				/* add terminating zero */
				buffer[dwRead] = '\0';
				std::vector<std::function<void(std::string)>> vec = read_callbacks[pipe_name];
				
				for (unsigned int i = 0; i < vec.size(); i++)
					vec[i](std::string{ buffer, strlen(buffer) });
			}
		}

		DisconnectNamedPipe(handle);
	}
}

namespace cherry {
	class pipe {
	private:
		std::string name;
		HANDLE handle = nullptr;
	public:
		pipe(std::string pipe_name) {
			this->name = "\\\\.\\pipe\\" + pipe_name;
			this->handle = CreateNamedPipeA(TEXT(this->name.c_str()),
				PIPE_ACCESS_DUPLEX,
				PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
				1,
				9999999,
				9999999,
				NMPWAIT_USE_DEFAULT_WAIT,
				NULL);

			read_callbacks.insert( std::pair<std::string, std::vector<std::function<void(std::string)>>>(pipe_name, {  }));
			std::thread(callback_init, this->name, this->handle).detach();
		}


		auto write(std::string data) -> DWORD {
			DWORD bytes_written;
			WriteFile(this->handle, data.c_str(), (data.size() + 1), &bytes_written, NULL);
			return bytes_written;
		};

		auto on_read(std::function<void(std::string)> read_func) -> void {
			read_callbacks[this->name].push_back(read_func);
		};

	};
}