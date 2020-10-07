// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include "LightAttributes.h"
namespace esp {
namespace metadata {
namespace attributes {

const std::string LightAttributes::JSONConfigTestString =
    R"({
  "lights":{
      "test":{
        "position": [2.5,0.1,3.8],
        "direction": [1.0,-1.0,1.0],
        "intensity": -0.1,
        "color": [2,1,-1],
        "type": "spot",
        "spot": {
          "innerConeAngle": -0.75,
          "outerConeAngle": -1.57
        }
      }
    }
  })";

LightAttributes::LightAttributes(const std::string& handle)
    : AbstractAttributes("LightAttributes", handle) {
  setPosition({0.0, 0.0, 0.0});
  setDirection({0.0, -1.0, 0.0});
  setColor({1.0, 1.0, 1.0});
  setIntensity(1.0);
  setType("point");
  // ignored for all but spot lights
  setInnerConeAngle(0.75);
  setOuterConeAngle(1.5);
}  // ctor

}  // namespace attributes
}  // namespace metadata
}  // namespace esp
