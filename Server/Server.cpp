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

int main()
{
	// Set up rpc server and listen to PORT
	rpc::server srv(PORT);
	std::cout << "Listening to port: " << PORT << std::endl;

	// Define a rpc function: auto echo(string const& s, Player& p){} (return type is deduced)
	srv.bind("echo"/*function name*/, [/*put = here if you want to capture environment by value, & by reference*/]
	(string const& s, Player& p) /*function parameters*/
	{
		std::cout << "Get message: " << s << std::endl;
		std::cout << "Before: " << p.to_string() << std::endl;
		p.hp -= 1;
		p.pos.y += 1;
		p.rotation = glm::rotate(p.rotation, glm::radians(90.0f), glm::vec3(0, 1, 0));
		std::cout << "After: " << p.to_string() << std::endl;
		// return value : that will be returned back to client side
		return std::make_tuple(string("> ") + s, p);
	});

	// Blocking call to start the server: non-blocking call is srv.async_run(threadsCount);
	srv.run();
	return 0;
}
