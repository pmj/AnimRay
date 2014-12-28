/*
    Copyright 2014, Kirit Saelensminde.
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


#ifndef ANIMRAY_CAMERA_MOVIE_HPP
#define ANIMRAY_CAMERA_MOVIE_HPP
#pragma once


#include <animray/mixins/frame.hpp>


namespace animray {


    /// Movie cameras take a succession of shots
    template<typename C, typename F = std::size_t>
    class stacatto_movie {
    public:
        /// The type of the frame camera
        typedef C frame_camera_type;
        /// The type used to store a frame number
        typedef F frame_number_type;
        /// The type of ray generated by the camera
        typedef typename animray::with_frame<typename C::ray_type>::type ray_type;
        /// The underlying local co-ordinate type
        typedef typename ray_type::local_coord_type local_coord_type;

        /// The frame number the camera is working on
        fostlib::accessors<frame_number_type> frame;
        /// The camera used to expose a frame
        fostlib::accessors<frame_camera_type> frame_camera;

        /// Construct by passing all parameters to the contained camera
        template<typename... A>
        explicit stacatto_movie(A&&... args)
        : frame_camera(std::forward<A>(args)...) {
        }

        /// Allow the instance to be used as a camera
        template<typename S>
        ray_type operator() (S x, S y) const {
            ray_type ray{frame_camera()(x, y)};
            ray.at_frame(frame());
            return ray;
        }
    };


}


#endif // ANIMRAY_CAMERA_MOVIE_HPP
