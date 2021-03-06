#ifndef POSE_FILTER_INTERFACE_H
#define POSE_FILTER_INTERFACE_H

//-- includes -----
#include "MathEigen.h"

//-- constants -----
// Calibration Pose transform
extern const Eigen::Matrix3f *k_eigen_identity_pose_upright;
extern const Eigen::Matrix3f *k_eigen_identity_pose_laying_flat;

//Sensor Transforms
extern const Eigen::Matrix3f *k_eigen_sensor_transform_identity;
extern const Eigen::Matrix3f *k_eigen_sensor_transform_opengl;

// 1 g-unit is equal 980.66499997877 gal (cm/s�)
#define k_g_units_to_gal  980.665000f // gal (cm/s�)
#define k_g_units_to_ms2  9.80665000f // m/s�
#define k_ms2_to_g_units  1.f/9.80665000f // g-units

//HACK - DO NOT CHECK IN
#define k_meters_to_centimeters  1.f//100.f
#define k_centimeters_to_meters  1.f//0.01f

//-- declarations -----
struct ExponentialCurve
{
	float A;
	float B;
	float MaxValue;

	inline float evaluate(float x) const
	{
		return fminf(A*exp(B*x), MaxValue);
	}
};

/// A snapshot of IMU data emitted from a controller
/// Intended to only exist on the stack.
struct PoseSensorPacket
{
    // Optical readings in the world reference frame
    Eigen::Vector3f optical_position_cm; // cm
    Eigen::Quaternionf optical_orientation;
    float tracking_projection_area; // pixels^2

    // Sensor readings in the controller's reference frame
    Eigen::Vector3f imu_accelerometer; // g-units
    Eigen::Vector3f imu_magnetometer; // unit vector
    Eigen::Vector3f imu_gyroscope; // rad/s

	inline Eigen::Vector3f get_optical_position_in_meters() const
	{
		return optical_position_cm * k_centimeters_to_meters;
	}
};

/// A snapshot of IMU data transformed into a world space plus world space calibration vectors
/// used to update a state filter
struct PoseFilterPacket : PoseSensorPacket
{
    /// The current orientation of the controller
    Eigen::Quaternionf current_orientation;

    /// The current position of the controller
    Eigen::Vector3f current_position_cm;

    /// The accelerometer reading in the world reference frame
    Eigen::Vector3f world_accelerometer; // g-units

	inline Eigen::Vector3f get_current_position_in_meters() const
	{
		return current_position_cm * k_centimeters_to_meters;
	}
};

/// Used to transform sensor data from a controller into an arbitrary space
class PoseFilterSpace
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    
    PoseFilterSpace();

    inline void setIdentityGravity(const Eigen::Vector3f &identityGravity)
    { m_IdentityGravity= identityGravity; }
    inline void setIdentityMagnetometer(const Eigen::Vector3f &identityMagnetometer)
    { m_IdentityMagnetometer= identityMagnetometer; }

    inline void setCalibrationTransform(const Eigen::Matrix3f &calibrationTransform)
    { m_CalibrationTransform= calibrationTransform; }
    inline void setSensorTransform(const Eigen::Matrix3f &sensorTransform)
    { m_SensorTransform= sensorTransform; }

    Eigen::Vector3f getGravityCalibrationDirection() const;
    Eigen::Vector3f getMagnetometerCalibrationDirection() const;

    void createFilterPacket(
        const PoseSensorPacket &sensorPacket,
        const Eigen::Quaternionf &orientation,
        const Eigen::Vector3f &position,
        PoseFilterPacket &outFilterPacket) const;

private:
    Eigen::Vector3f m_IdentityGravity;
    Eigen::Vector3f m_IdentityMagnetometer;

    Eigen::Matrix3f m_CalibrationTransform;
    Eigen::Matrix3f m_SensorTransform;
};

/// Filter parameters that remain constant during the lifetime of the the filter
struct OrientationFilterConstants 
{
    /// The direction of gravity when the controller is in it's calibration pose
    Eigen::Vector3f gravity_calibration_direction; // unit vector

    /// The direction of the magnetic field when the controller is in it's calibration pose
    Eigen::Vector3f magnetometer_calibration_direction; // unit vector

    /// The average time delta between position updates during calibration
    float mean_update_time_delta; // seconds

	/// Best fit parameters for variance as a function of screen projection area
	ExponentialCurve orientation_variance_curve;

	/// The variance of the accelerometer over a short period
	Eigen::Vector3f accelerometer_variance; // g-units^2

	/// The drift of the accelerometer over a long period
	Eigen::Vector3f accelerometer_drift; // g-units/s

    /// The variance of the gyroscope over a short period
	Eigen::Vector3f gyro_variance; // (rad/s)^2

    /// The drift of the gyroscope over a long period
	Eigen::Vector3f gyro_drift; // rad/s

	/// The variance of the magnetometer
	Eigen::Vector3f magnetometer_variance; // units^2

	/// The drift of the magnetometer (usually zero)
	Eigen::Vector3f magnetometer_drift; // units^2
};

/// Filter parameters that remain constant during the lifetime of the the filter
struct PositionFilterConstants 
{
    /// The direction of gravity when the controller is in it's calibration pose
    Eigen::Vector3f gravity_calibration_direction; // unit vector

    float accelerometer_noise_radius; // meters
	Eigen::Vector3f accelerometer_variance; // g-units^2
	Eigen::Vector3f accelerometer_drift; // g-units
    float max_velocity; // meters/s^2

    /// The average time delta between position updates during calibration
    float mean_update_time_delta; // seconds

    /// Best fit parameters for variance as a function of screen projection area
	ExponentialCurve position_variance_curve;
};

/// Filter parameters that remain constant during the lifetime of the the filter
struct PoseFilterConstants 
{
    OrientationFilterConstants orientation_constants;
    PositionFilterConstants position_constants;
};

/// Common interface to all state filters
class IStateFilter
{
public:
    /// Not true until the filter has updated at least once
    virtual bool getIsStateValid() const = 0;

    /// Update the state in the filter given the filter packet
    virtual void update(const float delta_time, const PoseFilterPacket &packet) = 0;

    /// Clears the current history of the filter
    virtual void resetState() = 0;

    /// The current state becomes the identity pose
    virtual void recenterState(const Eigen::Vector3f& p_pose, const Eigen::Quaternionf& q_pose) = 0;
};

/// Common interface to all orientation filters
class IOrientationFilter : public IStateFilter
{
public:
    virtual bool init(const OrientationFilterConstants &constant) = 0;
	virtual bool init(const OrientationFilterConstants &constant, const Eigen::Quaternionf &initial_orientation) = 0;

    /// Estimate the current orientation of the filter given a time offset into the future
    virtual Eigen::Quaternionf getOrientation(float time = 0.f) const = 0;

    /// Get the current world space angular velocity of the filter state (rad/s)
    virtual Eigen::Vector3f getAngularVelocity() const = 0;

    /// Get the current world space angular acceleration of the filter state (rad/s^2)
    virtual Eigen::Vector3f getAngularAcceleration() const = 0;
};

/// Common interface to all position filters
class IPositionFilter : public IStateFilter
{
public:
    virtual bool init(const PositionFilterConstants &constant) = 0;
	virtual bool init(const PositionFilterConstants &constant, const Eigen::Vector3f &initial_position) = 0;

    /// Estimate the current position of the filter state given a time offset into the future (meters)
    virtual Eigen::Vector3f getPosition(float time = 0.f) const = 0;

    /// Get the current velocity of the filter state (m/s)
    virtual Eigen::Vector3f getVelocity() const = 0;

    /// Get the current velocity of the filter state (m/s^2)
    virtual Eigen::Vector3f getAcceleration() const = 0;
};

/// Common interface to all pose filters (filter orientation and position simultaneously)
class IPoseFilter : public IStateFilter
{
public:
    /// Estimate the current orientation of the filter given a time offset into the future
    virtual Eigen::Quaternionf getOrientation(float time = 0.f) const = 0;

    /// Get the current world space angular velocity of the filter state (rad/s)
    virtual Eigen::Vector3f getAngularVelocity() const = 0;

    /// Get the current world space angular acceleration of the filter state (rad/s^2)
    virtual Eigen::Vector3f getAngularAcceleration() const = 0;

    /// Estimate the current position of the filter state given a time offset into the future (meters)
    virtual Eigen::Vector3f getPosition(float time = 0.f) const = 0;

    /// Get the current velocity of the filter state (m/s)
    virtual Eigen::Vector3f getVelocity() const = 0;

    /// Get the current velocity of the filter state (m/s^2)
    virtual Eigen::Vector3f getAcceleration() const = 0;
};

#endif // POSE_FILTER_INTERFACE_H