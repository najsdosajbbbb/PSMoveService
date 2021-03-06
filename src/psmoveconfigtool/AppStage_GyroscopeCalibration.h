#ifndef APP_STAGE_GYROSCOPE_CALIBRATION_H
#define APP_STAGE_GYROSCOPE_CALIBRATION_H

//-- includes -----
#include "AppStage.h"
#include "ClientGeometry.h"
#include "ClientPSMoveAPI.h"

#include <deque>
#include <chrono>

//-- pre-declarations -----

//-- definitions -----
class AppStage_GyroscopeCalibration : public AppStage
{
public:
    AppStage_GyroscopeCalibration(class App *app);
    virtual ~AppStage_GyroscopeCalibration();

    virtual void enter() override;
    virtual void exit() override;
    virtual void update() override;
    virtual void render() override;

    virtual void renderUI() override;

    static const char *APP_STAGE_NAME;

    inline void setBypassCalibrationFlag(bool bFlag)
    {
        m_bBypassCalibration = bFlag;
    }

protected:
	enum eCalibrationMenuState
	{
		inactive,

		pendingTrackingSpaceSettings,
		failedTrackingSpaceSettings,
		waitingForStreamStartResponse,
		failedStreamStart,
		waitForStable,
		measureBiasAndDrift,
		measureComplete,
		test
	};

	void setState(eCalibrationMenuState newState);
	void onExitState(eCalibrationMenuState newState);
	void onEnterState(eCalibrationMenuState newState);

	void request_tracking_space_settings();
	static void handle_tracking_space_settings_response(
		const ClientPSMoveAPI::ResponseMessage *response_message,
		void *userdata);
    void request_set_gyroscope_calibration(const float raw_drift, const float raw_variance);
    static void handle_acquire_controller(
        const ClientPSMoveAPI::ResponseMessage *response,
        void *userdata);
    void request_exit_to_app_stage(const char *app_stage_name);

private:

    eCalibrationMenuState m_menuState;
    bool m_bBypassCalibration;

    class ClientControllerView *m_controllerView;
    bool m_isControllerStreamActive;
    int m_lastControllerSeqNum;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastSampleTime;
    bool m_bLastSampleTimeValid;

    PSMoveIntVector3 m_lastRawGyroscope;
    PSMoveFloatVector3 m_lastCalibratedGyroscope;
    PSMoveFloatVector3 m_lastCalibratedAccelerometer;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_stableStartTime;
    bool m_bIsStable;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_resetPoseButtonPressTime;
	bool m_bResetPoseRequestSent;

    struct GyroscopeNoiseSamples *m_gyroNoiseSamples;
	float m_global_forward_degrees;
};

#endif // APP_STAGE_GYROSCOPE_CALIBRATION_H