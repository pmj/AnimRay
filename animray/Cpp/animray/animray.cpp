/*
    Copyright 2014-2018, Kirit Saelensminde.
    http://www.kirit.com/AnimRay

    This file is part of AnimRay.

    AnimRay is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AnimRay is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AnimRay.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <fost/main>
#include <fost/progress-cli>
#include <fost/unicode>
#include <animray/animation/procedural/rotate.hpp>
#include <animray/camera/flat-jitter.hpp>
#include <animray/camera/pinhole.hpp>
#include <animray/camera/movie.hpp>
#include <animray/geometry/planar/plane.hpp>
#include <animray/geometry/quadrics/sphere-unit.hpp>
#include <animray/geometry/collection-surface.hpp>
#include <animray/compound.hpp>
#include <animray/maths/angles.hpp>
#include <animray/movable.hpp>
#include <animray/intersection.hpp>
#include <animray/scene.hpp>
#include <animray/shader.hpp>
#include <animray/surface/matte.hpp>
#include <animray/surface/gloss.hpp>
#include <animray/surface/reflective.hpp>
#include <animray/light/ambient.hpp>
#include <animray/light/collection.hpp>
#include <animray/light/point.hpp>
#include <animray/targa.hpp>
#include <animray/affine.hpp>
#include <animray/threading/sub-panel.hpp>
#include <animray/color/hls.hpp>
#include <thread>


FSL_MAIN(
    "animray",
    "AnimRay. Copyright 2010-2018 Kirit Saelensminde"
)( fostlib::ostream &out, fostlib::arguments &args ) {
    const std::size_t threads(
        fostlib::coerce<fostlib::nullable<int>>(args.commandSwitch("t")).value(
            std::thread::hardware_concurrency()));
    const std::size_t samples(fostlib::coerce<int>(
        args.commandSwitch("ss").value("6")));
    const std::size_t spheres(fostlib::coerce<int>(
        args.commandSwitch("sp").value("20")));
    const std::size_t frames(fostlib::coerce<int>(
        args.commandSwitch("frames").value("60")));
    const std::size_t start_frame(fostlib::coerce<int>(
        args.commandSwitch("frames-start").value("0")));
    const std::size_t cycle(60);

    const int width = fostlib::coerce< int >( args[1].value("180") );
    const int height = fostlib::coerce< int >( args[2].value("135") );
    boost::filesystem::wpath output_filename =
        fostlib::coerce< boost::filesystem::wpath >(args[3].value("spheres-falling.tga"));

    typedef double world;
    const world aspect = double(width) / height;
    const world fw = width > height ? aspect * 0.024 : 0.024;
    const world fh = width > height ? 0.024 : 0.024 / aspect;

    const world speed(world(cycle) / world(frames));

    typedef std::function<
        animray::point3d<world>(
            typename animray::with_frame<animray::ray<world>, float>::frame_type)> position_function;
    typedef animray::surface<
            animray::unit_sphere<animray::animate<position_function>>,
            animray::gloss< world >,
            animray::matte< animray::rgb<float> >
        > gloss_sphere_type;
    typedef animray::scene<
        animray::collection<gloss_sphere_type>,
        animray::light<
            std::tuple<
                animray::light<void, float>,
                animray::light<
                    std::vector<
                        animray::light<animray::point3d<world>, animray::rgb<float>>>,
                    animray::rgb<float>>
            >, animray::rgb<float>
        >,
        animray::rgb<float>>
            scene_type;
    scene_type scene;
    scene.background(animray::rgb<float>(20, 70, 100));

    std::default_random_engine generator;
    std::uniform_int_distribution<int> phase(0, cycle);
    std::uniform_real_distribution<world> hue(0, 360), x_position(-10, 10), y_position(-20, 20);
    for ( auto count = 0u; count != spheres; ++count ) {
        animray::hls<float> hls_colour(hue(generator), 0.5f, 1.0f);
        auto colour(fostlib::coerce<animray::rgb<float>>(hls_colour));
        auto x_location(x_position(generator)), y_location(y_position(generator));
        auto start_phase(phase(generator));
        position_function position = [x_location, y_location, start_phase, frames, speed](
            typename animray::with_frame<animray::ray<world>, float>::frame_type frame
        ) {
            return animray::point3d<world>(
                x_location, y_location, world(-12) + std::fmod(start_phase + frame * speed, cycle));
        };
        animray::animate<position_function> location(position);
        gloss_sphere_type g(10.0f, colour);
        g.geometry().position((location));
        scene.geometry().insert(g);
    }

    std::get<0>(scene.light()).color(50);
    std::get<1>(scene.light()).push_back(
        animray::light<animray::point3d<world>, animray::rgb<float>>(
            animray::point3d<world>(-25.0, 25.0, -25.0),
            animray::rgb<float>(0x40, 0xa0, 0x40)));
    std::get<1>(scene.light()).push_back(
        animray::light<animray::point3d<world>, animray::rgb<float>>(
            animray::point3d<world>(-25.0, -25.0, -25.0),
            animray::rgb<float>(0xa0, 0x40, 0x40)));
    std::get<1>(scene.light()).push_back(
        animray::light<animray::point3d<world>, animray::rgb<float>>(
            animray::point3d<world>(25.0, -25.0, -25.0),
            animray::rgb<float>(0x40, 0x40, 0xa0)));

    for ( std::size_t frame{start_frame}; frame != frames; ++frame ) {
        animray::movable<
                animray::movie<
                    animray::pinhole_camera<
                        animray::ray<world>,
                        animray::flat_jitter_camera<world>
                    >
                >,
                typename animray::with_frame<animray::ray<world>, float>::type
            > camera(fw, fh, width, height, 0.05);
        camera
            (animray::rotate_x<world>(-65_deg))
            (animray::translate<world>(0.0, -4.0, -40));
        camera.instance().frame(frame);

        typedef animray::film<animray::rgb<uint8_t>> film_type;

        fostlib::worker worker;
        fostlib::meter tracking;
        fostlib::future<film_type> result(worker.run<film_type>(
            [threads, samples, width, height, &scene, &camera] () {
                return animray::threading::sub_panel<film_type>(
                    threads, width, height,
                    [samples, &scene, &camera](
                        const film_type::size_type x, const film_type::size_type y
                    ) {
                        animray::rgb<float> photons;
                        for ( std::size_t sample{}; sample != samples; ++sample ) {
                            photons += scene(camera, x, y) /= samples;
                        }
                        const float exposure = 1.4f;
                        photons /= exposure;
                        return animray::rgb<uint8_t>(
                            uint8_t(photons.red() > 255 ? 255 : photons.red()),
                            uint8_t(photons.green() > 255 ? 255 : photons.green()),
                            uint8_t(photons.blue() > 255 ? 255 : photons.blue()));
                    });
                }));
        fostlib::cli::monitor(out, tracking, result,
            [frame](const fostlib::meter::reading &current) {
                fostlib::stringstream out;
                out << "] f"  << frame << " "
                    << current.done() << "/" << current.work().value(0);
                if ( current.meta().size() && not current.meta()[0].isnull() ) {
                    fostlib::json meta(current.meta()[0]);
                    out << " (" << fostlib::json::unparse(meta["panels"]["x"], false)
                        << "x" << fostlib::json::unparse(meta["panels"]["y"], false)
                        << " of size " << fostlib::json::unparse(meta["size"]["x"], false)
                        << "x" << fostlib::json::unparse(meta["size"]["y"], false)
                        << ")";
                }
                return out.str();
            },
            [](const fostlib::meter::reading &) {
                return "[";
            });
        boost::filesystem::wpath filename(output_filename);
        filename.replace_extension(
            fostlib::coerce<boost::filesystem::wpath>(
                fostlib::coerce<fostlib::string>(frame) + ".tga"));
        animray::targa(filename, result());
    }

    return 0;
}

