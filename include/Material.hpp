//
// Created by chaku on 28/02/2026.
//

#ifndef THE_RAYTRACER_CHALLENGE_MATERIAL_HPP
#define THE_RAYTRACER_CHALLENGE_MATERIAL_HPP

#include "Colour.hpp"
#include "Light.hpp"

namespace raytracer {
    struct Material {
        Colour colour{1, 1, 1};
        float ambient{0.1};
        float diffuse{0.9};
        float specular{0.9};
        float shininess{200.0};

        constexpr auto operator<=>(const Material &) const = default;
    };

    Colour lighting(const Material &material, const PointLight &light, const Point &point, const Vector &eye,
                    const Vector &normal);
}

#endif //THE_RAYTRACER_CHALLENGE_MATERIAL_HPP
