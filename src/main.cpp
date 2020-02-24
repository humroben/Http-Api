#include "Api.h"

#include <iostream>

int main () {
	ApiServer::Api api;
	api.Configure();
	api.Start();

	api.Join();
}
