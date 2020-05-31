/**
    Copyright 2010-2020, [Kirit Saelensminde](https://kirit.com/AnimRay).

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


#include <animray/color/hsl.hpp>

#include <animray/color/concept.hpp>
#include <fost/test>


FSL_TEST_SUITE(hsl);


static_assert(animray::Spectrum<animray::hsl<float>>);


namespace {
    void check(float h, float l, float s, float r, float g, float b) {
        animray::hsl<float> f(h, s, l);
        animray::rgb<float> t(fostlib::coerce<animray::rgb<float>>(f));
        FSL_CHECK_EQ(int(t.array()[0] * 1000 + .5f), int(r * 1000 + .5f));
        FSL_CHECK_EQ(int(t.array()[1] * 1000 + .5f), int(g * 1000 + .5f));
        FSL_CHECK_EQ(int(t.array()[2] * 1000 + .5f), int(b * 1000 + .5f));
    }
}
FSL_TEST_FUNCTION(to_rgb) {
    // Test data taken from http://en.wikipedia.org/wiki/HSL_and_HSV
    check(0, 0, 0, 0, 0, 0); // black
    check(0, 1, 0, 1, 1, 1); // white
    check(0, .5f, 0, .5f, .5f, .5f); // gray
    check(0, .5f, 1, 1, 0, 0); // red
    check(60, .375f, 1, .75f, .75f, 0); // olive
    check(49.5f, .497f, .893f, .941f, .785f, .053f); // orange
    check(240.5f, .607f, .290f, .495f, .493f, .721f); // light purple
}
