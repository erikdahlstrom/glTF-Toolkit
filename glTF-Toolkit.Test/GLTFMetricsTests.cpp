// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "pch.h"
#include <CppUnitTest.h>

#include "GLTFSDK/IStreamWriter.h"
#include "GLTFSDK/GLTFConstants.h"
#include "GLTFSDK/Serialize.h"
#include "GLTFSDK/Deserialize.h"
#include "GLTFSDK/GLBResourceReader.h"
#include "GLTFSDK/GLTFResourceWriter.h"
#include "GLTFSDK/RapidJsonUtils.h"

#include "GLTFMetrics.h"

#include "Helpers/WStringUtils.h"
#include "Helpers/TestUtils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Microsoft::glTF;
using namespace Microsoft::glTF::Toolkit;

namespace Microsoft::glTF::Toolkit::Test
{
	TEST_CLASS(GLTFMetricsTests)
	{
		const char* c_cubeWithLODJson = "Resources\\gltf\\CubeWithLOD.gltf";
		const char* c_cubeWithoutLODJson = "Resources\\gltf\\CubeAsset3D.gltf";

		TEST_METHOD(GLTFMetrics_GetNumberOfNodeLODLevels)
		{
			TestUtils::LoadAndExecuteGLTFTest(c_cubeWithLODJson, [](auto doc, auto path)
			{
				std::wstring message(path.begin(), path.end());
				Assert::IsTrue(1 == GLTFMetrics::GetNumberOfNodeLODLevels(doc), message.c_str());
			});
		}

		TEST_METHOD(GLTFMetrics_GetNumberOfNodeLODLevels_NoLod)
		{
			TestUtils::LoadAndExecuteGLTFTest(c_cubeWithoutLODJson, [](auto doc, auto path)
			{
				std::wstring message(path.begin(), path.end());
				Assert::IsTrue(0 == GLTFMetrics::GetNumberOfNodeLODLevels(doc), message.c_str());
			});
		}

	};
}

