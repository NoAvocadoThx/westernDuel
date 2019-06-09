// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "rpc/server.h"
#include <string>
#include <iostream>

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

void foo(int id, Player& p) {
	if (id == 1)
		first = p;
	else
		second = p;
}

int main()
{
	// Set up rpc server and listen to PORT
	rpc::server srv(PORT);
	std::cout << "Listening to port: " << PORT << std::endl;


	// Define a rpc function: auto echo(string const& s, Player& p){} (return type is deduced)
	srv.bind("in", &foo);
	srv.bind("out", [](int id) {
		if (id == 1)
			return first;
		else
			return second;
	});

	// Blocking call to start the server: non-blocking call is srv.async_run(threadsCount);
	srv.run();
	return 0;
}
