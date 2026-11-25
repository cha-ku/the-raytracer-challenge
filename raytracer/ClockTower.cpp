#include "ClockTower.hpp"

#include "MatrixImpl.hpp"

#include <initializer_list>
#include <numbers>

// How the model is built:
//  - A Difference shows the cutter's material on the cut surface, so recesses take their colour from their cutter.
//  - A group used as a CSG child must hold parts that don't overlap: filter_intersections flips inside/outside on
//    every hit, so overlapping parts would be counted wrongly. Cutters that cross each other go in nested Differences.
//  - refresh_bounds() doesn't look inside a Csg, so make_cutters() caches the bounds of each group used as a cutter.
namespace raytracer {
    namespace {
        constexpr double pi{std::numbers::pi};

        Material matte(const Colour &colour) {
            Material material;
            material.colour = colour;
            material.specular = 0.1f;
            material.shininess = 20.0f;
            return material;
        }

        const Material stone{matte({.r = 0.80f, .g = 0.68f, .b = 0.46f})};
        const Material trim{matte({.r = 0.88f, .g = 0.78f, .b = 0.57f})};
        const Material recess{matte({.r = 0.55f, .g = 0.45f, .b = 0.29f})};
        const Material slate{matte({.r = 0.24f, .g = 0.27f, .b = 0.31f})};
        const Material dark{matte({.r = 0.06f, .g = 0.07f, .b = 0.12f})};

        Material make_gold() {
            Material material{matte({.r = 0.90f, .g = 0.68f, .b = 0.20f})};
            material.ambient = 0.2f;
            material.specular = 0.7f;
            material.shininess = 80.0f;
            return material;
        }

        // Lit from within, so mostly independent of the light
        Material make_glow(const Colour &colour) {
            Material material{matte(colour)};
            material.ambient = 0.6f;
            material.diffuse = 0.4f;
            material.specular = 0.0f;
            return material;
        }

        const Material gold{make_gold()};
        const Material dial{make_glow({.r = 0.97f, .g = 0.95f, .b = 0.86f})};
        const Material lamp{make_glow({.r = 1.0f, .g = 0.85f, .b = 0.45f})};

        // An axis-aligned box between two corners
        Geom &box(World &world, const Point &min, const Point &max, const Material &material) {
            Geom &geom = world.add(Cube{});
            geom.set_transform(multiply(
                translation<double>((min.x + max.x) / 2, (min.y + max.y) / 2, (min.z + max.z) / 2),
                scale<double>((max.x - min.x) / 2, (max.y - min.y) / 2, (max.z - min.z) / 2)));
            geom.material = material;
            return geom;
        }

        // A square block centred on the y axis
        Geom &block(World &world, const float half_width, const float bottom, const float top,
                    const Material &material) {
            return box(world, Point{.x = -half_width, .y = bottom, .z = -half_width},
                       Point{.x = half_width, .y = top, .z = half_width}, material);
        }

        // A closed cylinder along z, from z = front to z = back
        Geom &disc(World &world, const float x, const float y, const float radius, const float front,
                   const float back, const Material &material) {
            Geom &geom = world.add(Cylinder{.minimum = 0.0f, .maximum = 1.0f, .closed = true});
            // rotation_x(90°) turns the cylinder's +y towards +z
            geom.set_transform(multiply(translation<double>(x, y, front),
                                        multiply(rotation_x(pi / 2), scale<double>(radius, back - front, radius))));
            geom.material = material;
            return geom;
        }

        Geom &csg(World &world, const Csg::Operation op, Geom &left, Geom &right) {
            return world.add(Csg{op, left, right});
        }

        Geom &make_cutters(World &world, const std::initializer_list<Geom *> parts) {
            Geom &group = world.add(Group{});
            for (Geom *part: parts) {
                add_child(group, *part);
            }
            refresh_bounds(group);
            return group;
        }

        // A solid with a Gothic arch outline, from z = front to z = back: a rectangle topped by the lens where two
        // circles overlap, each centred on the other side of the opening. The tip is sqrt(3) * half_width above
        // `spring`, where the curve begins.
        Geom &arch(World &world, const float x, const float half_width, const float bottom, const float spring,
                   const float front, const float back, const Material &material) {
            Geom &lens = csg(world, Csg::Operation::Intersection,
                             disc(world, x - half_width, spring, 2 * half_width, front, back, material),
                             disc(world, x + half_width, spring, 2 * half_width, front, back, material));
            Geom &opening = box(world, Point{.x = x - half_width, .y = bottom, .z = front},
                                Point{.x = x + half_width, .y = spring, .z = back}, material);
            return csg(world, Csg::Operation::Union, opening, lens);
        }

        // Three arches side by side, which run along z through the whole tower
        Geom &arcade(World &world, const float spacing, const float half_width, const float bottom,
                     const float spring, const Material &material) {
            return make_cutters(world, {
                                    &arch(world, -spacing, half_width, bottom, spring, -2, 2, material),
                                    &arch(world, 0, half_width, bottom, spring, -2, 2, material),
                                    &arch(world, spacing, half_width, bottom, spring, -2, 2, material),
                                });
        }

        // `solid` with an arcade cut through it in both directions. The two arcades cross, hence two Differences.
        Geom &cut_arcades(World &world, Geom &solid, const float spacing, const float half_width, const float bottom,
                          const float spring, const Material &material) {
            Geom &across = arcade(world, spacing, half_width, bottom, spring, material);
            across.set_transform(rotation_y(pi / 2));
            Geom &cut = csg(world, Csg::Operation::Difference, solid,
                            arcade(world, spacing, half_width, bottom, spring, material));
            return csg(world, Csg::Operation::Difference, cut, across);
        }

        // A square pyramid on the y axis, cut off flat `visible` above its base. Two diamond-shaped prisms at right
        // angles (cubes turned 45°) intersect in a double pyramid, and a block keeps the part above the base.
        Geom &pyramid(World &world, const float half_width, const float base, const float height,
                      const float visible, const Material &material) {
            // A unit cube turned 45° reaches sqrt(2) along x and y
            const double w{half_width / std::numbers::sqrt2};
            const double h{height / std::numbers::sqrt2};
            const auto diamond{multiply(scale<double>(w, h, half_width * 2), rotation_z(pi / 4))};
            const auto lift{translation<double>(0, base, 0)};

            Geom &along_z = world.add(Cube{});
            along_z.set_transform(multiply(lift, diamond));
            along_z.material = material;
            Geom &along_x = world.add(Cube{});
            along_x.set_transform(multiply(lift, multiply(rotation_y(pi / 2), diamond)));
            along_x.material = material;

            Geom &double_pyramid = csg(world, Csg::Operation::Intersection, along_z, along_x);
            return csg(world, Csg::Operation::Intersection, double_pyramid,
                       block(world, half_width * 1.5f, base, base + visible, material));
        }

        // A thin bar standing proud of the dial, at `degrees` clockwise from 12, from radius `inner` to `outer`.
        // The box is drawn pointing at 12; a clockwise turn, seen from -z, is a negative rotation about z.
        Geom &dial_bar(World &world, const float centre_y, const float degrees, const float inner, const float outer,
                       const float half_width, const float front, const float back, const Material &material) {
            Geom &bar = box(world, Point{.x = -half_width, .y = inner, .z = front},
                            Point{.x = half_width, .y = outer, .z = back}, material);
            bar.set_transform(multiply(translation<double>(0, centre_y, 0),
                                       multiply(rotation_z(-degrees * pi / 180), bar.transform())));
            return bar;
        }

        // Plinth with a doorway on the front
        void add_base(World &world, Geom &tower) {
            Geom &plinth = block(world, 0.70f, 0.0f, 0.60f, stone);
            Geom &door = arch(world, 0, 0.13f, -0.1f, 0.25f, -1.0f, -0.6f, dark);
            add_child(tower, csg(world, Csg::Operation::Difference, plinth, door));
            add_child(tower, block(world, 0.74f, 0.0f, 0.08f, trim));
            add_child(tower, block(world, 0.73f, 0.60f, 0.68f, trim));
        }

        // Shaft with tall recessed panels on every side, corner buttresses and stone bands
        void add_shaft(World &world, Geom &tower) {
            Geom *shaft = &block(world, 0.60f, 0.6f, 5.4f, stone);
            for (int side = 0; side < 4; ++side) {
                Geom &panels = make_cutters(world, {
                    &box(world, Point(-0.42, 0.85, -0.7), Point(-0.28, 5.0, -0.55), recess),
                    &box(world, Point(-0.18, 0.85, -0.7), Point(-0.02, 5.0, -0.55), recess),
                    &box(world, Point(0.02, 0.85, -0.7), Point(0.18, 5.0, -0.55), recess),
                    &box(world, Point(0.28, 0.85, -0.7), Point(0.42, 5.0, -0.55), recess),
                });
                // Deeper slit windows inside the two middle panels
                Geom &windows = make_cutters(world, {
                    &arch(world, -0.1, 0.035, 1.2, 1.7, -0.7, -0.4, dark),
                    &arch(world, 0.1, 0.035, 1.2, 1.7, -0.7, -0.4, dark),
                    &arch(world, -0.1, 0.035, 2.6, 3.1, -0.7, -0.4, dark),
                    &arch(world, 0.1, 0.035, 2.6, 3.1, -0.7, -0.4, dark),
                    &arch(world, -0.1, 0.035, 4.0, 4.5, -0.7, -0.4, dark),
                    &arch(world, 0.1, 0.035, 4.0, 4.5, -0.7, -0.4, dark),
                });
                panels.set_transform(rotation_y(side * pi / 2));
                windows.set_transform(rotation_y(side * pi / 2));
                shaft = &csg(world, Csg::Operation::Difference, *shaft, panels);
                shaft = &csg(world, Csg::Operation::Difference, *shaft, windows);
            }
            // Give the shaft its own box, as nothing else culls a CSG
            add_child(tower, make_cutters(world, {shaft}));

            for (const float x: {-0.58f, 0.58f}) {
                for (const float z: {-0.58f, 0.58f}) {
                    add_child(tower, box(world, Point{.x = x - 0.08f, .y = 0.6f, .z = z - 0.08f},
                                         Point{.x = x + 0.08f, .y = 5.3f, .z = z + 0.08f}, stone));
                }
            }
            for (const float y: {2.2f, 3.6f, 5.0f}) {
                add_child(tower, block(world, 0.63f, y, y + 0.08f, trim));
            }
        }

        // The details of one dial, on the side facing -z. The dial itself is cut by add_clock_stage.
        Geom &dial_face(World &world, const float centre_y) {
            Geom &face = world.add(Group{});

            // Gilded square frame, and rings around the dial
            Geom &frame = box(world, Point(-0.66, centre_y - 0.66, -0.75), Point(0.66, centre_y + 0.66, -0.70), gold);
            Geom &frame_hole = box(world, Point(-0.61, centre_y - 0.61, -0.80), Point(0.61, centre_y + 0.61, -0.65),
                                   gold);
            add_child(face, csg(world, Csg::Operation::Difference, frame, frame_hole));
            add_child(face, csg(world, Csg::Operation::Difference,
                                disc(world, 0, centre_y, 0.57f, -0.74f, -0.65f, gold),
                                disc(world, 0, centre_y, 0.51f, -0.80f, -0.60f, gold)));
            add_child(face, csg(world, Csg::Operation::Difference,
                                disc(world, 0, centre_y, 0.36f, -0.675f, -0.65f, dark),
                                disc(world, 0, centre_y, 0.34f, -0.80f, -0.60f, dark)));

            for (int hour = 0; hour < 12; ++hour) {
                add_child(face, dial_bar(world, centre_y, 30.0f * hour, 0.38f, 0.49f, hour % 3 == 0 ? 0.022f : 0.012f,
                                         -0.68f, -0.65f, dark));
            }
            // Ten past ten
            add_child(face, dial_bar(world, centre_y, 305.0f, -0.06f, 0.29f, 0.025f, -0.70f, -0.685f, dark));
            add_child(face, dial_bar(world, centre_y, 60.0f, -0.10f, 0.46f, 0.016f, -0.72f, -0.705f, dark));
            add_child(face, disc(world, 0, centre_y, 0.04f, -0.73f, -0.65f, gold));
            return face;
        }

        // The wider stage that carries the four dials, each sunk into the stone by a white cylinder
        void add_clock_stage(World &world, Geom &tower) {
            constexpr float centre_y{6.2f};
            add_child(tower, block(world, 0.68f, 5.25f, 5.40f, trim));
            Geom *stage = &block(world, 0.72f, 5.40f, 7.0f, stone);
            for (int side = 0; side < 4; ++side) {
                const auto turn{rotation_y(side * pi / 2)};
                Geom &hole = disc(world, 0, centre_y, 0.55f, -0.9f, -0.66f, dial);
                hole.set_transform(multiply(turn, hole.transform()));
                stage = &csg(world, Csg::Operation::Difference, *stage, hole);

                Geom &face = dial_face(world, centre_y);
                face.set_transform(turn);
                add_child(tower, face);
            }
            add_child(tower, make_cutters(world, {stage}));
            add_child(tower, block(world, 0.80f, 7.0f, 7.12f, trim));
        }

        // Open belfry: arches on all sides and a chamber inside for the bell
        void add_belfry(World &world, Geom &tower) {
            Geom &walls = cut_arcades(world, block(world, 0.68f, 7.12f, 7.95f, stone), 0.38f, 0.14f, 7.22f, 7.52f,
                                      recess);
            Geom &belfry = csg(world, Csg::Operation::Difference, walls,
                               block(world, 0.50f, 7.22f, 7.85f, recess));
            add_child(tower, make_cutters(world, {&belfry}));

            // The bell: an egg shape with its lower half and a smaller egg taken out of it
            Geom &outside = world.add(Sphere{});
            outside.set_transform(multiply(translation<double>(0, 7.4, 0), scale<double>(0.28, 0.42, 0.28)));
            outside.material = gold;
            Geom &inside = world.add(Sphere{});
            inside.set_transform(multiply(translation<double>(0, 7.4, 0), scale<double>(0.24, 0.36, 0.24)));
            inside.material = gold;
            Geom &shell = csg(world, Csg::Operation::Difference, outside, inside);
            add_child(tower, csg(world, Csg::Operation::Difference, shell,
                                 block(world, 0.5f, 6.9f, 7.4f, gold)));

            add_child(tower, block(world, 0.78f, 7.95f, 8.05f, trim));
            for (const float x: {-0.68f, 0.68f}) {
                for (const float z: {-0.68f, 0.68f}) {
                    Geom &pinnacle = pyramid(world, 0.09f, 8.05f, 0.55f, 0.6f, trim);
                    pinnacle.set_transform(translation<double>(x, 0, z));
                    add_child(tower, pinnacle);
                }
            }
        }

        // Lower roof, the lit lantern above it, and the spire
        void add_roof(World &world, Geom &tower) {
            add_child(tower, pyramid(world, 0.72f, 8.05f, 2.0f, 0.75f, slate));
            add_child(tower, block(world, 0.50f, 8.78f, 8.86f, gold));

            Geom &lantern = cut_arcades(world, block(world, 0.42f, 8.86f, 9.35f, stone), 0.25f, 0.09f, 8.90f, 9.10f,
                                        recess);
            add_child(tower, make_cutters(world, {&lantern}));
            // The Ayrton light, glowing behind the arches
            add_child(tower, block(world, 0.30f, 8.87f, 9.34f, lamp));
            add_child(tower, block(world, 0.48f, 9.35f, 9.42f, gold));

            add_child(tower, pyramid(world, 0.43f, 9.42f, 1.7f, 1.7f, slate));
            // Finial: a gilded rod, a small pyramid on it and a cross
            add_child(tower, block(world, 0.022f, 10.95f, 11.5f, gold));
            add_child(tower, pyramid(world, 0.09f, 11.08f, 0.14f, 0.14f, gold));
            add_child(tower, box(world, Point(-0.12, 11.32, -0.022), Point(0.12, 11.365, 0.022), gold));
        }
    }

    Geom &add_clock_tower(World &world) {
        Geom &tower = world.add(Group{});
        add_base(world, tower);
        add_shaft(world, tower);
        add_clock_stage(world, tower);
        add_belfry(world, tower);
        add_roof(world, tower);
        refresh_bounds(tower);
        return tower;
    }
}
