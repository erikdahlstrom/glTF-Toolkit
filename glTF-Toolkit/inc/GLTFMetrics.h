// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include <GLTFSDK/GLTFDocument.h>
#include <GLTFSDK/IStreamReader.h>
#include <map>

namespace Microsoft::glTF::Toolkit
{
	/// <summary>
	/// Utilities to query glTF assets' metrics, such as the max animation duration, triangles per LOD level etc.
	/// </summary>
	class GLTFMetrics
	{
	public:
		static std::vector<size_t> GetNumberOfTrianglesPerLOD(const GLTFDocument& doc);
		static std::vector<uint32_t> GetNumberOfNodesPerLOD(const GLTFDocument& doc);
		static std::vector<uint32_t> GetMaxNumberOfSubmeshesPerLOD(const GLTFDocument& doc); // GetMaxNumberOfMeshPrimitivesPerLOD?
		static float GetMaxAnimationDurationInSeconds(const GLTFDocument& doc);
		static std::vector<std::pair<size_t, size_t>> GetAllTextureDimensions(const IStreamReader& streamReader, const GLTFDocument& doc);
		static std::pair<size_t, size_t> GetMaxTextureSize(const IStreamReader& streamReader, const GLTFDocument& doc);
		//static std::map<AnimationChannel, unsigned int> GetMaxKeyFramesPerChannel(const GLTFDocument& doc);
		static uint32_t GetNumberOfNodeLODLevels(const GLTFDocument& doc);
	};
}
