//
// Created by chaku on 28/02/2026.
//

#ifndef THE_RAYTRACER_CHALLENGE_MATERIAL_HPP
#define THE_RAYTRACER_CHALLENGE_MATERIAL_HPP

#include "Colour.hpp"
#include "Light.hpp"
#include "Pattern.hpp"

#include <optional>

namespace raytracer {
    struct Shape;

    struct Material {
        Colour colour{.r = 1, .g = 1, .b = 1};
        std::optional<AnyPattern> pattern;
        float ambient{0.1};
        float diffuse{0.9};
        float specular{0.9};
        float shininess{200.0};
        float reflective{0.0f};
        float transparency{0.0f};
        float refractive_index{1.0f};

        bool operator==(const Material &) const = default;
    };

    Colour lighting(const Shape &shape, const Material &material, const PointLight &light, const Point &point,
                    const Vector &eye, const Vector &normal, bool in_shadow);
}

#endif //THE_RAYTRACER_CHALLENGE_MATERIAL_HPP
