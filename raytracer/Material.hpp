//
// Created by chaku on 28/02/2026.
//

#ifndef THE_RAYTRACER_CHALLENGE_MATERIAL_HPP
#define THE_RAYTRACER_CHALLENGE_MATERIAL_HPP

#include "Colour.hpp"
#include "Light.hpp"
#include "Pattern.hpp"

namespace raytracer {
    struct Material {
        ColourOrPattern colour{Colour{.r = 1, .g = 1, .b = 1}};
        float ambient{0.1};
        float diffuse{0.9};
        float specular{0.9};
        float shininess{200.0};

        constexpr auto operator<=>(const Material &) const = default;
    };

    Colour lighting(const Material &material, const PointLight &light, const Point &point, const Vector &eye,
                    const Vector &normal, bool in_shadow);
}

#endif //THE_RAYTRACER_CHALLENGE_MATERIAL_HPP
