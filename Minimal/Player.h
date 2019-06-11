#include <rpc/client.h>
#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>

struct Player
{
	bool fire;
	bool fired;
	bool pickedUp;
	bool finishFire;
	bool dead;
	glm::quat rotation;
	glm::vec3 handpos;
	glm::quat handrotation;
	glm::vec3 headPos;
	glm::quat headrotation;
	glm::vec3 viewDir;
	glm::vec3 shootDir;

	// rpc Macro to generate serialize code for the struct (Note: for glm object, manually specify x,y,z,w)
	MSGPACK_DEFINE_MAP(fire, dead, pickedUp, finishFire, fired,
		rotation.x, rotation.y, rotation.z, rotation.w,
		handpos.x, handpos.y, handpos.z,
		handrotation.x, handrotation.y, handrotation.z, handrotation.w,
		headPos.x, headPos.y, headPos.z,
		headrotation.x, headrotation.y, headrotation.z, headrotation.w,
		viewDir.x, viewDir.y, viewDir.z,
		shootDir.x, shootDir.y, shootDir.z
	)
};

