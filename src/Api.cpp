#include "Api.h"

#include <sstream>
#include <json/json.h>

namespace ApiServer {

Api::Api() : httpsServer("server.crt","server.key"), storage("workspace") {
	locations = {
			{"GET",
					{"/json/","/json/status","/json/files"}
			},
			{"POST",
					{"/json/","/json/update"}
			},
			{"PUT",
					{"/json/"}
			},
			{"DELETE",
					{"/json/"}
			}
	};

	storage.SetAttributes("/home/humroben");
	storage.Init();
}

Api::~Api() {
	server_thread.~thread();
}

void Api::Configure() {
	httpsServer.config.port = 8443;
	httpsServer.config.thread_pool_size = 5;

	for (auto iter = locations.begin(); iter != locations.end(); iter++) {
		for (auto viter = (*iter).second.begin(); viter < (*iter).second.end(); viter++) {
			httpsServer.resource[*viter][(*iter).first] = [this](std::shared_ptr<Https::Response> response, std::shared_ptr<Https::Request> request) {
				std::unordered_map<std::string,std::string> header;
				std::string content = Process(request->method, request->path, header, request->content.string());

				std::stringstream ss;

				for (const auto& head : header) {
					if (head.first == "Content-Length")
						ss << head.first << ": " << content.length() << "\r\n";
					else
						ss << head.first << ": " << head.second << "\r\n";
				}

				if (content.find("JSON Error") != std::string::npos) {
					*response << "HTTP/1.1 400 OK\r\n" << ss.str() << "\r\n" << content;
				} else if (content.find("JSON Not Found") != std::string::npos) {
					*response << "HTTP/1.1 404 Not Found\r\n" << ss.str() << "\r\n" << content;
				} else {
					*response << "HTTP/1.1 200 OK\r\n" << ss.str() << "\r\n" << content;
				}
			};
		}
	}

	httpsServer.default_resource["HEAD"] = [](std::shared_ptr<Https::Response> response, std::shared_ptr<Https::Request> request) {
		auto content = request->content.string();

		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
	};

	httpsServer.default_resource["GET"] = [](std::shared_ptr<Https::Response> response, std::shared_ptr<Https::Request> request) {
		auto content = request->content.string();

		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
	};
}

void Api::Start() {
	server_thread = std::thread(
			[&httpsServer = httpsServer, &server_port = server_port]() {
				httpsServer.start([&server_port](unsigned short port) {
							server_port.set_value(port);
						});
			});
	//server_thread.detach();
}

void Api::Join() {
	server_thread.join();
}

std::string Api::Process(std::string method, std::string path, std::unordered_map<std::string,std::string> &header, std::string data) {
	std::string content;

	Json::Value jValue;
	Json::Reader jReader;

	bool valid;
	if (!data.empty()) {
		valid = jReader.parse(data,jValue);
	} else {
		valid = false;
	}

	if (path == "/json/files" && method == "GET") {
		header["Content-Length"] = "";
		header["Content-Type"] = "application/json";
		header["Server"] = "CP_API/0.0.1";

		if (!data.empty() && valid) {
			const Json::Value path = jValue["path"];
			Directory temp = storage.GetDir(path.asString());
			if (temp.GetName() != "null") {
				content = temp.Print("json",true);
			} else {
				content = "{ \"data\": [{ \"type\" : \"JSON Not Found\", \"msg\" : \"Requested Data Not Found\" }]}";
			}
		} else if (!data.empty() && !valid) {
			content = "{ \"data\": [{ \"type\" : \"JSON Error\", \"msg\" : \"Invalid JSON Input\" }]}";
		} else {
			content = storage.Print("json",true);
		}
	}

	if (path == "/json/update" && method == "POST") {
		if (!data.empty()) {

		} else {
			return "{ \"data\": [{ \"type\" : \"JSON Error\", \"msg\" : \"Empty dataset\" }]}";
		}
	}


	return content;
}

}
