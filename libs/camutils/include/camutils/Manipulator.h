/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CAMUTILS_MANIPULATOR_H
#define CAMUTILS_MANIPULATOR_H

#include <camutils/Bookmark.h>

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <cstdint>

namespace filament {
namespace camutils {

enum class Mode { ORBIT, MAP };

enum class MapConstraint { NONE, AXIS, FULL };

enum class Fov { VERTICAL, HORIZONTAL };

/**
 * Helper that enables interaction similar to sketchfab or Google Maps.
 *
 * This has no dependency on Filament or Filament's camera model. Instead, clients notify the
 * manipulator of various mouse or touch events, then periodically call its getLookAt() so
 * that they can adjust their camera(s) appopriately.
 *
 * Currently, two manipulator modes are supported: ORBIT and MAP. To construct a manipulator
 * instance, the desired mode is passed into the static \see create method.
 */
template <typename FLOAT>
class Manipulator {
public:
    using vec2 = filament::math::vec2<FLOAT>;
    using vec3 = filament::math::vec3<FLOAT>;
    using vec4 = filament::math::vec4<FLOAT>;
    using Bookmark = filament::camutils::Bookmark<FLOAT>;

    /**
     * Raycast callback. (map mode only)
     *
     * Users can optionally provide a raycasting function to enable perspective-correct panning.
     */
    typedef bool (*RayCallback)(const vec3& origin, const vec3& dir, FLOAT* t, void* userdata);

    /**
     * User-controlled properties that are never computed or changed by the manipulator.
     *
     * To prevent a proliferation of setters and getters, we decided to bundle these up into a
     * simple struct. If any field is zero-filled, it is automatically replaced with a reasonable
     * default value in setProperties().
     */
    struct Properties {
        /* Common properties */
        int viewport[2];     //! Width and height of the viewing area in terms of physical pixels.
        FLOAT zoomSpeed;     //! Multiplied with scroll delta to compute the zoom speed.
        vec3 homeTarget;     //! World-space position of interest.
        vec3 homeUpVector;   //! Orientation vector for "up" when in the home position.

        /* Raycast properties */
        vec4 groundPlane;            //! Plane equation that defaults to (0, 0, 1, 0)
        RayCallback raycastCallback; //! Optional raycast function (defaults to plane intersector)
        void* raycastUserdata;       //! Optional callback userdata (for closures)

        /* Orbit mode properties */
        vec3 homeVector;
        vec2 orbitSpeed;
        vec2 strafeSpeed;

        /* Map mode properties */
        Fov fovDirection;
        FLOAT fovDegrees;
        FLOAT farPlane;
        vec2 mapExtent;
        MapConstraint mapConstraint;
        FLOAT mapMinDistance;
    };

    static Manipulator* create(Mode mode, const Properties& props);

    virtual ~Manipulator() = default;

    Mode getMode() const { return mMode; }

    void setProperties(const Properties& props);

    const Properties& getProperties() const;

    void getLookAt(vec3* eyepos, vec3* target, vec3* upward) const;

    bool raycast(int x, int y, vec3* result) const;

    virtual void grabBegin(int x, int y, bool strafe) = 0;

    virtual void grabUpdate(int x, int y) = 0;

    virtual void grabEnd() = 0;

    virtual void zoom(int x, int y, FLOAT scrolldelta) = 0;

    virtual Bookmark getCurrentBookmark() const = 0;

    virtual Bookmark getHomeBookmark() const = 0;

    virtual void jumpToBookmark(const Bookmark& bookmark) = 0;

protected:
    Manipulator(Mode mode, const Properties& props);

    const Mode mMode;
    Properties mProps;
    vec3 mEye;
    vec3 mTarget;
};

} // namespace camutils
} // namespace filament

#endif /* CAMUTILS_MANIPULATOR_H */