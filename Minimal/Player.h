#include <rpc/client.h>
#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>

struct Player
{
	bool fire;
	bool dead;
	glm::vec3 pos;
	glm::quat rotation;
	glm::vec3 handpos;
	glm::quat handrotation;

	// rpc Macro to generate serialize code for the struct (Note: for glm object, manually specify x,y,z,w)
	MSGPACK_DEFINE_MAP(fire, dead,
		pos.x, pos.y, pos.z,
		rotation.x, rotation.y, rotation.z, rotation.w,
		handpos.x, handpos.y, handpos.z,
		handrotation.x, handrotation.y, handrotation.z, handrotation.w
	)
};
