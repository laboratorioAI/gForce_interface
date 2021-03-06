/*This file contains auxiliar functions to the C++ MEX function*/

#pragma once
#include <windows.h>
#include "mexForce.h"
#include "mex.hpp"
#include "mexAdapter.hpp"

using namespace gf;
using namespace std;


///\brief Converts some returned code to a string
string code2string(GForceHandle::StatusConection res)
{
	string resp;
	switch (res)
	{
	case GForceHandle::StatusConection::idle:
		resp = "idle";
		break;
	case GForceHandle::StatusConection::Scanning:
		resp = "Scanning";
		break;
	case GForceHandle::StatusConection::Connecting:
		resp = "Connecting";
		break;
	case GForceHandle::StatusConection::Paired:
		resp = "Paired";
		break;
	case GForceHandle::StatusConection::Connected:
		resp = "Connected";
		break;
	case GForceHandle::StatusConection::Disconnected:
		resp = "Disconnected";
		break;
	case GForceHandle::StatusConection::Disconnecting:
		resp = "Disconnecting";
		break;
	case GForceHandle::StatusConection::Unknown:
		resp = "Unknown";
		break;
	case GForceHandle::StatusConection::Configuring:
		resp = "Configuring";
		break;
	case GForceHandle::StatusConection::Configured:
		resp = "Configured";
		break;
	default:
		break;
	}
	return resp;
}




///\brief Converts some returned code to a string
string code2string(gf::DeviceConnectionStatus res)
{
	switch (static_cast<GF_UINT32>(res))
	{
	case 0:
		return "disconnected";
		break;
	case 1:
		return "disconneting";
		break;

	case 2:
		return "Connecting";
		break;
	case 3:
		return "connected";
		break;
	default:
		return "???";
		break;
	}
}

///\brief Converts some returned code to a string
string code2string(DeviceSetting::DeviceStatusFlags res)
{
	UINT16 resp = (static_cast<UINT16>(res));
	switch (resp)
	{
	case 0:// DeviceSetting::DeviceStatusFlags::DSF_NONE:
		return "None";
		break;
	case 1://DeviceSetting::DeviceStatusFlags::DSF_RECENTER:
		return "Recenter";
		break;
	case 2: // DeviceSetting::DeviceStatusFlags::DSF_USBSTATUS:
		return "usb_status";
		break;
	case 4: //DeviceSetting::DeviceStatusFlags::DSF_MOTIONLESS:
		return "motionless";
		break;

	default:
		return to_string(resp) + "???";
		break;
	}
}

///\brief Converts some returned code to a string
string code2string(gf::ResponseResult res)
{
	switch (static_cast<GF_UINT32>(res))
	{
	case 0:
		return "success";
		break;
	case 1:
		return "not support";
		break;

	case 2:
		return "bad param";
		break;
	case 3:
		return "failed";
		break;
	case 4:
		return "timeout";
		break;
	default:
		return "???";
		break;
	}
}

///\brief Converts some returned code to a string
string code2string(gf::HubState res)
{
	switch (static_cast<GF_UINT32>(res))
	{
	case 0:
		return "Idle";
		break;
	case 1:
		return "Scanning";
		break;

	case 2:
		return "Connecting";
		break;
	case 3:
		return "Disconnected";
		break;
	case 4:
		return "Unknown";
		break;
	default:
		return "???";
		break;
	}
}

///\brief Converts some returned code to a string
string code2string(GF_RET_CODE res)
{
	switch (static_cast<GF_UINT32>(res))
	{
	case 0:
		return "success";
		break;
	case 1:
		return "error";
		break;

	case 2:
		return "errorbad param";
		break;
	case 3:
		return "error: bad state";
		break;
	case 4:
		return "error: not support";
		break;
	case 5:
		return "error: scan bussy";
		break;
	case 6:
		return "error: no resource";
		break;
	case 7:
		return "error: time out";
		break;
	case 8:
		return "error: device busy";
		break;
	case 9:
		return "error: not ready";
		break;
	default:
		return "???";
		break;
	}
}


class MexFunction : public matlab::mex::Function {
public:
	//---Properties
	enum class Status {
		// it is first call, it must be created
		INITIALIZED = 0,

		// it was already started 
		TRANSMITING = 1,

		// error
		FAILED = 3
	};

	// matlab own
	std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr;
	matlab::data::ArrayFactory factory;

	// interface
	int configurationMs = 1000; // time watining to config device
	int mSWaitRequests = 5000; // the requests have a timeout
	bool showMsjs = true;

	// GForce dependencies	
	gfsPtr<Hub> pHub;
	gfsPtr<HubListener> listener;
	gfsPtr<GForceHandle> gforceHandle;

	// auxiliar GForce_mex
	Status status = Status::INITIALIZED;


	// times ms (used in start_transimission)
	UINT transmitionDelay = 10000; // time that workMode::Pulling is running before changing to WorkMode::Freerun
	//UINT transmitionDelay = 30000; // time that workMode::Pulling is running before changing to WorkMode::Freerun
	UINT scanTimeOut = 500; // time that devices are searched.


	/// \brief constructor. It starts the matlab engine and the connection with the device!
	MexFunction() {
		matlabPtr = getEngine();

		// Try to transmit!
		GF_RET_CODE statusGForce = GF_RET_CODE::GF_ERROR; // prealloc
		try {
			statusGForce = startTransmission();
		}
		catch (...) {
			disp("WARNING: Could not start transmission");
		}

		if (statusGForce == GF_RET_CODE::GF_SUCCESS || statusGForce == GF_RET_CODE::GF_ERROR_TIMEOUT)
			status = Status::TRANSMITING;
		else
		{
			status = Status::FAILED;
			matlabError("FATAL ERROR!Not transmitting!!! The device may be disconnected. Clean, reconnect, turn on, turn off and/or try again!");

		}
	}
	/*Methods definition*/



	/// \brief Handles the promise to reconfigure the Emg. Uses the defined EMG params in gForce. By design, all the channels are requested. Data length is 128 unchangable.
	bool requested_configEmg();

	/// \brief Handles the promise to enable or disable the Quats.
	bool requested_disableQuats(bool isEnable);

	/// \brief Handles the promise to get the battery.
	/// \return The percentage of the battery. In case something went wrong returns -1.
	int requested_Battery();

	/// \brief Handles the request to get the temperature.
	/// \return Temperature, in case something went wrong returns -273
	//int requested_Temperature();

	/// \brief matlab requested the device to vibrate time ms, 
	/// \param time: in ms
	/// \return GF_code: default is error not support.
	gf::ResponseResult vibrate(UINT time);

	/// \brief matlab requested the device to power off.
	/// \return GF_code
	GF_RET_CODE turnOff();

	/// \brief check that only 1 input argument exists, else, raises matlab error.
	void check_noMoreInputs(matlab::mex::ArgumentList inputs);

	/// \brief Checks that there is a second DOUBLE input command. Raises matlab error, otherwise.
	void check_secondInput(matlab::mex::ArgumentList inputs);

	/// \brief Shows a message in matlab command window. Can be disabled by reseting showMsjs flag
	void disp(const string msj);

	/// \brief Raises a matlab error with the given message
	void matlabError(const string msj);

	/// \brief Checks that there is at least 1 input and that the 1st one is a vector char. Raises a matlab error, otherwise.	
	void check_matlabCommand(matlab::mex::ArgumentList inputs);

	/// \brief Checks that for some commands there is maximum one output argument
	void check_uniqueOutput(matlab::mex::ArgumentList outputs);

	/// \brief check that for some commands there is no output argument. Raises matlab error, otherwise.
	void check_noOutput(matlab::mex::ArgumentList outputs);

	/// \brief Checks that there is maximum 3 output arguments. Raises matlab error, otherwise.
	void check_3Outputs(matlab::mex::ArgumentList outputs);

	/// \brief Handles the request to the status of the device
	string requested_deviceStatus();

	/// \brief Method that does everything required to start transmitting data.
	GF_RET_CODE startTransmission();


	/// \brief Entry point of the Mex function. It switchs the funcitonality based on matlab command
	/// \return 
	void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs);

	void cleanUp();


	// /////////////////////////////////////////
	~MexFunction() {
		cleanUp();
	}
};



// /////////////////////////////////////////
// Class methods extension
//NOTE: no se ha puesto try catch en los requests
bool MexFunction::requested_configEmg()
{
	promise<gf::ResponseResult> aPromise;
	auto theFuture = aPromise.get_future();

	// result to be returned.
	bool result = false; //prealloc
	if (nullptr != gforceHandle->ds)
	{
		disp("Asking to change Emg configuration");

		size_t bitR = 8; // prealloc
		if (gforceHandle->bitResolution == GForceHandle::ADCResolution::_12bits)
			bitR = 12;

		GF_RET_CODE respPromise = gforceHandle->ds->setEMGRawDataConfig(
			(GF_UINT16)gforceHandle->frequency,						//sample rate
			(DeviceSetting::EMGRowDataChannels)(0x00FF),	//channel 0~7
			128,											//data length
			(GF_UINT8)bitR,											// adc resolution			
			[&aPromise, &result](gf::ResponseResult inResult) {

				if (inResult == gf::ResponseResult::RREST_SUCCESS)
					result = true;
				aPromise.set_value(inResult);
			});

		//waiting
		disp("Result of emg config is: " + code2string(respPromise));
		if (respPromise == GF_RET_CODE::GF_SUCCESS)
		{
			disp("Waiting.");
			std::future_status status = theFuture.wait_for(std::chrono::milliseconds(mSWaitRequests));
			if (status == std::future_status::ready) {
				gf::ResponseResult theResponse = theFuture.get();
				disp("Change Emg configuration state is: " + code2string(theResponse));
			}
			else {
				disp("WARNING: timeout of request");
			}
		}
		else
		{
			disp("WARNING: bad state");
		}
	}
	else
	{
		disp("WARNING: could not change Emg configuration!");
	}

	return result;
}


bool MexFunction::requested_disableQuats(bool isEnable)
{
	promise<gf::ResponseResult> aPromise;
	auto theFuture = aPromise.get_future();

	// result to be returned.
	bool result = false; //prealloc
	if (nullptr != gforceHandle->ds)
	{
		if (isEnable)
			disp("Asking to enable quaternions");
		else
			disp("Asking to disable quaternions");

		// prealloc. no quats
		DeviceSetting::DataNotifFlags flags = (DeviceSetting::DataNotifFlags)(
			DeviceSetting::DNF_OFF
			//| DeviceSetting::DNF_ACCELERATE
			//| DeviceSetting::DNF_GYROSCOPE
			//| DeviceSetting::DNF_MAGNETOMETER
			//| DeviceSetting::DNF_EULERANGLE
			//| DeviceSetting::DNF_QUATERNION
			//| DeviceSetting::DNF_ROTATIONMATRIX
			| DeviceSetting::DNF_EMG_GESTURE
			| DeviceSetting::DNF_EMG_RAW
			//| DeviceSetting::DNF_HID_MOUSE
			//| DeviceSetting::DNF_HID_JOYSTICK
			| DeviceSetting::DNF_DEVICE_STATUS
			);

		if (isEnable)
			flags = (DeviceSetting::DataNotifFlags)(
				DeviceSetting::DNF_OFF
				//| DeviceSetting::DNF_ACCELERATE
				//| DeviceSetting::DNF_GYROSCOPE
				//| DeviceSetting::DNF_MAGNETOMETER
				//| DeviceSetting::DNF_EULERANGLE
				| DeviceSetting::DNF_QUATERNION
				//| DeviceSetting::DNF_ROTATIONMATRIX
				| DeviceSetting::DNF_EMG_GESTURE
				| DeviceSetting::DNF_EMG_RAW
				//| DeviceSetting::DNF_HID_MOUSE
				//| DeviceSetting::DNF_HID_JOYSTICK
				| DeviceSetting::DNF_DEVICE_STATUS
				);

		GF_RET_CODE respPromise = gforceHandle->ds->setDataNotifSwitch(flags,
			[&aPromise, &result](gf::ResponseResult inResult) {

				if (inResult == gf::ResponseResult::RREST_SUCCESS)
					result = true;
				aPromise.set_value(inResult);
			});

		//waiting
		disp("Result of quaternions is: " + code2string(respPromise));
		if (respPromise == GF_RET_CODE::GF_SUCCESS)
		{
			disp("Waiting.");
			std::future_status status = theFuture.wait_for(std::chrono::milliseconds(mSWaitRequests));
			if (status == std::future_status::ready) {
				gf::ResponseResult theResponse = theFuture.get();
				disp("Quaternions Setting state is: " + code2string(theResponse));
			}
			else {
				disp("WARNING: timeout of request");
			}
		}
		else {
			disp("WARNING: bad state");
		}
	}
	else
	{
		disp("WARNING: device not defined! could not change quaternions settings!");
	}
	gforceHandle->quatData.clear();
	return result;
}


string MexFunction::requested_deviceStatus() {
	string resp = "-.-";
	if (gforceHandle->ds != nullptr) {
		promise<gf::ResponseResult> aPromise;
		auto theFuture = aPromise.get_future();
		DeviceSetting::DeviceStatusFlags result;
		disp("Asking device Status Cap");

		GF_RET_CODE respPromise = gforceHandle->ds->getDeviceStatusCap(
			[&aPromise, &result](gf::ResponseResult inResult, DeviceSetting::DeviceStatusFlags flags) {

				if (inResult == gf::ResponseResult::RREST_SUCCESS)
					result = flags;
				aPromise.set_value(inResult);
			});

		//waiting
		disp("Result of get device status is: " + code2string(respPromise));
		if (respPromise == GF_RET_CODE::GF_SUCCESS)
		{
			disp("Waiting.");
			std::future_status status = theFuture.wait_for(std::chrono::milliseconds(mSWaitRequests));
			if (status == std::future_status::ready) {
				gf::ResponseResult theResponse = theFuture.get();
				disp("Device status state is: " + code2string(theResponse));
				resp = code2string(result);
			}
			else {
				disp("WARNING: timeout of request");
			}
		}
		else {
			disp("WARNING: bad state");
		}
	}
	else {
		disp("WARNING: device not defined! Nullprt");
	}
	return resp;
}

int MexFunction::requested_Battery()
{
	promise<gf::ResponseResult> aPromise;
	auto theFuture = aPromise.get_future();

	// batery
	int battery = -1;
	if (nullptr != gforceHandle->ds)
	{
		disp("Asking battery level.");
		//
		GF_RET_CODE respPromise = gforceHandle->ds->getBatteryLevel([&aPromise, &battery](gf::ResponseResult result, GF_UINT32 percentage) {

			if (result == gf::ResponseResult::RREST_SUCCESS)
				battery = percentage;
			aPromise.set_value(result);
			});

		//waiting
		disp("Result of async func bat is: " + code2string(respPromise));
		if (respPromise == GF_RET_CODE::GF_SUCCESS)
		{
			disp("Waiting battery.");
			std::future_status status = theFuture.wait_for(std::chrono::milliseconds(mSWaitRequests));
			if (status == std::future_status::ready) {
				gf::ResponseResult batResponse = theFuture.get();
				disp("Battery state is: " + code2string(batResponse));
			}
			else {
				disp("WARNING: timeout of request");
			}
		}
		else {
			disp("WARNING: bad state");
		}
	}
	else
	{
		disp("WARNING: device not defined! could not get battery level!");
	}

	return battery;
}

/* //Temperature
int MexFunction::requested_Temperature()
{
	promise<gf::ResponseResult> tempePromise;
	auto tempeFuture = tempePromise.get_future();

	// request
	int temperature = -273;
	if (nullptr != gforceHandle->ds)
	{
		disp("asking temperature");
		GF_RET_CODE respTempe = gforceHandle->ds->getTemperature([&tempePromise, &temperature](gf::ResponseResult result, GF_UINT32 val) {

			if (result == gf::ResponseResult::RREST_SUCCESS)
				temperature = val;
			tempePromise.set_value(result);
			});

		//waiting
		disp("result of async func temperature is: " + code2string(respTempe));
		disp("waiting temperature");
		gf::ResponseResult tempeResponse = tempeFuture.get();
		disp("Temperature state is: " + code2string(tempeResponse));
	}
	else
	{
		disp("WARNING: device not defined! could not get temperature!");
		//cleanUp();
	}

	return temperature;
}
*/

gf::ResponseResult MexFunction::vibrate(UINT time = 100)
{
	gf::ResponseResult respVibrate = gf::ResponseResult::RREST_NOT_SUPPORT; //prealloc

	if (nullptr != gforceHandle->ds)
	{
		promise<gf::ResponseResult> aPromise;
		auto theFuture = aPromise.get_future();

		DeviceSetting::VibrateControlType type = DeviceSetting::VibrateControlType::On;

		disp("asking to vibrate");
		GF_RET_CODE respVibrate1 = gforceHandle->ds->vibrateControl(type, [&aPromise](gf::ResponseResult result) {
			aPromise.set_value(result);
			});

		//waiting
		disp("result of async func vibrate is: " + code2string(respVibrate1));
		if (respVibrate1 == GF_RET_CODE::GF_SUCCESS)
		{
			disp("waiting vibration");
			std::future_status status = theFuture.wait_for(std::chrono::milliseconds(mSWaitRequests));
			if (status == std::future_status::ready) {
				respVibrate = theFuture.get();
				disp("vibration state is: " + code2string(respVibrate));
				if (respVibrate == gf::ResponseResult::RREST_SUCCESS)
				{
					disp("Vibrating");
					Sleep(time);

					// stopping
					disp("asking to stop");
					promise<gf::ResponseResult> aPromise2;
					auto theFuture2 = aPromise2.get_future();

					type = DeviceSetting::VibrateControlType::Off;
					respVibrate1 = gforceHandle->ds->vibrateControl(type, [&aPromise2](gf::ResponseResult result) {
						aPromise2.set_value(result);
						});

					//waiting
					disp("result of async func vibrate is: " + code2string(respVibrate1));
					disp("waiting stopping");
					// NOTE: we do not check that it stop correctly
					theFuture2.wait_for(std::chrono::milliseconds(mSWaitRequests));
				}
			}
			else {
				disp("WARNING: timeout of request");
			}
		}
		else {
			disp("WARNING: bad state");
		}
	}
	else
	{
		disp("WARNING: could not vibrate device!");
	}

	return respVibrate;
}


GF_RET_CODE MexFunction::turnOff()
{
	GF_RET_CODE response = GF_RET_CODE::GF_ERROR;
	if (gforceHandle->ds != nullptr) {
		disp("Asking to turn off");
		response = gforceHandle->ds->powerOff();
		disp("TURNING OFF... Response state is: " + code2string(response));
		disp("Please, clean the variable");
		cleanUp();
	}
	else
	{
		disp("WARNING: nullptr, not device defined");
	}
	return response;
}


void MexFunction::check_noMoreInputs(matlab::mex::ArgumentList inputs) {
	//disp("Checking unique input");
	if (inputs.size() != 1) {
		matlabError("This command does not accept other inputs! Command not executed.");
	}
}


void MexFunction::check_secondInput(matlab::mex::ArgumentList inputs) {
	//disp("Checking auxiliar input");
	if (inputs.size() != 2) {
		matlabError("This command only accepts 2 input arguments!");
	}

	if (inputs[1].getType() != matlab::data::ArrayType::DOUBLE) {
		matlabError("Second input must be numeric!");
	}
}


void MexFunction::disp(const string msj)
{
	// only ASCII!
	if (showMsjs)
	{
		matlabPtr->eval(u"fprintf('[labIA][%s] ', datestr(datetime))");
		matlabPtr->feval(u"disp",
			0, std::vector<matlab::data::Array>({ factory.createScalar(msj) }));
	}
}

void MexFunction::matlabError(const string msj) {
	matlabPtr->feval(u"error",
		0, std::vector<matlab::data::Array>({ factory.createScalar(msj) }));
}


void MexFunction::check_matlabCommand(matlab::mex::ArgumentList inputs) {
	//disp("Checking matlab command");
	if (inputs.size() < 1) {
		matlabError("Required at least 1 input argument.");
	}

	if (inputs[0].getType() != matlab::data::ArrayType::CHAR) {
		matlabError("Input must be char");
	}
	//disp("Input command seems fine!");
}


void MexFunction::check_uniqueOutput(matlab::mex::ArgumentList outputs) {

	if (outputs.size() > 1) {
		matlabError("This command only accepts 1 output");
	}

}

void MexFunction::check_noOutput(matlab::mex::ArgumentList outputs) {

	if (outputs.size() > 0)
		matlabError("This command does not return any value!");
}



void MexFunction::check_3Outputs(matlab::mex::ArgumentList outputs) {

	if (outputs.size() > 3)
		matlabError("Wrong number of outputs. This command returns 3 arguments.");
}

///\brief Converts some returned code to a string
string code2string(MexFunction::Status res)
{
	switch (static_cast<GF_UINT32>(res))
	{
	case 0:
		return "initialized";
		break;
	case 1:
		return "transmiting";
		break;

	case 3:
		return "failed";
		break;
	default:
		return "???";
		break;
	}
}

