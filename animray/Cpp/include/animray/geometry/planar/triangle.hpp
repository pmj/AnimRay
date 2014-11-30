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


#ifndef ANIMRAY_PLANAR_TRIANGLE_HPP
#define ANIMRAY_PLANAR_TRIANGLE_HPP
#pragma once


#include <animray/maths/cross.hpp>
#include <animray/maths/dot.hpp>


namespace animray {


    template<typename I, typename D = typename I::local_coord_type>
    class triangle : private detail::array_based< point3d<D>, 3 > {
        typedef detail::array_based< point3d<D>, 3 > superclass;
    public:
        /// The type of the local coordinates used
        typedef D local_coord_type;
        /// Type of intersection to be returned
        typedef I intersection_type;
        /// The type of the corners
        typedef point3d<local_coord_type> corner_type;

        /// Construct a triangle from three points
        triangle(
            const corner_type &one,
            const corner_type &two,
            const corner_type &three
        ) {
            superclass::array[0] = one;
            superclass::array[1] = two;
            superclass::array[2] = three;
        }

        /// Calculate the intersection point
        template<typename R, typename E>
        fostlib::nullable< intersection_type > intersects(R by, const E epsilon) const {
            const corner_type e1(superclass::array[1] - superclass::array[0]);
            const corner_type e2(superclass::array[2] - superclass::array[0]);

            const corner_type cross_d_e2(cross(by.direction(), e2));
            const local_coord_type determinant(dot(e1, cross_d_e2));
            if ( determinant > -epsilon && determinant < epsilon ) {
                return fostlib::null;
            }
            const local_coord_type inv_determinant(local_coord_type(1) / determinant);

            const corner_type t(by.from() - superclass::array[0]);
            const local_coord_type u(dot(t, cross_d_e2) * inv_determinant);
            if ( u < local_coord_type() || u > local_coord_type(1) ) {
                return fostlib::null;
            }

            const corner_type q(cross(t, e1));
            const local_coord_type v(dot(by.direction(), q) * inv_determinant);
            if ( v < local_coord_type() || u + v > local_coord_type(1) ) {
                return fostlib::null;
            }

            const local_coord_type s(dot(e2, q) * inv_determinant);
            if ( s > epsilon ) {
                throw fostlib::exceptions::not_implemented("triangle::intersects");
            } else {
                return fostlib::null;
            }
        }

        /// Returns true if the ray hits the triangle
        template<typename R, typename E>
        bool occludes(R by, const E epsilon) const {
            return not intersects(by, epsilon).isnull();
        }
    };


}


#endif // ANIMRAY_PLANAR_TRIANGLE_HPP
