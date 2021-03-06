// -- includes --
#include "PoseFilterInterface.h"

// -- constants --
// Calibration Pose transform
const Eigen::Matrix3f g_eigen_identity_pose_upright = Eigen::Matrix3f::Identity();
const Eigen::Matrix3f *k_eigen_identity_pose_upright = &g_eigen_identity_pose_upright;

const Eigen::Matrix3f g_eigen_identity_pose_laying_flat((Eigen::Matrix3f() << 1,0,0, 0,0,-1, 0,1,0).finished());
const Eigen::Matrix3f *k_eigen_identity_pose_laying_flat = &g_eigen_identity_pose_laying_flat;

//Sensor Transforms
const Eigen::Matrix3f g_eigen_sensor_transform_identity = Eigen::Matrix3f::Identity();
const Eigen::Matrix3f *k_eigen_sensor_transform_identity = &g_eigen_sensor_transform_identity;

const Eigen::Matrix3f g_eigen_sensor_transform_opengl((Eigen::Matrix3f() << 1,0,0, 0,0,1, 0,-1,0).finished());
const Eigen::Matrix3f *k_eigen_sensor_transform_opengl= &g_eigen_sensor_transform_opengl;

// -- public interface -----
//-- Orientation Filter Space -----
PoseFilterSpace::PoseFilterSpace()
    : m_IdentityGravity(Eigen::Vector3f(0.f, 1.f, 0.f))
    , m_IdentityMagnetometer(Eigen::Vector3f(0.f, -1.f, 0.f))
    , m_CalibrationTransform(Eigen::Matrix3f::Identity())
    , m_SensorTransform(Eigen::Matrix3f::Identity())
{
}

Eigen::Vector3f PoseFilterSpace::getGravityCalibrationDirection() const
{
	// First apply the calibration data transform.
	// This allows us to pretend the "identity pose" was some other orientation the vertical during calibration
    const Eigen::Vector3f calibrationSpaceVector= m_CalibrationTransform * m_IdentityGravity;

	// Next apply the sensor data transform.
	// This allows us to pretend the sensors are in some other coordinate system (like OpenGL where +Y is up)
    const Eigen::Vector3f filterSpaceVector= m_SensorTransform * calibrationSpaceVector;

    return filterSpaceVector;
}

Eigen::Vector3f PoseFilterSpace::getMagnetometerCalibrationDirection() const
{
	// First apply the calibration data transform.
	// This allows us to pretend the "identity pose" was some other orientation the vertical during calibration
    const Eigen::Vector3f calibrationSpaceVector= m_CalibrationTransform * m_IdentityMagnetometer;

	// Next apply the sensor data transform.
	// This allows us to pretend the sensors are in some other coordinate system (like OpenGL where +Y is up)
    const Eigen::Vector3f filterSpaceVector= m_SensorTransform * calibrationSpaceVector;

    return filterSpaceVector;
}

void PoseFilterSpace::createFilterPacket(
    const PoseSensorPacket &sensorPacket,
	const Eigen::Quaternionf &orientation,
	const Eigen::Vector3f &position,
    PoseFilterPacket &outFilterPacket) const
{
	outFilterPacket.current_orientation= orientation;
	outFilterPacket.current_position_cm= position;

    outFilterPacket.optical_orientation = sensorPacket.optical_orientation;

	// Positional filtering is done is meters to improve numerical stability
    outFilterPacket.optical_position_cm = sensorPacket.optical_position_cm;
    outFilterPacket.tracking_projection_area= sensorPacket.tracking_projection_area;

    outFilterPacket.imu_gyroscope= m_SensorTransform * sensorPacket.imu_gyroscope;
    outFilterPacket.imu_accelerometer= m_SensorTransform * sensorPacket.imu_accelerometer;
    outFilterPacket.imu_magnetometer= m_SensorTransform * sensorPacket.imu_magnetometer;
        
	outFilterPacket.world_accelerometer=
		eigen_vector3f_clockwise_rotate(orientation, outFilterPacket.imu_accelerometer);	
}