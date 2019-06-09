// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "rpc/server.h"
#include <string>
#include <iostream>
#include <unordered_map>
#include "rpc/this_session.h"

// Shared struct
#include "player.h"

using std::string;

#define PORT 8080

void StartServer() {
	rpc::server srv(PORT);
	// handlers goes here 
	//srv.bind("move", ...);

}

void UpdateLoop() {
	bool terminated = false;
	auto dt = std::chrono::milliseconds(30);
	while (!terminated) {
		// do data update
		// sleep at the end
		std::this_thread::sleep_for(dt);
	}
}

Player first;
Player second;

int main()
{
	// Set up rpc server and listen to PORT
	rpc::server srv(PORT);
	std::cout << "Listening to port: " << PORT << std::endl;
	std::unordered_map<rpc::session_id_t, int> data;

	// Define a rpc function: auto echo(string const& s, Player& p){} (return type is deduced)
	srv.bind("in", [&](int id, Player &p) {
		if (id == 1)
			second = p;
		else
			first = p;
	});
	srv.bind("out", [&](int id) {
		if (id == 1)
			return first;
		else
			return second;
	});
	srv.bind("fire", [&](int id) {
		if (id == 1)
			second.fire = true;
		else
			first.fire = true;
	});

	srv.bind("store_me_maybe", [&](int identifier) {
		auto id = rpc::this_session().id();
		data[id] = identifier;
	});


	// Blocking call to start the server: non-blocking call is srv.async_run(threadsCount);
	srv.run();
	return 0;
}
