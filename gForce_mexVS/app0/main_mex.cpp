/*
 * Copyright 2017, OYMotion Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 *
 * The most important functions are here. The rest is in mexMethods
 */
#include "stdafx.h"
#include "mexMethods.h"
#include "mexForce.h"
#include "mex.hpp"
#include "mexAdapter.hpp"

#include <ctime>


 // /////////////////////////////////////////
/// \brief Runs all the process to connect the device.
GF_RET_CODE MexFunction::startTransmission() {
	// constructor to initialize object.
	disp("Connecting");
	GF_RET_CODE retCode = GF_RET_CODE::GF_ERROR; // prealloc

	// get the hub instance from hub factory
	tstring identifer(_T("Laboratorio I.A. EPN app"));

	pHub = HubManager::getHubInstance(identifer);
	Sleep(scanTimeOut);

	if (pHub == nullptr)
	{
		retCode = GF_RET_CODE::GF_ERROR; // something went wrong
		matlabError("Could not get a Hub instance");
		return retCode;
	}
	// set work mode to WorkMode::Polling, then the client has
	//     to call Hub::run to receive callbacks
	try {
		pHub->setWorkMode(WorkMode::Polling);
	}
	catch (...) {
		retCode = GF_RET_CODE::GF_ERROR; // something went wrong
		matlabError("Polling could not be configured");
		return retCode;
	}

	// create the listener implementation and register to hub
	gforceHandle = make_shared<GForceHandle>(pHub);
	listener = static_pointer_cast<HubListener>(gforceHandle);
	disp(code2string(gforceHandle->statusConection));
	//
	disp("Registering Listener...");
	try
	{
		if (listener != nullptr && pHub != nullptr)
			retCode = pHub->registerListener(listener);
		else
			retCode = GF_RET_CODE::GF_ERROR; // something went wrong
	}
	catch (...)
	{
		retCode = GF_RET_CODE::GF_ERROR; // something went wrong
	}
	disp(code2string(gforceHandle->statusConection));
	if (retCode == GF_RET_CODE::GF_SUCCESS)
		disp("Listener registered");
	else
	{
		disp("Return code of reginstering listerner:" + code2string(retCode));
		matlabError("Something went wrong with the listener! Ret code: " + code2string(retCode));
		return retCode;
	}
	disp(code2string(gforceHandle->statusConection));

	// Initialize hub. Could be failed in below cases:
	//   1. The hub is not plugged in the USB port.
	//   2. Other apps are connected to the hub already.
	disp("Registering Hub...");
	try {
		retCode = pHub->init();
		Sleep(scanTimeOut);
	}
	catch (...) {
		retCode = GF_RET_CODE::GF_ERROR; // something went wrong
	}

	if (retCode == GF_RET_CODE::GF_SUCCESS)
		disp("Hub registered");
	else
	{
		disp("Return code of hub init:" + code2string(retCode));
		matlabError("Hub not registered, maybe it is unplugged or other app is already connected. Try restarting the device. Ret code: " + code2string(retCode));
		return retCode;
	}
	disp(code2string(gforceHandle->statusConection));
	// start to scan devices
	disp("Scanning devices...");
	try {
		retCode = pHub->startScan();
		Sleep(scanTimeOut);
	}
	catch (...) {
		retCode = GF_RET_CODE::GF_ERROR; // something went wrong		
	}
	
	disp(code2string(gforceHandle->statusConection));
	disp("Scanned");
	if (retCode == GF_RET_CODE::GF_SUCCESS)
		disp("Device found");
	else
	{
		disp("Return code of scan:" + code2string(retCode));
		matlabError("Not device found, code is: " + code2string(retCode));
		return retCode;
	}

	disp(code2string(gforceHandle->statusConection));
	
	// Hub::run could be failed in the below cases:
	//   1. other threads have already been launching it.
	//   2. WorkMode is not set to WorkMode::Polling.

	// A return of GF_RET_CODE::GF_ERROR_TIMEOUT means no error but period expired.
	disp("run!");
	if (pHub != nullptr)
	{
		retCode = pHub->run(transmitionDelay);
		disp("Configuring");

		auto tBase = chrono::system_clock::now();
		
		chrono::duration<double, std::milli> moment;
		do 
		{
			auto tNow = chrono::system_clock::now();
			moment = tNow - tBase;
			Sleep(10);/*
			if (gforceHandle->statusConection != GForceHandle::StatusConection::idle)
				disp(code2string(gforceHandle->statusConection));*/
			
		} while (gforceHandle->statusConection != GForceHandle::StatusConection::Configured && 
			moment.count() < configurationMs);		
	}

	else
		retCode = GF_RET_CODE::GF_ERROR; // something went wrong		

	disp(code2string(gforceHandle->statusConection));
	disp("Return code of running was " + code2string(retCode));
	if ((GF_RET_CODE::GF_SUCCESS == retCode || GF_RET_CODE::GF_ERROR_TIMEOUT == retCode) 
		&& gforceHandle->statusConection == GForceHandle::StatusConection::Configured)
	{
		// everything fine
		//changing working mode!
		if (gforceHandle->EMGdata.size() == 0 && gforceHandle->EMGdata_long.size() == 0)
		{//  not received anything!
			disp("WARNING: not received anything!");
			retCode = GF_RET_CODE::GF_ERROR;
		}
		gforceHandle->EMGdata.clear();
		gforceHandle->EMGdata_long.clear();
		gforceHandle->quatData.clear();

	}
	else {
		matlabError("Something went wrong in running! Ret code:" + code2string(retCode));
	}
	disp(code2string(gforceHandle->statusConection));

	disp("Changing to freerun");
	try {
		pHub->setWorkMode(WorkMode::Freerun);
		disp("GForce ready");
	}
	catch (...) {
		retCode = GF_RET_CODE::GF_ERROR; // something went wrong
		matlabError("Could not change to freerun");
		return retCode;
	}
	disp(code2string(gforceHandle->statusConection));
	return retCode; // success or timeout		
}


// /////////////////////////////////////////
void MexFunction::operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
	//------------ Inputs
	check_matlabCommand(inputs);

	// -- validating device connectivity
	if (status != Status::TRANSMITING || gforceHandle->hubState == HubState::Disconnected)
	{
		//cleanUp();
		matlabError("FATAL ERROR! The device is not connected nor transmiting any data. Clean, reconnect, turn on, turn off and/or try again!");
		return;
	}

	if (gforceHandle->devicestatus != "")
		matlabError("!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Device status: " + gforceHandle->devicestatus);
	if (gforceHandle->dataType != "")
		matlabError("!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Data type: " + gforceHandle->dataType);

	// ---State Machine!!! Getting matlab command
	matlab::data::CharArray input0(inputs[0]);
	string matlabCommand = input0.toAscii();
	disp("Command: " + matlabCommand);

	// ----AUX---------------/////////////////////////////////////////
	if (matlabCommand == "getStatus") {
		check_noMoreInputs(inputs);
		check_uniqueOutput(outputs);

		bool out = gforceHandle->statusConection == GForceHandle::StatusConection::Configured; // prealloc

		disp("S: " + code2string(gforceHandle->statusConection));

		if (nullptr != gforceHandle->mDevice) {
			gf::DeviceConnectionStatus stat = gforceHandle->mDevice->getConnectionStatus();
			disp("Requested device estatus: " + code2string(gforceHandle->mDevice->getConnectionStatus()));
			out = out && stat == gf::DeviceConnectionStatus::Connected;
		}
		else
			disp("WARNING: device not defined!");

		if (nullptr != gforceHandle->ds)
			disp("Device setting found");
		else
		{
			disp("WARNING: device setting not defined!");
			out = false;
		}


		disp("Stored device status: " + gforceHandle->devicestatus + " _");
		disp("Stored data type: " + gforceHandle->dataType + " _");

		disp("Stored hub state: " + code2string(gforceHandle->hubState));
		if (nullptr != gforceHandle->mHub)
			disp("Requested Hub state: " + code2string(gforceHandle->mHub->getState()));
		else
			disp("WARNING: Hub state not defined!");

		if (gforceHandle->isConnectedHub)
			disp("hub is connected (stored)");
		else
			disp("hub is not connected (stored)");

		disp("Mex status: " + code2string(status));



		disp("Device status configured: " + requested_deviceStatus());

		if (gforceHandle->logLevelSetted)
			disp("Log level set");
		else
			disp("Log level could not be set");

		// salida
		matlab::data::TypedArray<bool> A = factory.createScalar<bool>(out);
		outputs[0] = std::move(A);
	}
	// /////////////////////////////////////////
	else if (matlabCommand == "getBattery")
	{
		check_noMoreInputs(inputs);
		check_uniqueOutput(outputs);

		int battery = requested_Battery();
		matlab::data::TypedArray<int8_t> A = factory.createScalar<int8_t>(battery);
		outputs[0] = std::move(A);
		return;
	}
	// /////////////////////////////////////////
	/* //Temperature  %       gForce_mex('getTemperature') returns (int8) the temperature. Im
					%       case something went wrong returns -273.
	else if (matlabCommand == "getTemperature")
	{
		check_noMoreInputs(inputs);
		check_uniqueOutput(outputs);

		int temperature = requested_Temperature();
		matlab::data::TypedArray<int8_t> A = factory.createScalar<int8_t>(temperature);
		outputs[0] = std::move(A);
		return;
	}*/
	// /////////////////////////////////////////
	else if (matlabCommand == "vibrate")
	{
		check_secondInput(inputs);
		check_noOutput(outputs);

		// get time
		INT time = (INT)inputs[1][0];
		if (time > 0)
			vibrate(time);

		else
			matlabError("Vibration time must be greater than 0");

		return;
	}
	// /////////////////////////////////////////
	else if (matlabCommand == "powerOff")
	{
		check_noMoreInputs(inputs);
		check_noOutput(outputs);

		turnOff();
		return;
	}
	// /////////////////////////////////////////
	else if (matlabCommand == "verbose") {
		check_secondInput(inputs);
		int a = (int)inputs[1][0];
		if (a == 0)
		{
			showMsjs = false;
			disp("No more annoying messages");
		}
		else {
			showMsjs = true;
			disp("Enabled information messages");
		}

		return;
	}
	// ------------EMG-------------/////////////////////////////////////////
	else if (matlabCommand == "getEmg") {
		check_noMoreInputs(inputs);
		check_uniqueOutput(outputs);

		//---------------- SALIDA
		disp("Configuring output!");

		size_t tamEMG;
		if (gforceHandle->bitResolution == GForceHandle::ADCResolution::_8bits)
			tamEMG = gforceHandle->EMGdata.size();
		else
			tamEMG = gforceHandle->EMGdata_long.size();

		if (tamEMG % 8 != 0 || tamEMG == 0)
		{
			const string txt = to_string(tamEMG);
			disp("WARNING: Emg size is a little bit strange: " + txt);
			matlab::data::Array A = factory.createEmptyArray();
			outputs[0] = std::move(A);

			// both just in case
			gforceHandle->EMGdata.clear();
			gforceHandle->EMGdata_long.clear();
			return;
		}

		//else it is ok!
		disp("Emg construction!");
		if (gforceHandle->bitResolution == GForceHandle::ADCResolution::_8bits)
		{
			matlab::data::ArrayDimensions dims{ 8, gforceHandle->EMGdata.size() / 8 };

			vector<uint8_t>::iterator inicio = gforceHandle->EMGdata.begin();
			vector<uint8_t>::iterator fin = gforceHandle->EMGdata.end();
			matlab::data::TypedArray<uint8_t> A = factory.createArray<vector<uint8_t>::iterator, uint8_t>(dims, inicio, fin);

			outputs[0] = std::move(A);

			// both just in case
			gforceHandle->EMGdata.clear();
			gforceHandle->EMGdata_long.clear();
		}
		else
		{
			matlab::data::ArrayDimensions dims{ 8, gforceHandle->EMGdata_long.size() / 8 };

			vector<uint16_t>::iterator inicio = gforceHandle->EMGdata_long.begin();
			vector<uint16_t>::iterator fin = gforceHandle->EMGdata_long.end();
			matlab::data::TypedArray<uint16_t> A = factory.createArray<vector<uint16_t>::iterator, uint16_t>(dims, inicio, fin);

			outputs[0] = std::move(A);

			// both just in case
			gforceHandle->EMGdata.clear();
			gforceHandle->EMGdata_long.clear();
		}
		disp("Emg returned successfully!");
		return;
	}
	// /////////////////////////////////////////
	else if (matlabCommand == "clearEmg") {
		check_noMoreInputs(inputs);
		check_noOutput(outputs);

		gforceHandle->EMGdata.clear();
		gforceHandle->EMGdata_long.clear();
		disp("Emg buffer cleared!");
		return;
	}
	// /////////////////////////////////////////
	else if (matlabCommand == "setEmgResolution") {
		check_secondInput(inputs);
		check_uniqueOutput(outputs);

		gforceHandle->EMGdata.clear();
		gforceHandle->EMGdata_long.clear();
		// verifying resolution
		size_t resolution = (size_t)inputs[1][0];
		switch (resolution)
		{
		case 8:
			gforceHandle->bitResolution = GForceHandle::ADCResolution::_8bits;
			break;
		case 12:
			gforceHandle->bitResolution = GForceHandle::ADCResolution::_12bits;
			if (gforceHandle->frequency > 500)
			{
				disp("WARNING: 12 bit resolution does not allow sampling rates higher than 500Hz. Automatically adjusting it to 500Hz.");
				gforceHandle->frequency = 500;
			}
			break;
		default:
			matlabError("Incorrect ADC resolution! Available resolutions are 8 and 12 bits.");

			// just in case
			matlab::data::TypedArray<bool> A = factory.createScalar<bool>(false);
			outputs[0] = std::move(A);
			return;
			break;
		}

		bool resp = requested_configEmg();

		if (resp)
		{// updating only when the change was correctly done.
			if (resolution == 8)
				gforceHandle->bitResolution = GForceHandle::ADCResolution::_8bits;
			else
				gforceHandle->bitResolution = GForceHandle::ADCResolution::_12bits;
		}

		// both just in case
		gforceHandle->EMGdata.clear();
		gforceHandle->EMGdata_long.clear();

		matlab::data::TypedArray<bool> A = factory.createScalar<bool>(resp);
		outputs[0] = std::move(A);
		return;
	}
	// /////////////////////////////////////////
	else if (matlabCommand == "setEmgSamplingRate")
	{
		check_secondInput(inputs);
		check_uniqueOutput(outputs);

		// both just in case
		gforceHandle->EMGdata.clear();
		gforceHandle->EMGdata_long.clear();

		// verifying frequency
		int freq = (int)inputs[1][0];
		if (freq < 0 || freq > 1000)
		{
			matlabError("Sampling rate not supported.");
			// just in case
			matlab::data::TypedArray<bool> A = factory.createScalar<bool>(false);
			outputs[0] = std::move(A);
			return;
		}
		else if (freq > 500 && GForceHandle::ADCResolution::_12bits == gforceHandle->bitResolution)
		{
			disp("WARNING: 12 bit resolution does not allow sampling rates higher than 500Hz. Automatically adjusting it to 500Hz.");
			gforceHandle->frequency = 500;
		}
		else
			gforceHandle->frequency = freq;

		bool resp = requested_configEmg();

		// both just in case
		gforceHandle->EMGdata.clear();
		gforceHandle->EMGdata_long.clear();

		matlab::data::TypedArray<bool> A = factory.createScalar<bool>(resp);
		outputs[0] = std::move(A);
		return;
	}
	// --------ORIENTATION------------/////////////////////////////////////////
	else if (matlabCommand == "getQuaternions")
	{
		check_noMoreInputs(inputs);
		check_uniqueOutput(outputs);

		//---------------- SALIDA
		disp("Configuring quat output!");

		size_t QuatTam = gforceHandle->quatData.size();

		if (QuatTam == 0 || QuatTam % 4 != 0)
		{
			const string txt = to_string(QuatTam);
			disp("WARNING: Quaternions buffer size is a little bit strange: " + txt);
			disp("WARNING: Try enabling quaternions");
			gforceHandle->quatData.clear();
			matlab::data::Array A = factory.createEmptyArray();
			outputs[0] = std::move(A);
			return;
		}

		//else it is ok!
		disp("Quat construction!");
		matlab::data::ArrayDimensions dims{ 4, gforceHandle->quatData.size() / 4 };

		vector<float>::iterator inicio = gforceHandle->quatData.begin();
		vector<float>::iterator fin = gforceHandle->quatData.end();
		matlab::data::TypedArray<float> A = factory.createArray<vector<float>::iterator, float>(dims, inicio, fin);

		outputs[0] = std::move(A);

		gforceHandle->quatData.clear();

		disp("Quaternions buffer returned successfully!");
		return;
	}
	// /////////////////////////////////////////
	else if (matlabCommand == "clearQuaternions")
	{
		check_noMoreInputs(inputs);
		check_noOutput(outputs);

		gforceHandle->quatData.clear();

		disp("Quaternions buffer cleared!");
		return;
	}
	// /////////////////////////////////////////
	else if (matlabCommand == "enableQuaternions")
	{
		check_secondInput(inputs);
		check_uniqueOutput(outputs);

		gforceHandle->quatData.clear();

		bool toEnable = (bool)inputs[1][0];
		bool resp = requested_disableQuats(toEnable);

		matlab::data::TypedArray<bool> A = factory.createScalar<bool>(resp);
		outputs[0] = std::move(A);
		gforceHandle->quatData.clear();
		if (resp)
			disp("Quaternions enabled!");
		else
			disp("Quaternions disabled!");
		return;
	}
	// --------GESTURE------------/////////////////////////////////////////
	else if (matlabCommand == "getPredictions")
	{
		check_noMoreInputs(inputs);
		check_3Outputs(outputs);

		disp("Prediction construction!");
		// ---- classes
		matlab::data::ArrayDimensions dims{ 1, gforceHandle->gestureData.gesturesPredicted.size() };

		vector<string>::iterator inicio = gforceHandle->gestureData.gesturesPredicted.begin();
		vector<string>::iterator fin = gforceHandle->gestureData.gesturesPredicted.end();
		matlab::data::TypedArray<matlab::data::MATLABString> A =
			factory.createArray<vector<string>::iterator, matlab::data::MATLABString>(dims, inicio, fin);

		outputs[0] = std::move(A); // always 1 output

			//--- timestamp
		matlab::data::ArrayDimensions dims2{ 1, gforceHandle->gestureData.timeStamp.size() };

		vector<double>::iterator inicio2 = gforceHandle->gestureData.timeStamp.begin();
		vector<double>::iterator fin2 = gforceHandle->gestureData.timeStamp.end();
		matlab::data::TypedArray<double> A2 = factory.createArray<vector<double>::iterator, double>(dims, inicio2, fin2);

		outputs[1] = std::move(A2);

		//----------------- time reference
		auto timeRef = gforceHandle->gestureData.timeReference;

		std::time_t t = system_clock::to_time_t(timeRef);

		/* //ooption string
		char str[26];
		ctime_s(str, sizeof str, &t);
		matlab::data::TypedArray<matlab::data::MATLABString> A3 = factory.createScalar(str);
		outputs[2] = std::move(A3);
		*/

		// option struct
		struct tm buf;
		localtime_s(&buf, &t); // converts time in struct time

		matlab::data::StructArray S = factory.createStructArray({ 1 },
			{ "sec", "min", "hour", "day", "mon", "year" });

		S[0]["sec"] = factory.createScalar<uint8_t>(buf.tm_sec);
		S[0]["min"] = factory.createScalar<uint8_t>(buf.tm_min);
		S[0]["hour"] = factory.createScalar<uint8_t>(buf.tm_hour);
		S[0]["day"] = factory.createScalar<uint8_t>(buf.tm_mday);
		S[0]["mon"] = factory.createScalar<uint8_t>(buf.tm_mon + 1);
		S[0]["year"] = factory.createScalar<uint16_t>(buf.tm_year + 1900);

		outputs[2] = std::move(S);

		gforceHandle->gestureData.restartGestureTracking();
		disp("Predictions returned successfully!");
		return;
	}
	// /////////////////////////////////////////
	else if (matlabCommand == "clearPredictions")
	{
		check_noMoreInputs(inputs);
		check_noOutput(outputs);

		gforceHandle->gestureData.restartGestureTracking();

		disp("Prediction buffer cleared!");
		return;
	}
	// /////////////////////////////////////////
	else
	{
		matlabError("Unkown command: " + matlabCommand);
		return;
	}
}



void MexFunction::cleanUp()
{
	disp("Cleaning everything.");
	// Delete and restart!
	if (nullptr == pHub)
		goto exit;

	/*
	* disp("The famous last words:");
	wstring secretWs = pHub->getDescString();
	std::string secret((const char*)&secretWs[0], sizeof(wchar_t) / sizeof(char) * secretWs.size());
	disp(secret);
	// Welcome to the gForce Wolrd*/

	// Clean execution envionment while exiting

	/* // sotpping scan!
	try
	{
		disp("Stopping scan");
		gf::GF_RET_CODE resp = pHub->stopScan();
		Sleep(scanTimeOut);
		disp("result of stopping scan:" + code2string(resp));
	}
	catch (...)
	{
		disp("WARNING: could not stop scanning devices.");
		goto exit;
	}*/

	if (listener != nullptr && nullptr != pHub)
	{
		disp("Unregistering");
		gf::GF_RET_CODE resp = pHub->unRegisterListener(listener);
		Sleep(scanTimeOut);
		disp("result of unregistering:" + code2string(resp));
	}
	else
		disp("WARNING: could not unregister listerner.");


	/* // descnnneting
	try
	{
		disp("Desconecting device");
		gf::GF_RET_CODE resp = gforceHandle->mDevice->disconnect();
		Sleep(scanTimeOut);
		disp("result of desconeting device: " + code2string(resp));
	}
	catch (...)
	{
		disp("WARNING: could not desconenct device.");
	}*/

	if (nullptr != pHub)
	{
		disp("Deiniting hub");
		gf::GF_RET_CODE resp = pHub->deinit();
		disp("result of deinit :" + code2string(resp));
	}
	else
		disp("WARNING: could not deinit phub.");


exit:
	pHub.reset();
	listener.reset();
}

