#ifndef KALMAN_POSITION_FILTER_H
#define KALMAN_POSITION_FILTER_H

#include "PoseFilterInterface.h"

/// Kalman Position filter for Optical Pose + Gravity(Accelerometer)
class KalmanPositionFilter : public IPositionFilter
{
public:
	KalmanPositionFilter();
	virtual ~KalmanPositionFilter();

	bool init(const PositionFilterConstants &constant) override;
	bool init(const PositionFilterConstants &constant, const Eigen::Vector3f &initial_position) override;
	void update(const float delta_time, const PoseFilterPacket &packet) override;

	// -- IStateFilter --
	bool getIsStateValid() const override;
	void resetState() override;
	void recenterState(const Eigen::Vector3f& p_pose, const Eigen::Quaternionf& q_pose) override;

	// -- IPositionFilter ---
	Eigen::Vector3f getPosition(float time = 0.f) const override;
	Eigen::Vector3f getVelocity() const override;
	Eigen::Vector3f getAcceleration() const override;

protected:
	PositionFilterConstants m_constants;
	class KalmanPositionFilterImpl *m_filter;
};

#endif // KALMAN_POSITION_FILTER_H