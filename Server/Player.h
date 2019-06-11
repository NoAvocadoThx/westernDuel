#include <rpc/client.h>
#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>

struct Player
{
	bool fire;
	bool pickedUp;
	bool finishFire;
	bool dead;
	glm::vec3 pos;
	glm::quat rotation;
	glm::vec3 handpos;
	glm::quat handrotation;
	glm::vec3 headPos;
	glm::quat headrotation;

	// rpc Macro to generate serialize code for the struct (Note: for glm object, manually specify x,y,z,w)
	MSGPACK_DEFINE_MAP(fire, dead, pickedUp, finishFire,
		pos.x, pos.y, pos.z,
		rotation.x, rotation.y, rotation.z, rotation.w,
		handpos.x, handpos.y, handpos.z,
		handrotation.x, handrotation.y, handrotation.z, handrotation.w,
		headPos.x, headPos.y, headPos. z,
		headrotation.x, headrotation.y, headrotation.z, headrotation.w
	)
};

