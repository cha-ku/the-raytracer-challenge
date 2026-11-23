//
// Created by chaku on 14/06/25.
//

#ifndef THE_RAYTRACER_CHALLENGE_SIMULATION_HPP
#define THE_RAYTRACER_CHALLENGE_SIMULATION_HPP

#include <string>

void simulate_projectile();

void simulate_clock();

void simulate_sphere();

void simulate_material_sphere();

void simulate_multiple_spheres();

void simulate_group_reflections();

// Renders the OBJ model at `obj_path` on a checkered floor, saved as <file stem>.ppm
void simulate_model(const std::string &obj_path);

#endif //THE_RAYTRACER_CHALLENGE_SIMULATION_HPP
