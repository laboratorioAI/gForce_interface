/*
* Ejemplo para comprobar el funcionamiento de las funciones MEX
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
 */
#include "stdafx.h"
#include <vector>


#include "mex.hpp"
#include "mexAdapter.hpp"

using namespace std;

class MexFunction : public matlab::mex::Function {

	// matlab own
	std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr;
	matlab::data::ArrayFactory factory;

	// bandera para mostrar msj
	bool showMsjs = true;

public:
	MexFunction() {
		// En el constructor se inicializa el puntero a Matlab
		matlabPtr = getEngine();
	}


	// /////////////////////////////////////////
	void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {

		// retrieve data
		checkArguments(outputs, inputs);		
		
		//---------------- SALIDA
		vector<int16_t> a = { 2,3,4,5,7,7 };
		vector<int16_t>::iterator inicio = a.begin();
		vector<int16_t>::iterator fin = a.end();

		matlab::data::ArrayDimensions dims{ a.size() / 2, a.size() / 3 };
		matlab::data::TypedArray<int16_t> A = factory.createArray<vector<int16_t>::iterator, int16_t>(dims, inicio, fin);
		
		outputs[0] = std::move(A);
	}

	// /////////////////////////////////////////
	void disp(const string msj)
	{ 
		// only ASCII!
		if (showMsjs)
			matlabPtr->feval(u"disp",
				0, std::vector<matlab::data::Array>({ factory.createScalar(msj) }));
	}

	// /////////////////////////////////////////
	void matlabError(const string msj) {
		matlabPtr->feval(u"error",
			0, std::vector<matlab::data::Array>({ factory.createScalar(msj) }));
	}

	// /////////////////////////////////////////
	void checkArguments(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
		// runs an error in Matlab to stop the execution!


		disp("Checking inputs");
		if (inputs.size() != 1) {
			matlabError("Only 1 input accepted");
		}

		if (outputs.size() > 1) {
			matlabError("Too many outputs! Only 1 output is possible");
		}

		if (inputs[0].getType() != matlab::data::ArrayType::CHAR) {
			matlabError("Input must be char");
		}

		disp("Inputs fine!");
	}
};
