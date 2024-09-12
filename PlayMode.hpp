#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, space, enter;

	struct Cup {
		std::string color;
		Scene::Transform *transform = nullptr;
		glm::vec3 base_position = glm::vec3(0.0f, 0.0f, 0.0f);
	};

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	// Scene::Transform *hip = nullptr;
	// Scene::Transform *upper_leg = nullptr;
	// Scene::Transform *lower_leg = nullptr;
	// Scene::Transform *red = nullptr;
	// Scene::Transform *orange = nullptr;
	// Scene::Transform *yellow = nullptr;
	// Scene::Transform *green = nullptr;
	// Scene::Transform *blue = nullptr;
	// Scene::Transform *purple = nullptr;
	// glm::quat hip_base_rotation;
	// glm::quat upper_leg_base_rotation;
	// glm::quat lower_leg_base_rotation;
	// float wobble = 0.0f;
	Cup red;
	Cup orange;
	Cup yellow;
	Cup green;
	Cup blue;
	Cup purple;
	
	Cup *player_cup_order[6];
	Cup *target_order[6];
	uint32_t current_cup_index = 0;
	uint32_t selected_1 = 6;
	uint32_t selected_2 = 6;
	
	//camera:
	Scene::Camera *camera = nullptr;
	uint32_t correct_cups = 0;
	bool moving_cups = false;
	Cup *temp_1 = nullptr;
	Cup *temp_2 = nullptr;
	uint32_t moves = 0;

};
