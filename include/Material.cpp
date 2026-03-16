//
// Created by chaku on 15/03/2026.
//

#include "Material.hpp"

namespace raytracer {
    Colour lighting(const Material &material, const PointLight &light, const Point &point, const Vector &eye,
        const Vector &normal)
    {
        // combine surface colour with the light's colour/intensity
        const auto &effective_colour{material.colour * light.intensity};

        // find direction to the light source
        const auto &light_vector{Vector::normalize(light.position - point)};

        // compute the ambient contribution
        const auto &ambient{effective_colour * material.ambient};

        // this is the cosine of the angle between light vector and normal. Negative number means light is on the other
        // side of the surface
        const auto &light_dot_normal{Vector::dot(light_vector, normal)};
        Colour diffuse{0, 0, 0};
        Colour specular{0, 0, 0};
        if (light_dot_normal > 0) {
            diffuse = effective_colour * material.diffuse * light_dot_normal;
            const auto& reflect{Vector::reflect(-light_vector, normal)};
            if (const auto reflect_dot_eye{Vector::dot(reflect, eye)}; reflect_dot_eye > 0) {
                const auto factor = std::pow(reflect_dot_eye, material.shininess);
                specular = light.intensity * material.specular * factor;
            }
        }
        return ambient + diffuse + specular;
    }

}
