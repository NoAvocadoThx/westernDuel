#include <rpc/client.h>
#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>

struct Player
{
	float hp;
	float cd;
	glm::vec3 pos;
	glm::quat rotation;

	std::string to_string()
	{
		return "HP: " + std::to_string(hp) + "; CD: " + std::to_string(cd) +
			";\nPosition: " + glm::to_string(pos) + ";\nRotation: " + glm::to_string(rotation);
	}

	// rpc Macro to generate serialize code for the struct (Note: for glm object, manually specify x,y,z,w)
	MSGPACK_DEFINE_MAP(hp, cd, pos.x, pos.y, pos.z, rotation.x, rotation.y, rotation.z, rotation.w)
};
