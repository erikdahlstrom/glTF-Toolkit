// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "pch.h"
#include "GLTFMetrics.h"

#include "GLTFLODUtils.h"

#include <DirectXTex.h>
#include <GLTFSDK/GLTFResourceReader.h>

using namespace Microsoft::glTF;
using namespace Microsoft::glTF::Toolkit;

namespace
{
	size_t GetTriangleCountForMesh(const GLTFDocument& doc, const std::string& meshId)
	{
		size_t numtris = 0;
		auto mesh = doc.meshes.Get(meshId);
		for (auto primitive : mesh.primitives)
		{
			if (primitive.mode == MeshMode::MESH_TRIANGLES)
			{
				auto accessor = doc.accessors.Get(primitive.indicesAccessorId);
				numtris += accessor.count / 3;
			}
			else
			{
				throw std::invalid_argument("Unhandled primitive.mode: " + primitive.mode);
			}
		}

		return numtris;
	}

	size_t GetTriangleCountForNode(const GLTFDocument& doc, const std::string& nodeId)
	{
		size_t tricount = 0;

		auto node = doc.nodes.Get(nodeId);

		if (!node.meshId.empty())
		{
			tricount += GetTriangleCountForMesh(doc, node.meshId);
		}

		for (auto childId : node.children)
		{
			tricount += GetTriangleCountForNode(doc, childId);
		}

		return tricount;
	}

	size_t GetMaxPrimitiveCountForNode(const GLTFDocument& doc, const std::string& nodeId)
	{
		size_t maxPrimitiveCount = 0;

		auto node = doc.nodes.Get(nodeId);

		if (!node.meshId.empty())
		{
			auto mesh = doc.meshes.Get(node.meshId);
			maxPrimitiveCount = std::max(maxPrimitiveCount, mesh.primitives.size());
		}

		for (auto childId : node.children)
		{
			maxPrimitiveCount = std::max(maxPrimitiveCount, GetMaxPrimitiveCountForNode(doc, childId));
		}

		return maxPrimitiveCount;
	}

	int GetNodeCount(const GLTFDocument& doc, const Node& node)
	{
		int count = 1;

		for (auto childId : node.children)
		{
			count += GetNodeCount(doc, doc.nodes.Get(childId));
		}

		return count;
	}

	int GetSceneNodeCount(const GLTFDocument& doc, const Scene& scene)
	{
		int count = 0;
		for (auto nodeId : scene.nodes)
		{
			count += GetNodeCount(doc, doc.nodes.Get(nodeId));
		}
		return count;
	}
}

std::vector<size_t> GLTFMetrics::GetNumberOfTrianglesPerLOD(const GLTFDocument& doc)
{
	std::vector<size_t> tricountsPerLOD;
	auto lodMap = GLTFLODUtils::ParseDocumentNodeLODs(doc);
	for (auto lod : lodMap)
	{
		if (lod.second == nullptr || lod.second->size() == 0)
		{
			continue;
		}

		tricountsPerLOD.emplace_back(GetTriangleCountForNode(doc, lod.first));

		for (int i = 0; i < lod.second->size(); i++)
		{
			tricountsPerLOD.emplace_back(GetTriangleCountForNode(doc, lod.second->at(i)));
		}
	}

	return tricountsPerLOD;
}

std::vector<unsigned int> GLTFMetrics::GetNumberOfNodesPerLOD(const GLTFDocument& doc)
{
	std::vector<unsigned int> nodecountsPerLOD;
	nodecountsPerLOD.emplace_back(GetSceneNodeCount(doc, doc.GetDefaultScene()));
	// FIXME: traverse LOD1..n too
	return nodecountsPerLOD;
}

std::vector<unsigned int> GLTFMetrics::GetMaxNumberOfSubmeshesPerLOD(const GLTFDocument& doc)
{
	std::vector<unsigned int> maxNumberOfSubmeshesPerLOD;
	maxNumberOfSubmeshesPerLOD.emplace_back(static_cast<unsigned int>(GetMaxPrimitiveCountForNode(doc, doc.GetDefaultScene().nodes[0])));
	// FIXME: traverse LOD1..n too
	return maxNumberOfSubmeshesPerLOD;
}

float GLTFMetrics::GetMaxAnimationDurationInSeconds(const GLTFDocument& doc)
{
	float maxAnimationDurationInSeconds = 0;
	for (auto anim : doc.animations.Elements())
	{
		for (auto channel : anim.channels)
		{
			auto sampler = anim.samplers.Get(channel.samplerId);
			auto inputAccessor = doc.accessors.Get(sampler.inputAccessorId);
			maxAnimationDurationInSeconds  = std::max(inputAccessor.max[0], maxAnimationDurationInSeconds);
		}
	}

	return maxAnimationDurationInSeconds;
}

std::vector<std::pair<size_t, size_t>> GLTFMetrics::GetAllTextureDimensions(const IStreamReader& streamReader, const GLTFDocument& doc)
{
	std::vector<std::pair<size_t, size_t>> allImageDimensions;
	GLTFResourceReader gltfResourceReader(streamReader);

	for (auto texture : doc.textures.Elements())
	{
		const Image& image = doc.images.Get(texture.imageId);

		std::vector<uint8_t> imageData = gltfResourceReader.ReadBinaryData(doc, image);

		DirectX::TexMetadata info;
		if (FAILED(DirectX::GetMetadataFromDDSMemory(imageData.data(), imageData.size(), DirectX::DDS_FLAGS_NONE, info)))
		{
			// DDS failed, try WIC
			// Note: try DDS first since WIC can load some DDS (but not all)
			if (FAILED(DirectX::GetMetadataFromWICMemory(imageData.data(), imageData.size(), DirectX::WIC_FLAGS_IGNORE_SRGB, info)))
			{
				throw GLTFException("Failed to load image - Image could not be loaded as DDS or read by WIC.");
			}
		}

		allImageDimensions.emplace_back(std::make_pair(info.width, info.height));
	}

	return allImageDimensions;
}

std::pair<size_t, size_t> GLTFMetrics::GetMaxTextureSize(const IStreamReader& streamReader, const GLTFDocument& doc)
{
	auto dims = GetAllTextureDimensions(streamReader, doc);

	std::pair<size_t, size_t> maxTextureSize(0, 0);

	for (const auto& dim : dims)
	{
		if (dim.first * dim.second > maxTextureSize.first * maxTextureSize.second)
		{
			maxTextureSize = dim;
		}
	}

	return maxTextureSize;
}

// std::map<AnimationChannel, unsigned int> GLTFMetrics::GetMaxKeyFramesPerChannel(const GLTFDocument& doc)
// {
// 	std::map<AnimationChannel, unsigned int> maxKeyFramesPerChannel;
// 	return maxKeyFramesPerChannel;
// }

unsigned int GLTFMetrics::GetNumberOfNodeLODLevels(const GLTFDocument& doc)
{
	auto lodMap = GLTFLODUtils::ParseDocumentNodeLODs(doc);
	return GLTFLODUtils::NumberOfNodeLODLevels(doc, lodMap);
}
