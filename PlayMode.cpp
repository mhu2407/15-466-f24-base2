#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint cups_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > cups_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("cups.pnct"));
	cups_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > cups_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("cups.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = cups_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = cups_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

PlayMode::PlayMode() : scene(*cups_scene) {
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "Cylinder.006") {
			red.color = "red";
			red.transform = &transform;
			red.base_position = red.transform->position;
			player_cup_order[0] = &red;
			target_order[3] = &red;
		} else if (transform.name == "Cylinder.005") {
			orange.color = "orange";
			orange.transform = &transform;
			orange.base_position = orange.transform->position;
			player_cup_order[1] = &orange;
			target_order[1] = &orange;
		} else if (transform.name == "Cylinder.004") {
			yellow.color = "yellow";
			yellow.transform = &transform;
			yellow.base_position = yellow.transform->position;
			player_cup_order[2] = &yellow;
			target_order[5] = &yellow;
		} else if (transform.name == "Cylinder.003") {
			purple.color = "purple";
			purple.transform = &transform;
			purple.base_position = purple.transform->position;
			player_cup_order[5] = &purple;
			target_order[4] = &purple;
		} else if (transform.name == "Cylinder.002") {
			green.color = "green";
			green.transform = &transform;
			green.base_position = green.transform->position;
			player_cup_order[3] = &green;
			target_order[0] = &green;
		} else if (transform.name == "Cylinder.001") {
			blue.color = "blue";
			blue.transform = &transform;
			blue.base_position = blue.transform->position;
			player_cup_order[4] = &blue;
			target_order[2] = &blue;
		}

	}
	if (red.transform == nullptr) throw std::runtime_error("Red cup not found.");
	if (orange.transform == nullptr) throw std::runtime_error("Orange cup not found.");
	if (yellow.transform == nullptr) throw std::runtime_error("Yellow cup not found.");
	if (green.transform == nullptr) throw std::runtime_error("Green cup not found.");
	if (blue.transform == nullptr) throw std::runtime_error("Blue cup not found.");
	if (purple.transform == nullptr) throw std::runtime_error("Purple cup not found.");

	camera = &scene.cameras.front();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			if (current_cup_index == 0) current_cup_index = 5;
			else current_cup_index -= 1;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			if (current_cup_index == 5) current_cup_index = 0;
			else current_cup_index += 1;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = true;
			space.downs += 1;
			// unselect if already selected
			if (selected_1 == current_cup_index) selected_1 = 6;
			else if (selected_2 == current_cup_index) {
				selected_2 = selected_1;
				selected_1 = 6;
			} else {
				assert((selected_1 != current_cup_index) && (selected_1 != current_cup_index));
				selected_2 = selected_1;
				selected_1 = current_cup_index;
			}
			return true;
		} else if (evt.key.keysym.sym == SDLK_RETURN) {
			// swap cups and cup positions
			if ((selected_2 != 6) && (selected_1 != 6)) {
				enter.pressed = true;
				enter.downs += 1;
			}
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RETURN) {
			enter.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	float speed = 20.0f;
	if (enter.pressed) {
		moving_cups = true;
	}
	if (moving_cups) {
		// ensure that selected_2 has the greater y position
		if (selected_1 > selected_2) {
			uint32_t temp_selected = selected_1;
			selected_1 = selected_2;
			selected_2 = temp_selected;
		}
		temp_1 = player_cup_order[selected_1];
		temp_2 = player_cup_order[selected_2];

		assert(selected_1 < selected_2);
		// move cups to new x
		if (player_cup_order[selected_1]->transform->position.y < player_cup_order[selected_2]->base_position.y) {
			player_cup_order[selected_1]->transform->position.y += speed * elapsed;
			player_cup_order[selected_2]->transform->position.y -= speed * elapsed;
			if (player_cup_order[selected_1]->transform->position.y > player_cup_order[selected_2]->base_position.y) {
				player_cup_order[selected_1]->transform->position.y = player_cup_order[selected_2]->base_position.y;
			}
			if (player_cup_order[selected_2]->transform->position.y < player_cup_order[selected_1]->base_position.y) {
				player_cup_order[selected_2]->transform->position.y = player_cup_order[selected_1]->base_position.y;
			}
		// move cups down to original height
		} else if (player_cup_order[selected_1]->transform->position.z > player_cup_order[selected_1]->base_position.z) {
			player_cup_order[selected_1]->transform->position.z -= speed * elapsed;
			player_cup_order[selected_2]->transform->position.z -= speed * elapsed;
			if (player_cup_order[selected_1]->transform->position.z < player_cup_order[selected_1]->base_position.z) {
				player_cup_order[selected_1]->transform->position.z = player_cup_order[selected_1]->base_position.z;
			}
			if (player_cup_order[selected_2]->transform->position.z < player_cup_order[selected_2]->base_position.z) {
				player_cup_order[selected_2]->transform->position.z = player_cup_order[selected_2]->base_position.z;
			}
		// swap cups within array
		} else {
			player_cup_order[selected_1]->base_position = player_cup_order[selected_1]->transform->position;
			player_cup_order[selected_2]->base_position = player_cup_order[selected_2]->transform->position;
			player_cup_order[selected_1] = temp_2;
			player_cup_order[selected_2] = temp_1;
			selected_1 = 6;
			selected_2 = 6;
			moves += 1;
			moving_cups = false;
		}
	// hover cups to indicate selection and current cup
	} else { 
		correct_cups = 0;
		for (uint32_t i = 0; i < 6; i++) {
			if ((i == selected_1) || (i == selected_2)){
				player_cup_order[i]->transform->position.z = player_cup_order[i]->base_position.z + 2.0f;
			} else if (i == current_cup_index) {
				player_cup_order[i]->transform->position.z = player_cup_order[i]->base_position.z + 0.2f;
			} else {
				player_cup_order[i]->transform->position.z = player_cup_order[i]->base_position.z;
			}
			if (player_cup_order[i]->color == target_order[i]->color) {
				correct_cups++;
			}
		}
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	space.downs = 0;
	enter.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text, display how many cups are correct
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		std::string num_right = std::to_string(correct_cups);
		std::string num_moves = std::to_string(moves);
		std::string display_string;
		if (correct_cups == 6) {
			display_string = "You won in " + num_moves + " moves!";
		} else {
			display_string = num_right + " cups correctly placed";
		}
		lines.draw_text(display_string,
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text(display_string,
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
