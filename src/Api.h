#ifndef API_H
#define API_H

#define USE_STANDALONE_ASIO 1

#include "../../Directory/src/Directory.h"

#include <server_http.hpp>
#include <server_https.hpp>

#include <thread>

namespace ApiServer {

using Http = SimpleWeb::Server<SimpleWeb::HTTP>;
using Https = SimpleWeb::Server<SimpleWeb::HTTPS>;

class Api {
public:
	Api();
	virtual ~Api();

	void Configure();
	void Start();
	void Stop();
	void Join();

	std::string Process(std::string method, std::string path, std::unordered_map<std::string, std::string> &header, std::string data = "");

private:
	std::promise<unsigned short> server_port;

	Http httpServer;
	Https httpsServer;

	std::unordered_map<std::string, std::vector<std::string>> locations;
	std::thread server_thread;

	Directory storage;
};

}

#endif
