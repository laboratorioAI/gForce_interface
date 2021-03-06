/*
This is an adaptation of the GFoce-PRO SDK code to work as C++ MEX function.
*/

#pragma once
#include "gforce.h"

#include <atomic>
#include <functional>
#include <future>
#include <chrono> 

using namespace std::chrono;

using namespace gf;
using namespace std;




// The GForceHandle implements HubListener,
//     operates gForce device and receives data
class GForceHandle : public HubListener
{
public:

	//------------- Properties
	// one dim vectors, that are reshapes to 4-by-m later on. Convention W, X,Y,Z...
	vector<float> quatData;

	bool isQuatEnabled = true;

	// one dim vectors, that are reshapes to 8-by-m later on.
	vector<uint8_t> EMGdata; // EMg signal buffer, collects the Emg signal on every data reception
	vector<uint16_t> EMGdata_long; // A replace buffer used when 12bit resolution. Only one of the two is used at any time.

	// EMG
	// resolution is enum instead of enum class for easyness
	enum class ADCResolution : size_t {
		_8bits = 8,
		_12bits = 12
	};

	// flag to denote the bit resolution. Options are 8 and 12 bits.
	ADCResolution bitResolution = ADCResolution::_8bits;
	size_t frequency = 500;

	HubState hubState = HubState::Idle;

	//------------- Gestures
	class GestureData
	{
	public:
		vector<string> gesturesPredicted; // classes
		vector<double> timeStamp; // ms count from the time reference for every prediction
		chrono::system_clock::time_point timeReference; // time from when the gestures started to be recorded.
		GestureData()
		{
			restartGestureTracking();
		}

		/// \brief adds a new gesture predicted to the buffer. It includes 
		void addGesture(string gesture)
		{
			auto tNow = chrono::system_clock::now();
			chrono::duration<double, std::milli> moment = tNow - timeReference;

			timeStamp.push_back(moment.count());
			gesturesPredicted.push_back(gesture);
		}

		/// \brief configures the vars required to track the gestures predicted by GForce-Pro armband.
		///		This consists of clearing the gesture buffers and setting a reference timepoint.
		void restartGestureTracking()
		{
			timeReference = chrono::system_clock::now();
			gesturesPredicted.clear();
			timeStamp.clear();
		}
	} gestureData;


	enum class StatusConection {
		idle,
		Scanning,
		Connecting,
		Paired,
		Connected,
		Disconnected,
		Disconnecting,
		Unknown,
		Configuring,
		Configured
	} statusConection = StatusConection::idle;

	// //////////////////////////////////
	GForceHandle(gfsPtr<Hub>& pHub)
		: mHub(pHub)
	{
	}

	/// This callback is called when the Hub finishes scanning devices.
	virtual void onScanFinished() override
	{
		if (nullptr == mDevice)
		{
			// if no device found, we do scan again
			statusConection = StatusConection::Scanning;
			mHub->startScan();
		}
		else
		{
			// or if there already is a device found and it's not
			//     in connecting or connected state, try to connect it.
			DeviceConnectionStatus status = mDevice->getConnectionStatus();
			if (DeviceConnectionStatus::Connected != status &&
				DeviceConnectionStatus::Connecting != status)
			{
				mDevice->connect();
			}
			switch (status)
			{
			case gf::DeviceConnectionStatus::Disconnected:
				statusConection = StatusConection::Disconnected;
				
				break;
			case gf::DeviceConnectionStatus::Disconnecting:
				statusConection = StatusConection::Disconnecting;
				break;
			case gf::DeviceConnectionStatus::Connecting:
				statusConection = StatusConection::Connecting;
				break;
			case gf::DeviceConnectionStatus::Connected:
				statusConection = StatusConection::Connected;
				break;
			default:
				break;
			}
		}
	}
	bool isConnectedHub = true;
	/// This callback is called when the state of the hub changed
	virtual void onStateChanged(HubState state) override
	{
		hubState = state;

		// if the hub is disconnected (such as unplugged), then set the flag of exiting the app.
		if (HubState::Disconnected == state)
		{
			//TODO: stop execution!
			isConnectedHub = false;			
		}
		switch (state)
		{
		case gf::HubState::Idle:
			statusConection = StatusConection::idle;
			break;
		case gf::HubState::Scanning:
			statusConection = StatusConection::Scanning;
			break;
		case gf::HubState::Connecting:
			statusConection = StatusConection::Connecting;
			break;
		case gf::HubState::Disconnected:
			statusConection = StatusConection::Disconnected;
			break;
		case gf::HubState::Unknown:
			statusConection = StatusConection::Unknown;
			break;
		default:
			break;
		}
	}

	/// This callback is called when the hub finds a device.
	virtual void onDeviceFound(SPDEVICE device) override
	{
		if (nullptr != device)
		{
			// only search the first connected device if we connected it before
			if (nullptr == mDevice || device == mDevice)
			{
				statusConection = StatusConection::Paired;
				mDevice = device;
				mHub->stopScan();
			}
		}
	}
	gfsPtr<DeviceSetting> ds;
	bool logLevelSetted = false;

	/// This callback is called a device has been connected successfully
	virtual void onDeviceConnected(SPDEVICE device) override
	{
		if (nullptr != device)
		{
			ds = device->getDeviceSetting();
			if (nullptr != ds)
			{ // configuring the emg!
				statusConection = StatusConection::Configuring;

				ds->getFeatureMap(std::bind(&GForceHandle::featureCallback, this, ds, std::placeholders::_1, std::placeholders::_2));

				//------------ZTJ config the log
				/* //nothing changed
				{
					promise<gf::ResponseResult> aPromise;
					auto theFuture = aPromise.get_future();

					GF_RET_CODE respPromise = ds->setLogLevel(DeviceSetting::SWOLogLevel::All,
						[&aPromise](gf::ResponseResult inResult) {
							aPromise.set_value(inResult);
						});

					if (respPromise == GF_RET_CODE::GF_SUCCESS)
					{
						gf::ResponseResult theResponse = theFuture.get();
						if (theResponse == gf::ResponseResult::RREST_SUCCESS) {
							logLevelSetted = true;
						}
					}
				}*/

				//------------ZTJ config the status
				statusConection = StatusConection::Configured;
				/*{getDeviceStatusCap
					promise<gf::ResponseResult> aPromise;
					auto theFuture = aPromise.get_future();
					DeviceSetting::DeviceStatusFlags result;

					GF_RET_CODE respPromise = ds->setDeviceStatusConfig(DeviceSetting::DeviceStatusFlags:: ,
						[&aPromise](gf::ResponseResult inResult) {
							aPromise.set_value(inResult);
						});

					if (respPromise == GF_RET_CODE::GF_SUCCESS)
					{
						gf::ResponseResult theResponse = theFuture.get();
						if (theResponse == gf::ResponseResult::RREST_SUCCESS) {
							logLevelSetted = true;
						}
					}
				}*/
			}
		}
	}

	/// This callback is called when a device has been disconnected from
	///                                 connection state or failed to connect to
	virtual void onDeviceDisconnected(SPDEVICE device, GF_UINT8 reason) override
	{
		// REASON??
		// if connection lost, we will try to reconnect again.		
		if (nullptr != device && device == mDevice)
		{
			statusConection = StatusConection::Connecting;
			mDevice->connect();
		}
	}

	/// This callback is called when the quaternion data is received
	virtual void onOrientationData(SPDEVICE device, const Quaternion& rotation) override
	{
		// store the quat data [w,X,Y,Z]
		quatData.push_back(rotation.w());
		quatData.push_back(rotation.x());
		quatData.push_back(rotation.y());
		quatData.push_back(rotation.z());
	}

	/// This callback is called when the gesture data is recevied
	virtual void onGestureData(SPDEVICE device, Gesture gest) override
	{
		// a gesture event coming.
		string gesture;
		switch (gest)
		{
		case Gesture::Relax:
			gesture = "Relax";
			break;
		case Gesture::Fist:
			gesture = "Fist";
			break;
		case Gesture::SpreadFingers:
			gesture = "SpreadFingers";
			break;
		case Gesture::WaveIn:
			gesture = "WaveIn";
			break;
		case Gesture::WaveOut:
			gesture = "WaveOut";
			break;
		case Gesture::Pinch:
			gesture = "Pinch";
			break;
		case Gesture::Shoot:
			gesture = "Shoot";
			break;
		case Gesture::Undefined:
		default:
		{
			gesture = "Undefined: ";
			string s;
			stringstream ss(s);
			ss << static_cast<int>(gest);
			gesture += ss.str();
		}
		}
		gestureData.addGesture(gesture);
	}

	string devicestatus = "";
	/// This callback is called when the button on gForce is pressed by user
	virtual void onDeviceStatusChanged(SPDEVICE device, DeviceStatus status) override
	{
		switch (status)
		{
		case DeviceStatus::ReCenter:
			devicestatus = "ReCenter";
			break;
		case DeviceStatus::UsbPlugged:
			devicestatus = "UsbPlugged";
			break;
		case DeviceStatus::UsbPulled:
			devicestatus = "UsbPulled";
			break;
		case DeviceStatus::Motionless:
			devicestatus = "Motionless";
			break;
		default:
		{
			devicestatus = "Undefined: ";
			string s;
			stringstream ss(s);
			ss << static_cast<int>(status);
			devicestatus += ss.str();
		}
		}
	}
	string dataType = "";
	/// This callback is called when extended data is received
	virtual void onExtendedDeviceData(SPDEVICE device, DeviceDataType dataTypeReceived, gfsPtr<const std::vector<GF_UINT8>> data) override
	{
		// if not data, returns
		if (data->size() == 0)
			return;

		switch (dataTypeReceived) {
		case DeviceDataType::DDT_ACCELERATE:
			dataType = "Accel";
			break;

		case DeviceDataType::DDT_GYROSCOPE:
			dataType = "gyro";
			break;

		case DeviceDataType::DDT_MAGNETOMETER:
			dataType = "magnet";
			break;

		case DeviceDataType::DDT_EULERANGLE:
			dataType = "euler";
			break;

		case DeviceDataType::DDT_QUATERNION:
			dataType = "quatExtended";
			break;

		case DeviceDataType::DDT_ROTATIONMATRIX:
			dataType = "matRot";
			break;

		case DeviceDataType::DDT_GESTURE:
			dataType = "Accel";
			break;

		case DeviceDataType::DDT_EMGRAW:

			if (bitResolution == ADCResolution::_8bits) {
				// just append when is 8 bits
				EMGdata.reserve(EMGdata.size() + distance(data->begin(), data->end()));
				EMGdata.insert(EMGdata.end(), data->begin(), data->end());
			}
			else {
				// joining the 2 bytes
				for (size_t i = 0; i < data->size(); i++) {
					if (i % 2 == 1)
						continue;
					int a = (int)data->at(i);
					int b = (int)data->at(i + 1);
					int c = (b << 8) | a;

					EMGdata_long.push_back(c);
				}
			}

			break;

		case DeviceDataType::DDT_HIDMOUSE:
			dataType = "mouse";
			break;

		case DeviceDataType::DDT_HIDJOYSTICK:
			dataType = "joistick";
			break;

		case DeviceDataType::DDT_DEVICESTATUS:
			dataType = "status";
			break;
		}
	}
	// keep a instance of hub.
	gfsPtr<Hub> mHub;
	// keep a device to operate
	gfsPtr<Device> mDevice;

private:
	void featureCallback(gfsPtr<DeviceSetting> ds, ResponseResult res, GF_UINT32 featureMap)
	{
		printf("feature map of device is 0x%08x\n", featureMap);

		featureMap >>= 6;	// Convert feature map to notification flags


		// prealloc. no quats
		DeviceSetting::DataNotifFlags flags = (DeviceSetting::DataNotifFlags)(
			DeviceSetting::DNF_OFF
			//| DeviceSetting::DNF_ACCELERATE
			//| DeviceSetting::DNF_GYROSCOPE
			//| DeviceSetting::DNF_MAGNETOMETER
			//| DeviceSetting::DNF_EULERANGLE
			//| DeviceSetting::DNF_QUATERNION
			//| DeviceSetting::DNF_ROTATIONMATRIX
			//| DeviceSetting::DNF_EMG_GESTURE
			| DeviceSetting::DNF_EMG_RAW
			//| DeviceSetting::DNF_HID_MOUSE
			//| DeviceSetting::DNF_HID_JOYSTICK
			//| DeviceSetting::DNF_DEVICE_STATUS
			);

		if (isQuatEnabled)
			flags = (DeviceSetting::DataNotifFlags)(
				DeviceSetting::DNF_OFF
				//| DeviceSetting::DNF_ACCELERATE
				//| DeviceSetting::DNF_GYROSCOPE
				//| DeviceSetting::DNF_MAGNETOMETER
				//| DeviceSetting::DNF_EULERANGLE
				| DeviceSetting::DNF_QUATERNION
				//| DeviceSetting::DNF_ROTATIONMATRIX
				//| DeviceSetting::DNF_EMG_GESTURE
				| DeviceSetting::DNF_EMG_RAW
				//| DeviceSetting::DNF_HID_MOUSE
				//| DeviceSetting::DNF_HID_JOYSTICK
				//| DeviceSetting::DNF_DEVICE_STATUS
				);

		printf("desired notification flags is 0x%08x, valid is 0x%08x\n", (GF_UINT32)flags, (GF_UINT32)(flags & featureMap));

		ds->setDataNotifSwitch((DeviceSetting::DataNotifFlags)(flags & featureMap),
			[](ResponseResult res) {
				printf("result of setDataNotifSwitch is %u\n", (GF_UINT32)res);
			});

		ds->setEMGRawDataConfig(1000,						//sample rate
			(DeviceSetting::EMGRowDataChannels)(0x00FF),	//channel 0~7
			128,											//data length
			8,												// adc resolution
			[](ResponseResult result) {
				string ret = (result == ResponseResult::RREST_SUCCESS) ? ("sucess") : ("failed");
				printf("result of setEMGRawDataConfig is %s\n", ret.c_str());
			});
	}
};
