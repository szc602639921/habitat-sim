// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef ESP_SENSOR_CAMERASENSOR_H_
#define ESP_SENSOR_CAMERASENSOR_H_

#include <Magnum/Math/ConfigurationValue.h>
#include "VisualSensor.h"
#include "esp/core/esp.h"

namespace esp {
namespace sensor {

struct CameraSensorSpec : public VisualSensorSpec {
  int channels = 4;
  // description of Sensor observation space as gym.spaces.Dict()
  std::string observationSpace = "";
  CameraSensorSpec();
  void sanityCheck() override;
  bool operator==(const CameraSensorSpec& a) const;
  ESP_SMART_POINTERS(CameraSensorSpec)
};

class CameraSensor : public VisualSensor {
 public:
  // constructor: the status of the camera sensor is "valid" after
  // construction;
  // user can use them immediately
  explicit CameraSensor(scene::SceneNode& cameraNode,
                        const CameraSensorSpec::ptr& spec);
  ~CameraSensor() override { LOG(INFO) << "Deconstructing CameraSensor"; }

  /** @brief Updates this sensor's CameraSensorSpec cameraSensorSpec_ to reflect
   * the passed new values
   *  @param[in] spec Instance of CameraSensorSpec that sensor will use to
   * update its own SensorSpec
   */
  void setProjectionParameters(const CameraSensorSpec& spec);

  /** @brief Returns pointer to member RenderCamera that CameraSensor will use
   * for rendering
   */
  gfx::RenderCamera* getRenderCamera() const override;

  /**
   * @brief Draws an observation to the frame buffer using simulator's renderer,
   * then reads the observation to the sensor's memory buffer
   * @return true if success, otherwise false (e.g., failed to draw or read
   * observation)
   * @param[in] sim Instance of Simulator class for which the observation needs
   *                to be drawn, obs Instance of Observation class in which the
   * observation will be stored
   */
  bool getObservation(sim::Simulator& sim, Observation& obs) override;

  /**
   * @brief Updates ObservationSpace space with spaceType, shape, and dataType
   * of this sensor. The information in space is later used to resize the
   * sensor's memory buffer if sensor is resized.
   * @return true if success, otherwise false
   * @param[in] space Instance of ObservationSpace class which will be updated
   * with information from this sensor
   */
  bool getObservationSpace(ObservationSpace& space) override;

  /**
   * @brief Returns the parameters needed to unproject depth for this sensor's
   * perspective projection model.
   * See @ref gfx::calculateDepthUnprojection
   */
  Corrade::Containers::Optional<Magnum::Vector2> depthUnprojection()
      const override;

  /**
   * @brief Draw an observation to the frame buffer using simulator's renderer
   * @return true if success, otherwise false (e.g., frame buffer is not set)
   * @param[in] sim Instance of Simulator class for which the observation needs
   *                to be drawn
   */
  bool drawObservation(sim::Simulator& sim) override;

  /**
   * @brief Modify the zoom matrix for perspective and ortho cameras
   * @param factor Modification amount.
   */
  void modifyZoom(float factor) {
    zoomMatrix_ =
        Magnum::Matrix4::scaling({factor, factor, 1.0f}) * zoomMatrix_;
    recomputeProjectionMatrix();
  }

  /**
   * @brief Resets the zoom on this CameraSensor to reflect current sensor spec
   * values.
   */
  void resetZoom() {
    zoomMatrix_ = Magnum::Matrix4(Magnum::Math::IdentityInit);
    recomputeProjectionMatrix();
  }

  // ======== Accessors ========

  /**
   * @brief Sets the FOV for this CameraSensor.  Only consumed by
   * pinhole/perspective cameras.
   * @param FOV desired FOV to set.
   */
  void setFOV(Mn::Deg FOV) {
    hfov_ = FOV;
    if (cameraSensorSpec_->sensorSubType != SensorSubType::Pinhole) {
      LOG(INFO)
          << "CameraSensor::setFOV : Only Perspective-base CameraSensors use "
             "FOV. Specified value saved but will not be consumed by this "
             "CameraSensor.";
    }
    recomputeBaseProjectionMatrix();
  }  // CameraSensor::setFOV

  /**
   * @brief Sets camera type and calculates appropriate size vector for
   * display.
   */
  void setCameraType(const SensorSubType& _cameraType) {
    CORRADE_ASSERT(_cameraType == SensorSubType::Pinhole ||
                       _cameraType == SensorSubType::Orthographic,
                   "CameraSensor::setCameraType(): _cameraType is not "
                   "SensorSubType "
                   "Pinhole or Orthographic", );
    cameraSensorSpec_->sensorSubType = _cameraType;
    recomputeBaseProjectionMatrix();
  }  // CameraSensor::setCameraType

  /**
   * @brief Returns the camera type of this Sensor
   */
  SensorSubType getCameraType() const {
    return cameraSensorSpec_->sensorSubType;
  }

  /**
   * @brief Sets width of this sensor's view port
   */
  void setWidth(int width) {
    CORRADE_ASSERT(width > 0,
                   "CameraSensor::setWidth(): resolution height and "
                   "width must be greater than 0", );
    cameraSensorSpec_->resolution[1] = width;
    recomputeBaseProjectionMatrix();
  }

  /**
   * @brief Sets height of this sensor's view port
   */
  void setHeight(int height) {
    CORRADE_ASSERT(height > 0,
                   "CameraSensor::setHeight(): resolution height and "
                   "width must be greater than 0", );
    cameraSensorSpec_->resolution[0] = height;
    recomputeBaseProjectionMatrix();
  }

  /**
   * @brief Sets near plane distance.
   */
  void setNear(float _near) {
    CORRADE_ASSERT(_near > 0,
                   "CameraSensor::setNear(): near plane distance must be "
                   "greater than 0", );
    near_ = _near;
    recomputeBaseProjectionMatrix();
  }
  float getNear() const { return near_; }

  /**
   * @brief Sets far plane distance.
   */
  void setFar(float _far) {
    CORRADE_ASSERT(
        _far > 0,
        "CameraSensor::setFar(): Far plane distance must be greater than 0", );
    far_ = _far;
    recomputeBaseProjectionMatrix();
  }
  float getFar() const { return far_; }

 protected:
  /**
   * @brief Recalculate the base projection matrix, based on camera type and
   * display size. This should be called only when camera type, size or
   * clipping planes change.
   */
  void recomputeBaseProjectionMatrix();

  /**
   * @brief Recalculate the projection Matrix used by this Camera Sensor,
   * which should be recomputeulated @ref zoomMatrix_ or @ref baseProjMatrix_
   * change.
   */
  void recomputeProjectionMatrix();

  /**
   * @brief Read the observation that was rendered by the simulator
   * @param[in,out] obs Instance of Observation class in which the observation
   * will be stored
   */
  virtual void readObservation(Observation& obs);

  /**
   * @brief This camera's projection matrix. Should be recomputeulated every
   * time size changes.
   */
  Magnum::Matrix4 projectionMatrix_;

  /**
   * @brief A base projection matrix based on camera's type and display size.
   */
  Magnum::Matrix4 baseProjMatrix_;

  /**
   * @brief A matrix to determine the zoom for the projection.
   */
  Magnum::Matrix4 zoomMatrix_;

  /** @brief size of near plane
   */
  Mn::Vector2 nearPlaneSize_;

  /** @brief raw pointer to member RenderCamera that CameraSensor will use for
   * rendering
   */
  gfx::RenderCamera* renderCamera_;

  CameraSensorSpec::ptr cameraSensorSpec_ =
      std::dynamic_pointer_cast<CameraSensorSpec>(spec_);

 public:
  ESP_SMART_POINTERS(CameraSensor)
};

}  // namespace sensor
}  // namespace esp

#endif  // ESP_SENSOR_CAMERASENSOR_H_
