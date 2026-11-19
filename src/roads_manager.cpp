#include "roads_manager.h"
#include "packet_handler.h"
#include "main.h"
#include "utils.h"

#include <CHud.h>
#include <CMessages.h>
#include <CPed.h>
#include <CVehicle.h>
#include <CWorld.h>
#include <CColPoint.h>
#include <CPools.h>
#include <CObject.h>
#include <CPathFind.h>
#include <CPathNode.h>
#include <CCarPathLink.h>
#include <CAutoPilot.h>
#include <common.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

static constexpr float BT_PI = 3.14159265358979323846f;
static constexpr float BT_RAD_TO_DEG = 180.0f / BT_PI;
static constexpr float BT_DEG_TO_RAD = BT_PI / 180.0f;

static float NormalizeAngle(float angle)
{
	while (angle < 0.0f) angle += 360.0f;
	while (angle >= 360.0f) angle -= 360.0f;
	return angle;
}

static float NormalizeAngleDiff(float diff)
{
	while (diff < -180.0f) diff += 360.0f;
	while (diff > 180.0f) diff -= 360.0f;
	return diff;
}

#pragma pack(push, 1)
struct ImgHeader {
	char tag[4];
	uint32_t entryCount;
};

struct ImgDirEntry {
	uint32_t offset;
	uint16_t streamingSize;
	uint16_t sizeInSectors;
	char name[24];
};

struct RawNodeHeader {
	uint32_t numNodes;
	uint32_t numVehicleNodes;
	uint32_t numPedNodes;
	uint32_t numNaviNodes;
	uint32_t numLinks;
};

struct RawPathNodeEntry {
	uint32_t memAddress1;
	uint32_t memAddress2;
	int16_t posX;
	int16_t posY;
	int16_t posZ;
	int16_t distHeuristic;
	int16_t linkIndex;
	uint16_t areaId;
	uint16_t nodeId;
	uint8_t pathWidth;
	uint8_t floodGroup;
	uint32_t flags;
};

struct RawNaviNodeEntry {
	int16_t posX;
	int16_t posY;
	uint16_t targetAreaId;
	uint16_t targetNodeId;
	int8_t dirX;
	int8_t dirY;
	uint8_t pathWidth;
	uint8_t flags1;
	uint16_t flags2;
};

struct RawLinkEntry {
	uint16_t targetAreaId;
	uint16_t targetNodeId;
};
#pragma pack(pop)

RoadsManager& RoadsManager::Instance()
{
	static RoadsManager s_instance;
	return s_instance;
}

bool RoadsManager::Initialize(const std::string& customDir)
{
	if (m_bLoaded) return true;

	InitDefaultModelProfiles();

	ClientLog(LogLevel::Info, "RoadsManager: Initializing roads database from native nodes*.dat...");

	// 1. Primary: Load native nodes*.dat directly from models/gta3.img or data/paths/
	if (LoadFromNodesDat(customDir)) {
		BuildSpatialGrid();
		m_bLoaded = true;
		ClientLog(LogLevel::Info, std::format("RoadsManager: Successfully loaded {} nodes and {} navis from native nodes*.dat.", m_nodes.size(), m_navis.size()));
		return true;
	}

	// 2. Secondary fallback: Dynamic extraction from GTA SA engine's ThePaths (CPathFind)
	if (LoadFromEnginePaths()) {
		BuildSpatialGrid();
		m_bLoaded = true;
		ClientLog(LogLevel::Info, std::format("RoadsManager: Dynamically loaded {} nodes and {} navis from engine ThePaths.", m_nodes.size(), m_navis.size()));
		return true;
	}

	ClientLog(LogLevel::Info, "RoadsManager: Initialized in dynamic-only mode (custom roads, raycasting, and engine paths).");
	m_bLoaded = true;
	return true;
}

static bool ParseNodeFilename(std::string_view filename, int& outAreaId)
{
	// Matches: "nodes0.dat", "NODES15.DAT", "Nodes63.Dat", etc. (case-insensitive)
	if (filename.size() < 10) return false;

	size_t len = filename.size();
	char prefix[6] = { 0 };
	for (int i = 0; i < 5; ++i) {
		prefix[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(filename[i])));
	}
	if (std::memcmp(prefix, "nodes", 5) != 0) return false;

	char suffix[5] = { 0 };
	for (int i = 0; i < 4; ++i) {
		suffix[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(filename[len - 4 + i])));
	}
	if (std::memcmp(suffix, ".dat", 4) != 0) return false;

	int area = 0;
	bool hasDigit = false;
	for (size_t i = 5; i < len - 4; ++i) {
		char c = filename[i];
		if (c < '0' || c > '9') return false;
		area = area * 10 + (c - '0');
		hasDigit = true;
	}

	if (hasDigit && area >= 0 && area < 64) {
		outAreaId = area;
		return true;
	}
	return false;
}

bool RoadsManager::LoadFromNodesDat(const std::string& customDir)
{
	namespace fs = std::filesystem;

	std::vector<std::vector<uint8_t>> areaBuffers(64);
	bool foundAny = false;

	// 1. Gather base roots: customDir, game root exe dir, plugin dll dir, CWD
	std::vector<fs::path> baseRoots;
	if (!customDir.empty()) {
		std::error_code ec;
		baseRoots.push_back(fs::path(customDir));
	}

	// Game root directory from gta_sa.exe
	char exePath[MAX_PATH] = { 0 };
	if (GetModuleFileNameA(NULL, exePath, MAX_PATH) > 0) {
		std::error_code ec;
		baseRoots.push_back(fs::path(exePath).parent_path());
	}

	// Plugin directory from ASI module
	char dllPath[MAX_PATH] = { 0 };
	if (GetModuleFileNameA(ExtendedGamePlugin::Instance().GetPluginModule(), dllPath, MAX_PATH) > 0) {
		std::error_code ec;
		baseRoots.push_back(fs::path(dllPath).parent_path());
	}

	// Current working directory
	std::error_code ecCwd;
	baseRoots.push_back(fs::current_path(ecCwd));

	// Relative candidate subdirectories (case variations and common layouts)
	std::vector<std::string> relativeSubDirs = {
		"data/Paths",
		"data/paths",
		"data/PATHS"
	};

	std::vector<fs::path> candidateDirs;
	for (const auto& base : baseRoots) {
		for (const auto& sub : relativeSubDirs) {
			fs::path p = sub.empty() ? base : (base / sub);
			std::error_code ec;
			if (fs::exists(p, ec) && fs::is_directory(p, ec)) {
				fs::path can = fs::weakly_canonical(p, ec);
				if (!can.empty()) {
					if (std::find(candidateDirs.begin(), candidateDirs.end(), can) == candidateDirs.end()) {
						candidateDirs.push_back(can);
					}
				} else {
					candidateDirs.push_back(p);
				}
			}
		}
	}

	// Scan candidate directories for NODES*.DAT (case-insensitive)
	for (const auto& dirPath : candidateDirs) {
		int areasFoundInDir = 0;
		std::vector<std::vector<uint8_t>> tempBuffers(64);

		std::error_code ecDir;
		for (const auto& entry : fs::directory_iterator(dirPath, fs::directory_options::skip_permission_denied, ecDir)) {
			std::error_code ecFile;
			if (!entry.is_regular_file(ecFile)) continue;

			std::string fname = entry.path().filename().string();
			int areaId = -1;
			if (ParseNodeFilename(fname, areaId)) {
				std::ifstream lf(entry.path(), std::ios::binary | std::ios::ate);
				if (lf.is_open()) {
					auto sz = lf.tellg();
					if (sz > static_cast<std::streampos>(sizeof(RawNodeHeader))) {
						tempBuffers[areaId].resize(static_cast<size_t>(sz));
						lf.seekg(0, std::ios::beg);
						lf.read(reinterpret_cast<char*>(tempBuffers[areaId].data()), sz);
						areasFoundInDir++;
					}
				}
			}
		}

		if (areasFoundInDir > 0) {
			areaBuffers = std::move(tempBuffers);
			foundAny = true;
			ClientLog(LogLevel::Info, std::format("RoadsManager: Loaded {} NODES*.DAT files from relative path '{}'",
				areasFoundInDir, dirPath.string()));
			break;
		}
	}

	// 2. If loose files not found, search in models/gta3.img (case-insensitive)
	if (!foundAny) {
		std::vector<std::string> relImgPaths = {
			"models/gta3.img",
			"models/GTA3.IMG",
			"models/gta3.IMG",
			"models/GTA3.img"
		};

		std::vector<fs::path> candidateImgs;
		for (const auto& base : baseRoots) {
			for (const auto& rel : relImgPaths) {
				fs::path p = base / rel;
				std::error_code ec;
				if (fs::exists(p, ec) && fs::is_regular_file(p, ec)) {
					candidateImgs.push_back(p);
				}
			}
		}

		for (const auto& imgPath : candidateImgs) {
			std::ifstream imgFile(imgPath, std::ios::binary);
			if (!imgFile.is_open()) continue;

			ImgHeader hdr;
			imgFile.read(reinterpret_cast<char*>(&hdr), sizeof(ImgHeader));
			if (!imgFile || std::memcmp(hdr.tag, "VER2", 4) != 0) {
				continue;
			}

			std::vector<ImgDirEntry> entries(hdr.entryCount);
			imgFile.read(reinterpret_cast<char*>(entries.data()), hdr.entryCount * sizeof(ImgDirEntry));
			if (!imgFile) continue;

			struct NodeEntryLoc {
				uint32_t offsetBytes = 0;
				uint32_t sizeBytes = 0;
			};
			std::vector<NodeEntryLoc> nodeLocs(64);
			int nodeCount = 0;

			for (const auto& e : entries) {
				char nameBuf[25];
				std::memcpy(nameBuf, e.name, 24);
				nameBuf[24] = '\0';

				int areaId = -1;
				if (ParseNodeFilename(nameBuf, areaId)) {
					nodeLocs[areaId].offsetBytes = e.offset * 2048;
					nodeLocs[areaId].sizeBytes = e.sizeInSectors * 2048;
					nodeCount++;
				}
			}

			if (nodeCount > 0) {
				for (int a = 0; a < 64; ++a) {
					if (nodeLocs[a].sizeBytes > sizeof(RawNodeHeader)) {
						areaBuffers[a].resize(nodeLocs[a].sizeBytes);
						imgFile.seekg(nodeLocs[a].offsetBytes, std::ios::beg);
						imgFile.read(reinterpret_cast<char*>(areaBuffers[a].data()), nodeLocs[a].sizeBytes);
					}
				}
				foundAny = true;
				ClientLog(LogLevel::Info, std::format("RoadsManager: Loaded {} NODES*.DAT archives directly from '{}'", nodeCount, imgPath.string()));
				break;
			}
		}
	}

	if (!foundAny) {
		return false;
	}

	// Mapping from (areaId, localNodeId) -> globalNodeId
	std::vector<std::vector<int32_t>> areaNodeToGlobal(64);
	std::vector<std::vector<int32_t>> areaNaviToGlobal(64);

	m_nodes.clear();
	m_navis.clear();

	// Pass 1: Parse Vehicle Nodes & Navi Nodes
	for (int area = 0; area < 64; ++area) {
		if (areaBuffers[area].size() < sizeof(RawNodeHeader)) continue;

		const auto* pHeader = reinterpret_cast<const RawNodeHeader*>(areaBuffers[area].data());
		uint32_t numNodes = pHeader->numNodes;
		uint32_t numVeh = pHeader->numVehicleNodes;
		uint32_t numNavi = pHeader->numNaviNodes;

		areaNodeToGlobal[area].assign(numNodes, -1);
		areaNaviToGlobal[area].assign(numNavi, -1);

		size_t sec1Offset = sizeof(RawNodeHeader);
		if (sec1Offset + (numNodes * sizeof(RawPathNodeEntry)) > areaBuffers[area].size())
			continue;

		const auto* pRawNodes = reinterpret_cast<const RawPathNodeEntry*>(areaBuffers[area].data() + sec1Offset);
		for (uint32_t i = 0; i < numVeh; ++i) {
			const auto& rawNode = pRawNodes[i];
			// Water nodes: bit 4 of flags (0x10)
			if ((rawNode.flags & (1 << 4)) != 0) continue;

			RoadNode rNode;
			rNode.id = static_cast<uint32_t>(m_nodes.size());
			rNode.pos.x = static_cast<float>(rawNode.posX) / 8.0f;
			rNode.pos.y = static_cast<float>(rawNode.posY) / 8.0f;
			rNode.pos.z = static_cast<float>(rawNode.posZ) / 8.0f;
			rNode.linkCount = 0;

			areaNodeToGlobal[area][i] = static_cast<int32_t>(rNode.id);
			m_nodes.push_back(rNode);
		}

		size_t sec2Offset = sec1Offset + (numNodes * sizeof(RawPathNodeEntry));
		if (sec2Offset + (numNavi * sizeof(RawNaviNodeEntry)) > areaBuffers[area].size())
			continue;

		const auto* pRawNavis = reinterpret_cast<const RawNaviNodeEntry*>(areaBuffers[area].data() + sec2Offset);
		for (uint32_t j = 0; j < numNavi; ++j) {
			const auto& rawNavi = pRawNavis[j];
			RoadNavi rNavi;
			rNavi.from = -1;
			rNavi.to = -1;
			rNavi.leftLanes = rawNavi.flags1 & 0x07;
			rNavi.rightLanes = (rawNavi.flags1 >> 3) & 0x07;

			areaNaviToGlobal[area][j] = static_cast<int32_t>(m_navis.size());
			m_navis.push_back(rNavi);
		}
	}

	// Pass 2: Resolve Links & Navi Connections
	for (int area = 0; area < 64; ++area) {
		if (areaBuffers[area].size() < sizeof(RawNodeHeader)) continue;

		const auto* pHeader = reinterpret_cast<const RawNodeHeader*>(areaBuffers[area].data());
		uint32_t numNodes = pHeader->numNodes;
		uint32_t numVeh = pHeader->numVehicleNodes;
		uint32_t numNavi = pHeader->numNaviNodes;
		uint32_t numLinks = pHeader->numLinks;

		size_t sec1Offset = sizeof(RawNodeHeader);
		size_t sec3Offset = sec1Offset + (numNodes * sizeof(RawPathNodeEntry)) + (numNavi * sizeof(RawNaviNodeEntry));
		size_t sec5Offset = sec3Offset + (numLinks * sizeof(RawLinkEntry)) + 768; // 768 bytes filler in Section 4

		if (sec5Offset + (numLinks * sizeof(uint16_t)) > areaBuffers[area].size())
			continue;

		const auto* pRawNodes = reinterpret_cast<const RawPathNodeEntry*>(areaBuffers[area].data() + sec1Offset);
		const auto* pRawLinks = reinterpret_cast<const RawLinkEntry*>(areaBuffers[area].data() + sec3Offset);
		const auto* pRawNaviLinks = reinterpret_cast<const uint16_t*>(areaBuffers[area].data() + sec5Offset);

		for (uint32_t i = 0; i < numVeh; ++i) {
			if (i >= areaNodeToGlobal[area].size()) continue;
			int32_t globalNodeId = areaNodeToGlobal[area][i];
			if (globalNodeId < 0 || globalNodeId >= static_cast<int32_t>(m_nodes.size()))
				continue;

			const auto& rawNode = pRawNodes[i];
			uint32_t numAdj = rawNode.flags & 0x0F;
			int adjCount = std::min<int>(static_cast<int>(numAdj), 4);
			int16_t linkBase = rawNode.linkIndex;

			RoadNode& rNode = m_nodes[globalNodeId];
			for (int k = 0; k < adjCount; ++k) {
				uint32_t linkIdx = static_cast<uint32_t>(linkBase + k);
				if (linkIdx >= numLinks) continue;

				const auto& rawLink = pRawLinks[linkIdx];
				uint16_t tArea = rawLink.targetAreaId;
				uint16_t tNode = rawLink.targetNodeId;

				int32_t targetGlobalId = -1;
				if (tArea < 64 && tNode < areaNodeToGlobal[tArea].size()) {
					targetGlobalId = areaNodeToGlobal[tArea][tNode];
				}

				uint16_t rawNaviVal = pRawNaviLinks[linkIdx];
				uint16_t naviNode = rawNaviVal & 0x03FF;
				uint16_t naviArea = (rawNaviVal >> 10) & 0x003F;

				int32_t naviGlobalId = -1;
				if (naviArea < 64 && naviNode < areaNaviToGlobal[naviArea].size()) {
					naviGlobalId = areaNaviToGlobal[naviArea][naviNode];
				}

				if (targetGlobalId != -1 && rNode.linkCount < 4) {
					rNode.links[rNode.linkCount].targetNodeId = targetGlobalId;
					rNode.links[rNode.linkCount].naviId = naviGlobalId;
					rNode.linkCount++;

					if (naviGlobalId != -1 && naviGlobalId < static_cast<int32_t>(m_navis.size())) {
						m_navis[naviGlobalId].from = globalNodeId;
						m_navis[naviGlobalId].to = targetGlobalId;
					}
				}
			}
		}
	}

	return (!m_nodes.empty());
}

bool RoadsManager::LoadFromEnginePaths()
{
	m_nodes.clear();
	m_navis.clear();

	bool hasPathData = false;
	for (int area = 0; area < NUM_PATH_MAP_AREAS; ++area) {
		if (ThePaths.m_pPathNodes[area] && ThePaths.m_dwNumNodes[area] > 0) {
			hasPathData = true;
			break;
		}
	}

	if (!hasPathData) {
		ThePaths.UpdateStreaming(true);
	}

	uint32_t globalNodeId = 0;
	for (int area = 0; area < NUM_PATH_MAP_AREAS; ++area) {
		if (!ThePaths.m_pPathNodes[area]) continue;

		unsigned int numVehNodes = ThePaths.m_dwNumVehicleNodes[area];
		for (unsigned int n = 0; n < numVehNodes; ++n) {
			CPathNode* pNode = &ThePaths.m_pPathNodes[area][n];
			if (!pNode || pNode->WaterNode) continue;

			RoadNode rNode;
			rNode.id = globalNodeId++;
			rNode.pos = pNode->GetNodeCoors();
			rNode.linkCount = 0;

			if (ThePaths.m_pNodeLinks[area] && pNode->IndexAdjacentNodes >= 0) {
				int adjCount = std::min<int>(pNode->NumberAdjNodes, 4);
				for (int k = 0; k < adjCount; ++k) {
					CNodeAddress linkAddr = ThePaths.m_pNodeLinks[area][pNode->IndexAdjacentNodes + k];
					if (!linkAddr.IsEmpty()) {
						rNode.links[rNode.linkCount].targetNodeId = linkAddr.Index;
						rNode.links[rNode.linkCount].naviId = linkAddr.Region;
						rNode.linkCount++;
					}
				}
			}
			m_nodes.push_back(rNode);
		}
	}

	// Car path links (lane count and width)
	for (int area = 0; area < NUM_PATH_MAP_AREAS; ++area) {
		if (!ThePaths.m_pNaviNodes[area]) continue;
		unsigned int numLinks = ThePaths.m_dwNumCarPathLinks[area];
		for (unsigned int l = 0; l < numLinks; ++l) {
			CCarPathLink* pLink = &ThePaths.m_pNaviNodes[area][l];
			if (!pLink) continue;

			RoadNavi rNavi;
			rNavi.from = pLink->m_address.Index;
			rNavi.to = -1;
			rNavi.leftLanes = pLink->m_nNumLeftLanes;
			rNavi.rightLanes = pLink->m_nNumRightLanes;
			m_navis.push_back(rNavi);
		}
	}

	return (!m_nodes.empty());
}

void RoadsManager::Shutdown()
{
	m_nodes.clear();
	m_navis.clear();
	m_customSegments.clear();
	m_objectIdToCustomIdx.clear();
	m_modelProfiles.clear();

	for (int x = 0; x < GRID_DIM; ++x) {
		for (int y = 0; y < GRID_DIM; ++y) {
			m_grid[x][y].clear();
			m_customGrid[x][y].clear();
		}
	}
	m_bLoaded = false;
	ClientLog(LogLevel::Info, "RoadsManager: Shutdown complete.");
}

void RoadsManager::InitDefaultModelProfiles()
{
	// Common GTA SA and SA-MP modular road object profiles
	// Model 3458: Straight Road (20m x 8m, 2 lanes)
	RegisterRoadModelProfile(3458, 20.0f, 8.0f, 1, 1, 1);
	RegisterRoadModelProfile(3459, 40.0f, 8.0f, 1, 1, 1);
	RegisterRoadModelProfile(3460, 60.0f, 8.0f, 1, 1, 1);
	RegisterRoadModelProfile(3470, 20.0f, 16.0f, 2, 2, 1);
	RegisterRoadModelProfile(3471, 40.0f, 16.0f, 2, 2, 1);
	RegisterRoadModelProfile(3472, 60.0f, 16.0f, 2, 2, 1);

	// Stunt / modular track road pieces (18450..18460)
	for (uint16_t m = 18450; m <= 18460; ++m) {
		RegisterRoadModelProfile(m, 25.0f, 10.0f, 1, 1, 1);
	}
}

void RoadsManager::RegisterRoadModelProfile(uint16_t modelId, float length, float width,
	uint8_t leftLanes, uint8_t rightLanes, int forwardAxis)
{
	RoadModelProfile prof;
	prof.modelId = modelId;
	prof.length = (length > 0.1f) ? length : 20.0f;
	prof.width = (width > 0.1f) ? width : 8.0f;
	prof.leftLanes = leftLanes;
	prof.rightLanes = rightLanes;
	prof.forwardAxis = forwardAxis;
	m_modelProfiles[modelId] = prof;
}

void RoadsManager::BuildSpatialGrid()
{
	for (int x = 0; x < GRID_DIM; ++x) {
		for (int y = 0; y < GRID_DIM; ++y) {
			m_grid[x][y].clear();
		}
	}

	for (size_t i = 0; i < m_nodes.size(); ++i) {
		const auto& n = m_nodes[i];
		int cx = static_cast<int>((n.pos.x + GRID_OFFSET) / GRID_CELL_SIZE);
		int cy = static_cast<int>((n.pos.y + GRID_OFFSET) / GRID_CELL_SIZE);
		if (cx >= 0 && cx < GRID_DIM && cy >= 0 && cy < GRID_DIM) {
			m_grid[cx][cy].push_back(static_cast<uint32_t>(i));
		}
	}
}

void RoadsManager::AddCustomSegmentToGrid(uint32_t segIdx)
{
	if (segIdx >= m_customSegments.size()) return;
	const auto& seg = m_customSegments[segIdx];

	// Insert into all grid cells that the segment bounding box intersects
	float minX = std::min(seg.startPos.x, seg.endPos.x) - (seg.width * 0.5f);
	float maxX = std::max(seg.startPos.x, seg.endPos.x) + (seg.width * 0.5f);
	float minY = std::min(seg.startPos.y, seg.endPos.y) - (seg.width * 0.5f);
	float maxY = std::max(seg.startPos.y, seg.endPos.y) + (seg.width * 0.5f);

	int startCx = std::clamp(static_cast<int>((minX + GRID_OFFSET) / GRID_CELL_SIZE), 0, GRID_DIM - 1);
	int endCx = std::clamp(static_cast<int>((maxX + GRID_OFFSET) / GRID_CELL_SIZE), 0, GRID_DIM - 1);
	int startCy = std::clamp(static_cast<int>((minY + GRID_OFFSET) / GRID_CELL_SIZE), 0, GRID_DIM - 1);
	int endCy = std::clamp(static_cast<int>((maxY + GRID_OFFSET) / GRID_CELL_SIZE), 0, GRID_DIM - 1);

	for (int cx = startCx; cx <= endCx; ++cx) {
		for (int cy = startCy; cy <= endCy; ++cy) {
			m_customGrid[cx][cy].push_back(segIdx);
		}
	}
}

bool RoadsManager::RegisterCustomRoadObject(uint32_t objectId, uint16_t modelId, const CVector& pos, const CVector& rot,
	uint8_t leftLanes, uint8_t rightLanes, float width)
{
	// Look up model profile or fallback
	float segLength = 20.0f;
	float segWidth = (width > 0.5f) ? width : 8.0f;
	uint8_t lCount = leftLanes;
	uint8_t rCount = rightLanes;

	auto it = m_modelProfiles.find(modelId);
	if (it != m_modelProfiles.end()) {
		segLength = it->second.length;
		if (width <= 0.5f) segWidth = it->second.width;
		if (leftLanes == 1 && rightLanes == 1 && (it->second.leftLanes != 1 || it->second.rightLanes != 1)) {
			lCount = it->second.leftLanes;
			rCount = it->second.rightLanes;
		}
	}

	// Calculate 3D orientation vectors from Euler angles (rx=pitch, ry=roll, rz=yaw in degrees)
	float pitch = rot.x * BT_DEG_TO_RAD;
	float roll = rot.y * BT_DEG_TO_RAD;
	float yaw = rot.z * BT_DEG_TO_RAD;

	// Forward vector (local Y)
	CVector forward;
	forward.x = -std::sin(yaw) * std::cos(pitch);
	forward.y = std::cos(yaw) * std::cos(pitch);
	forward.z = std::sin(pitch);
	float fLen = std::sqrt(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
	if (fLen > 0.0001f) forward = forward / fLen;

	// Right vector (local X)
	CVector right;
	right.x = std::cos(yaw) * std::cos(roll) + std::sin(yaw) * std::sin(pitch) * std::sin(roll);
	right.y = std::sin(yaw) * std::cos(roll) - std::cos(yaw) * std::sin(pitch) * std::sin(roll);
	right.z = -std::cos(pitch) * std::sin(roll);
	float rLen = std::sqrt(right.x * right.x + right.y * right.y + right.z * right.z);
	if (rLen > 0.0001f) right = right / rLen;

	// Up / Surface Normal vector (local Z)
	CVector up;
	up.x = std::sin(yaw) * std::sin(pitch) * std::cos(roll) - std::cos(yaw) * std::sin(roll);
	up.y = -std::cos(yaw) * std::sin(pitch) * std::cos(roll) - std::sin(yaw) * std::sin(roll);
	up.z = std::cos(pitch) * std::cos(roll);
	float uLen = std::sqrt(up.x * up.x + up.y * up.y + up.z * up.z);
	if (uLen > 0.0001f) up = up / uLen;

	CustomRoadSegment seg;
	seg.id = static_cast<uint32_t>(m_customSegments.size());
	seg.objectId = objectId;
	seg.modelId = modelId;
	seg.centerPos = pos;
	seg.forward = forward;
	seg.right = right;
	seg.surfaceNormal = up;
	seg.length = segLength;
	seg.width = segWidth;
	seg.leftLanes = lCount;
	seg.rightLanes = rCount;
	seg.laneWidth = segWidth / static_cast<float>(std::max(1, static_cast<int>(lCount + rCount)));

	float halfL = segLength * 0.5f;
	seg.startPos = pos - (forward * halfL);
	seg.endPos = pos + (forward * halfL);

	// Auto-stitch with existing custom segments (if endpoint within 2.5m)
	for (size_t i = 0; i < m_customSegments.size(); ++i) {
		auto& other = m_customSegments[i];
		// If existing end connects to new start
		float dEndStart = (other.endPos - seg.startPos).Magnitude();
		if (dEndStart < 2.5f) {
			other.nextSegmentId = seg.id;
			seg.prevSegmentId = static_cast<int32_t>(i);
		}
		// If existing start connects to new end
		float dStartEnd = (other.startPos - seg.endPos).Magnitude();
		if (dStartEnd < 2.5f) {
			other.prevSegmentId = seg.id;
			seg.nextSegmentId = static_cast<int32_t>(i);
		}
	}

	m_customSegments.push_back(seg);
	if (objectId != 0) {
		m_objectIdToCustomIdx[objectId] = seg.id;
	}
	AddCustomSegmentToGrid(seg.id);
	return true;
}

bool RoadsManager::RegisterCustomRoadSegment(const CVector& startPos, const CVector& endPos,
	uint8_t leftLanes, uint8_t rightLanes, float laneWidth)
{
	CVector diff = endPos - startPos;
	float len = diff.Magnitude();
	if (len < 0.1f) return false;

	CVector fwd = diff / len;
	CVector rt = { fwd.y, -fwd.x, 0.0f };
	float rtLen = std::sqrt(rt.x * rt.x + rt.y * rt.y);
	if (rtLen > 0.0001f) rt = rt / rtLen;

	uint8_t lCount = std::max<uint8_t>(1, leftLanes);
	uint8_t rCount = std::max<uint8_t>(1, rightLanes);
	float lWidth = (laneWidth > 0.5f) ? laneWidth : 4.0f;
	float totalWidth = static_cast<float>(lCount + rCount) * lWidth;

	CustomRoadSegment seg;
	seg.id = static_cast<uint32_t>(m_customSegments.size());
	seg.objectId = 0;
	seg.modelId = 0;
	seg.startPos = startPos;
	seg.endPos = endPos;
	seg.centerPos = (startPos + endPos) * 0.5f;
	seg.forward = fwd;
	seg.right = rt;
	seg.surfaceNormal = { 0.0f, 0.0f, 1.0f };
	seg.length = len;
	seg.width = totalWidth;
	seg.laneWidth = lWidth;
	seg.leftLanes = lCount;
	seg.rightLanes = rCount;

	// Stitching
	for (size_t i = 0; i < m_customSegments.size(); ++i) {
		auto& other = m_customSegments[i];
		if ((other.endPos - seg.startPos).Magnitude() < 2.5f) {
			other.nextSegmentId = seg.id;
			seg.prevSegmentId = static_cast<int32_t>(i);
		}
		if ((other.startPos - seg.endPos).Magnitude() < 2.5f) {
			other.prevSegmentId = seg.id;
			seg.nextSegmentId = static_cast<int32_t>(i);
		}
	}

	m_customSegments.push_back(seg);
	AddCustomSegmentToGrid(seg.id);
	return true;
}

void RoadsManager::ClearCustomRoads()
{
	m_customSegments.clear();
	m_objectIdToCustomIdx.clear();
	for (int x = 0; x < GRID_DIM; ++x) {
		for (int y = 0; y < GRID_DIM; ++y) {
			m_customGrid[x][y].clear();
		}
	}
	ClientLog(LogLevel::Info, "RoadsManager: Cleared all custom road segments.");
}

const CustomRoadSegment* RoadsManager::GetCustomRoad(uint32_t id) const
{
	if (id < m_customSegments.size()) {
		return &m_customSegments[id];
	}
	return nullptr;
}

int32_t RoadsManager::FindClosestCustomRoad(const CVector& pos, float maxRadius, float* outDist) const
{
	int cx = static_cast<int>((pos.x + GRID_OFFSET) / GRID_CELL_SIZE);
	int cy = static_cast<int>((pos.y + GRID_OFFSET) / GRID_CELL_SIZE);

	float bestDist = maxRadius;
	int32_t bestId = -1;

	for (int dx = -1; dx <= 1; ++dx) {
		int scx = cx + dx;
		if (scx < 0 || scx >= GRID_DIM) continue;

		for (int dy = -1; dy <= 1; ++dy) {
			int scy = cy + dy;
			if (scy < 0 || scy >= GRID_DIM) continue;

			for (uint32_t segIdx : m_customGrid[scx][scy]) {
				if (segIdx >= m_customSegments.size()) continue;
				const auto& seg = m_customSegments[segIdx];

				// Distance from point to segment line
				CVector segVec = seg.endPos - seg.startPos;
				float segLen = seg.length;
				if (segLen < 0.001f) continue;

				float t = ((pos.x - seg.startPos.x) * seg.forward.x +
				           (pos.y - seg.startPos.y) * seg.forward.y +
				           (pos.z - seg.startPos.z) * seg.forward.z) / segLen;
				t = std::clamp(t, 0.0f, 1.0f);
				CVector proj = seg.startPos + (seg.forward * (t * segLen));
				float dist = (pos - proj).Magnitude();

				if (dist < bestDist) {
					bestDist = dist;
					bestId = static_cast<int32_t>(segIdx);
				}
			}
		}
	}

	if (outDist) *outDist = bestDist;
	return bestId;
}

void RoadsManager::AutoScanWorldRoadObjects()
{
	// Inspect CPools::ms_pObjectPool
	if (!CPools::ms_pObjectPool) return;

	int poolSize = CPools::ms_pObjectPool->m_nSize;
	for (int i = 0; i < poolSize; ++i) {
		if (CPools::ms_pObjectPool->IsFreeSlotAtIndex(i)) continue;

		CObject* pObj = CPools::ms_pObjectPool->GetAt(i);
		if (!pObj) continue;

		uint16_t modelId = pObj->m_nModelIndex;
		auto it = m_modelProfiles.find(modelId);
		if (it != m_modelProfiles.end()) {
			CVector objPos = pObj->GetPosition();

			// Check if already registered
			float existingDist = 0.0f;
			int32_t nearest = FindClosestCustomRoad(objPos, 2.0f, &existingDist);
			if (nearest == -1) {
				// Read rotation or heading
				CVector rot = { 0.0f, 0.0f, 0.0f };
				pObj->GetOrientation(rot.x, rot.y, rot.z);
				RegisterCustomRoadObject(static_cast<uint32_t>(i), modelId, objPos, rot, it->second.leftLanes, it->second.rightLanes, it->second.width);
			}
		}
	}
}

bool RoadsManager::RaycastRoadSurface(const CVector& testPos, RoadRaycastResult& outResult) const
{
	CVector origin = testPos;
	origin.z += 2.5f; // Cast downward from above vehicle/ped
	float distance = 10.0f;

	CColPoint colPoint;
	CEntity* hitEntity = nullptr;

	bool hit = CWorld::ProcessVerticalLine(
		origin,
		distance,
		colPoint,
		hitEntity,
		true,  // buildings
		false, // vehicles
		false, // peds
		true,  // objects
		false, // dummies
		true,  // doSeeThroughCheck
		nullptr // outCollPoly
	);

	if (hit) {
		outResult.hit = true;
		outResult.surfacePos = colPoint.m_vecPoint;
		outResult.surfaceNormal = colPoint.m_vecNormal;

		float nz = std::clamp(outResult.surfaceNormal.z, -1.0f, 1.0f);
		outResult.inclineDeg = std::acos(nz) * BT_RAD_TO_DEG;

		if (hitEntity) {
			outResult.hitModelId = hitEntity->m_nModelIndex;
			outResult.isCustomRoadObject = (m_modelProfiles.find(outResult.hitModelId) != m_modelProfiles.end());
		}
		return true;
	}

	outResult.hit = false;
	return false;
}

int32_t RoadsManager::FindClosestNode(const CVector& pos, float maxRadius, float* outDist) const
{
	int cx = static_cast<int>((pos.x + GRID_OFFSET) / GRID_CELL_SIZE);
	int cy = static_cast<int>((pos.y + GRID_OFFSET) / GRID_CELL_SIZE);

	float bestDistSq = maxRadius * maxRadius;
	int32_t bestId = -1;

	for (int dx = -1; dx <= 1; ++dx) {
		int scx = cx + dx;
		if (scx < 0 || scx >= GRID_DIM) continue;

		for (int dy = -1; dy <= 1; ++dy) {
			int scy = cy + dy;
			if (scy < 0 || scy >= GRID_DIM) continue;

			const auto& cellNodes = m_grid[scx][scy];
			for (uint32_t nodeId : cellNodes) {
				if (nodeId >= m_nodes.size()) continue;

				const auto& n = m_nodes[nodeId];
				float dxPos = n.pos.x - pos.x;
				float dyPos = n.pos.y - pos.y;
				float dzPos = n.pos.z - pos.z;
				float distSq = dxPos * dxPos + dyPos * dyPos + dzPos * dzPos;

				if (distSq < bestDistSq) {
					bestDistSq = distSq;
					bestId = static_cast<int32_t>(nodeId);
				}
			}
		}
	}

	if (outDist) {
		*outDist = (bestId != -1) ? std::sqrt(bestDistSq) : maxRadius;
	}
	return bestId;
}

const RoadNode* RoadsManager::GetNode(uint32_t id) const
{
	if (id < m_nodes.size()) {
		return &m_nodes[id];
	}
	return nullptr;
}

const RoadNavi* RoadsManager::GetNavi(uint32_t id) const
{
	if (id < m_navis.size()) {
		return &m_navis[id];
	}
	return nullptr;
}

std::vector<TrafficTurnOption> RoadsManager::GetIntersectionTurnOptions(int32_t fromNodeId, int32_t atNodeId) const
{
	std::vector<TrafficTurnOption> options;
	const RoadNode* pAt = GetNode(atNodeId);
	if (!pAt) return options;

	float inHeading = 0.0f;
	const RoadNode* pFrom = GetNode(fromNodeId);
	if (pFrom) {
		inHeading = std::atan2(pAt->pos.y - pFrom->pos.y, pAt->pos.x - pFrom->pos.x) * BT_RAD_TO_DEG - 90.0f;
		inHeading = NormalizeAngle(inHeading);
	}

	for (int i = 0; i < 4; ++i) {
		int32_t targetId = pAt->links[i].targetNodeId;
		if (targetId == -1 || targetId == fromNodeId) continue;

		const RoadNode* pTarget = GetNode(targetId);
		if (!pTarget) continue;

		float outHeading = std::atan2(pTarget->pos.y - pAt->pos.y, pTarget->pos.x - pAt->pos.x) * BT_RAD_TO_DEG - 90.0f;
		outHeading = NormalizeAngle(outHeading);

		float deltaAngle = NormalizeAngleDiff(outHeading - inHeading);

		TrafficTurnOption opt;
		opt.targetNodeId = targetId;
		opt.naviId = pAt->links[i].naviId;
		opt.turnAngle = deltaAngle;

		const RoadNavi* pNavi = GetNavi(opt.naviId);
		if (pNavi) {
			opt.targetLaneCount = pNavi->leftLanes + pNavi->rightLanes;
		}

		if (std::abs(deltaAngle) <= 35.0f) {
			opt.maneuver = TRAFFIC_MANEUVER_STRAIGHT;
		} else if (deltaAngle > 35.0f && deltaAngle <= 135.0f) {
			opt.maneuver = TRAFFIC_MANEUVER_RIGHT_TURN;
		} else if (deltaAngle < -35.0f && deltaAngle >= -135.0f) {
			opt.maneuver = TRAFFIC_MANEUVER_LEFT_TURN;
		} else {
			opt.maneuver = TRAFFIC_MANEUVER_UTURN;
		}

		options.push_back(opt);
	}

	return options;
}

bool RoadsManager::GetLaneWaypoint(int32_t fromNodeId, int32_t toNodeId, int8_t side, uint8_t laneIdx,
	float progress, CVector& outPos, CVector& outTangent) const
{
	const RoadNode* pFrom = GetNode(fromNodeId);
	const RoadNode* pTo = GetNode(toNodeId);
	if (!pFrom || !pTo) return false;

	CVector diff = pTo->pos - pFrom->pos;
	float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
	if (len < 0.001f) return false;

	CVector fwd = { diff.x / len, diff.y / len, diff.z / len };
	CVector rt = { fwd.y, -fwd.x, 0.0f };

	float pClamped = std::clamp(progress, 0.0f, 1.0f);
	CVector centerlinePos = pFrom->pos + (diff * pClamped);

	float laneWidth = 3.75f;
	float laneOffset = (static_cast<float>(laneIdx) + 0.5f) * laneWidth;
	if (side == ROAD_SIDE_LEFT) {
		laneOffset = -laneOffset;
		outTangent = -fwd;
	} else {
		outTangent = fwd;
	}

	outPos = centerlinePos + (rt * laneOffset);
	return true;
}

TrafficQueryResult RoadsManager::QueryTraffic(const CVector& pos, float headingDeg, float lookaheadDist) const
{
	TrafficQueryResult res;
	res.steering.distanceToTarget = lookaheadDist;

	// 1. Raycast ground surface beneath query position
	RaycastRoadSurface(pos, res.raycast);

	// 2. Check Custom Road Segments first
	float customDist = 9999.0f;
	int32_t customIdx = FindClosestCustomRoad(pos, 80.0f, &customDist);

	// 3. Check Native Road Nodes
	float nativeDist = 9999.0f;
	int32_t nativeNodeId = FindClosestNode(pos, 120.0f, &nativeDist);

	// Prefer custom road if within reach and closer or on top of it
	if (customIdx != -1 && (customDist <= nativeDist || res.raycast.isCustomRoadObject)) {
		const auto& seg = m_customSegments[customIdx];
		res.isCustomRoad = true;
		res.customRoadObjectId = seg.objectId;
		res.segmentStart = seg.startPos;
		res.segmentEnd = seg.endPos;
		res.segmentLength = seg.length;

		float segLen = std::max(0.01f, seg.length);
		CVector fwd = seg.forward;
		CVector rt = seg.right;

		float t = ((pos.x - seg.startPos.x) * fwd.x +
		           (pos.y - seg.startPos.y) * fwd.y +
		           (pos.z - seg.startPos.z) * fwd.z) / segLen;
		res.longitudinalProgress = std::clamp(t, 0.0f, 1.0f);

		CVector centerPoint = seg.startPos + (fwd * (res.longitudinalProgress * segLen));
		CVector toPos = pos - centerPoint;
		float lateralOffset = toPos.x * rt.x + toPos.y * rt.y + toPos.z * rt.z;

		float roadHeading = std::atan2(fwd.y, fwd.x) * BT_RAD_TO_DEG - 90.0f;
		res.roadHeading = NormalizeAngle(roadHeading);

		float vehAngle = NormalizeAngle(headingDeg);
		float angleDiff = NormalizeAngleDiff(res.roadHeading - vehAngle);

		res.lane.laneWidth = seg.laneWidth;
		if (lateralOffset >= 0.0f) {
			// Right side
			res.lane.side = ROAD_SIDE_RIGHT;
			res.lane.totalLanesOnSide = seg.rightLanes;
			uint8_t lIdx = static_cast<uint8_t>(lateralOffset / std::max(0.5f, seg.laneWidth));
			res.lane.laneIndex = std::min<uint8_t>(lIdx, std::max<uint8_t>(1, seg.rightLanes) - 1);

			res.lane.lateralOffsetFromCenter = (static_cast<float>(res.lane.laneIndex) + 0.5f) * seg.laneWidth;
			res.lane.currentLateralDeviation = lateralOffset - res.lane.lateralOffsetFromCenter;
			res.lane.laneCenterPos = centerPoint + (rt * res.lane.lateralOffsetFromCenter);
			res.lane.laneForwardTangent = fwd;
			res.lane.legalHeading = res.roadHeading;
			res.lane.isAlignedWithTraffic = (std::abs(angleDiff) <= 60.0f);
			res.status = res.lane.isAlignedWithTraffic ? ROAD_STATUS_ON_RIGHT_SIDE : ROAD_STATUS_ON_WRONG_SIDE;
		} else {
			// Left side
			res.lane.side = ROAD_SIDE_LEFT;
			res.lane.totalLanesOnSide = seg.leftLanes;
			float leftPos = -lateralOffset;
			uint8_t lIdx = static_cast<uint8_t>(leftPos / std::max(0.5f, seg.laneWidth));
			res.lane.laneIndex = std::min<uint8_t>(lIdx, std::max<uint8_t>(1, seg.leftLanes) - 1);

			res.lane.lateralOffsetFromCenter = -(static_cast<float>(res.lane.laneIndex) + 0.5f) * seg.laneWidth;
			res.lane.currentLateralDeviation = leftPos - ((static_cast<float>(res.lane.laneIndex) + 0.5f) * seg.laneWidth);
			res.lane.laneCenterPos = centerPoint + (rt * res.lane.lateralOffsetFromCenter);
			res.lane.laneForwardTangent = -fwd;
			res.lane.legalHeading = NormalizeAngle(res.roadHeading + 180.0f);
			float leftAngleDiff = NormalizeAngleDiff(res.lane.legalHeading - vehAngle);
			res.lane.isAlignedWithTraffic = (std::abs(leftAngleDiff) <= 60.0f);
			res.status = res.lane.isAlignedWithTraffic ? ROAD_STATUS_ON_RIGHT_SIDE : ROAD_STATUS_ON_WRONG_SIDE;
		}

		// Lookahead Steering Target
		float targetT = (res.lane.side == ROAD_SIDE_RIGHT)
			? (res.longitudinalProgress + (lookaheadDist / segLen))
			: (res.longitudinalProgress - (lookaheadDist / segLen));

		if (targetT >= 0.0f && targetT <= 1.0f) {
			res.steering.targetPos = seg.startPos + (fwd * (targetT * segLen)) + (rt * res.lane.lateralOffsetFromCenter);
			res.steering.tangent = res.lane.laneForwardTangent;
			res.steering.targetHeading = res.lane.legalHeading;
		} else if (targetT > 1.0f && seg.nextSegmentId != -1 && static_cast<size_t>(seg.nextSegmentId) < m_customSegments.size()) {
			const auto& nextSeg = m_customSegments[seg.nextSegmentId];
			float remainderDist = (targetT - 1.0f) * segLen;
			float nextT = std::clamp(remainderDist / std::max(0.01f, nextSeg.length), 0.0f, 1.0f);
			res.steering.targetPos = nextSeg.startPos + (nextSeg.forward * (nextT * nextSeg.length)) + (nextSeg.right * res.lane.lateralOffsetFromCenter);
			res.steering.tangent = nextSeg.forward;
			res.steering.targetHeading = NormalizeAngle(std::atan2(nextSeg.forward.y, nextSeg.forward.x) * BT_RAD_TO_DEG - 90.0f);
		} else {
			res.steering.targetPos = centerPoint + (res.lane.laneForwardTangent * lookaheadDist) + (rt * res.lane.lateralOffsetFromCenter);
			res.steering.tangent = res.lane.laneForwardTangent;
			res.steering.targetHeading = res.lane.legalHeading;
		}

		return res;
	}

	// Native Road Analysis
	if (nativeNodeId == -1 || static_cast<size_t>(nativeNodeId) >= m_nodes.size()) {
		res.status = ROAD_STATUS_INVALID_NODE;
		return res;
	}

	res.currentNodeId = nativeNodeId;
	const auto& firstNode = m_nodes[nativeNodeId];

	if (firstNode.linkCount > 2) {
		res.status = ROAD_STATUS_ON_INTERSECTION;
	}

	// Find the connecting node closest to current position
	int32_t bestConnectedNode = -1;
	int32_t bestNaviId = -1;
	float bestDistSq = 1e9f;

	for (int k = 0; k < 4; ++k) {
		int32_t targetId = firstNode.links[k].targetNodeId;
		if (targetId != -1 && static_cast<size_t>(targetId) < m_nodes.size()) {
			const auto& candidate = m_nodes[targetId];
			float dx = candidate.pos.x - pos.x;
			float dy = candidate.pos.y - pos.y;
			float dz = candidate.pos.z - pos.z;
			float d2 = dx * dx + dy * dy + dz * dz;

			if (d2 < bestDistSq) {
				bestDistSq = d2;
				bestConnectedNode = targetId;
				bestNaviId = firstNode.links[k].naviId;
			}
		}
	}

	if (bestConnectedNode == -1) {
		res.status = ROAD_STATUS_NO_CONNECTING_NODE;
		return res;
	}

	res.nextNodeId = bestConnectedNode;
	res.naviId = bestNaviId;
	const auto& secondNode = m_nodes[bestConnectedNode];
	res.segmentStart = firstNode.pos;
	res.segmentEnd = secondNode.pos;

	CVector diff = secondNode.pos - firstNode.pos;
	float segLen = std::sqrt(diff.x * diff.x + diff.y * diff.y);
	res.segmentLength = segLen;

	if (segLen > 0.001f) {
		CVector fwd = { diff.x / segLen, diff.y / segLen, diff.z / segLen };
		CVector rt = { fwd.y, -fwd.x, 0.0f };

		float t = ((pos.x - firstNode.pos.x) * fwd.x +
		           (pos.y - firstNode.pos.y) * fwd.y +
		           (pos.z - firstNode.pos.z) * fwd.z) / segLen;
		res.longitudinalProgress = std::clamp(t, 0.0f, 1.0f);

		CVector centerPoint = firstNode.pos + (fwd * (res.longitudinalProgress * segLen));
		CVector toPos = pos - centerPoint;
		float lateralOffset = toPos.x * rt.x + toPos.y * rt.y;

		float roadHeading = std::atan2(diff.y, diff.x) * BT_RAD_TO_DEG - 90.0f;
		res.roadHeading = NormalizeAngle(roadHeading);

		float vehAngle = NormalizeAngle(headingDeg);
		float angleDiff = NormalizeAngleDiff(res.roadHeading - vehAngle);

		uint8_t leftLanes = 1;
		uint8_t rightLanes = 1;
		if (bestNaviId != -1 && static_cast<size_t>(bestNaviId) < m_navis.size()) {
			leftLanes = m_navis[bestNaviId].leftLanes;
			rightLanes = m_navis[bestNaviId].rightLanes;
		}

		res.lane.laneWidth = 3.75f;
		if (lateralOffset >= 0.0f) {
			// Right side
			res.lane.side = ROAD_SIDE_RIGHT;
			res.lane.totalLanesOnSide = rightLanes;
			uint8_t lIdx = static_cast<uint8_t>(lateralOffset / 3.75f);
			res.lane.laneIndex = std::min<uint8_t>(lIdx, std::max<uint8_t>(1, rightLanes) - 1);

			res.lane.lateralOffsetFromCenter = (static_cast<float>(res.lane.laneIndex) + 0.5f) * 3.75f;
			res.lane.currentLateralDeviation = lateralOffset - res.lane.lateralOffsetFromCenter;
			res.lane.laneCenterPos = centerPoint + (rt * res.lane.lateralOffsetFromCenter);
			res.lane.laneForwardTangent = fwd;
			res.lane.legalHeading = res.roadHeading;
			res.lane.isAlignedWithTraffic = (std::abs(angleDiff) <= 60.0f);
			if (res.status != ROAD_STATUS_ON_INTERSECTION) {
				res.status = res.lane.isAlignedWithTraffic ? ROAD_STATUS_ON_RIGHT_SIDE : ROAD_STATUS_ON_WRONG_SIDE;
			}
		} else {
			// Left side
			res.lane.side = ROAD_SIDE_LEFT;
			res.lane.totalLanesOnSide = leftLanes;
			float leftPos = -lateralOffset;
			uint8_t lIdx = static_cast<uint8_t>(leftPos / 3.75f);
			res.lane.laneIndex = std::min<uint8_t>(lIdx, std::max<uint8_t>(1, leftLanes) - 1);

			res.lane.lateralOffsetFromCenter = -(static_cast<float>(res.lane.laneIndex) + 0.5f) * 3.75f;
			res.lane.currentLateralDeviation = leftPos - ((static_cast<float>(res.lane.laneIndex) + 0.5f) * 3.75f);
			res.lane.laneCenterPos = centerPoint + (rt * res.lane.lateralOffsetFromCenter);
			res.lane.laneForwardTangent = -fwd;
			res.lane.legalHeading = NormalizeAngle(res.roadHeading + 180.0f);
			float leftAngleDiff = NormalizeAngleDiff(res.lane.legalHeading - vehAngle);
			res.lane.isAlignedWithTraffic = (std::abs(leftAngleDiff) <= 60.0f);
			if (res.status != ROAD_STATUS_ON_INTERSECTION) {
				res.status = res.lane.isAlignedWithTraffic ? ROAD_STATUS_ON_RIGHT_SIDE : ROAD_STATUS_ON_WRONG_SIDE;
			}
		}

		// Lookahead Steering Target
		float targetT = (res.lane.side == ROAD_SIDE_RIGHT)
			? (res.longitudinalProgress + (lookaheadDist / segLen))
			: (res.longitudinalProgress - (lookaheadDist / segLen));

		if (targetT >= 0.0f && targetT <= 1.0f) {
			res.steering.targetPos = firstNode.pos + (diff * targetT) + (rt * res.lane.lateralOffsetFromCenter);
			res.steering.tangent = res.lane.laneForwardTangent;
			res.steering.targetHeading = res.lane.legalHeading;
		} else {
			res.steering.targetPos = centerPoint + (res.lane.laneForwardTangent * lookaheadDist) + (rt * res.lane.lateralOffsetFromCenter);
			res.steering.tangent = res.lane.laneForwardTangent;
			res.steering.targetHeading = res.lane.legalHeading;
		}
	}

	// Intersection options at next node
	res.turnOptions = GetIntersectionTurnOptions(nativeNodeId, bestConnectedNode);

	return res;
}

TrafficQueryResult RoadsManager::QueryVehicleTraffic(CVehicle* pVehicle, float lookaheadDist) const
{
	if (!pVehicle) {
		TrafficQueryResult emptyRes;
		return emptyRes;
	}
	CVector vehPos = pVehicle->GetPosition();
	float vehHeading = pVehicle->GetHeading() * BT_RAD_TO_DEG;
	return QueryTraffic(vehPos, vehHeading, lookaheadDist);
}

RoadSideResult RoadsManager::CheckSide(const CVector& pos, float headingDegrees) const
{
	RoadSideResult res;
	TrafficQueryResult tRes = QueryTraffic(pos, headingDegrees);

	res.status = tRes.status;
	res.closestNodeId = tRes.currentNodeId;
	res.connectingNodeId = tRes.nextNodeId;
	res.naviId = tRes.naviId;
	res.roadAngle = tRes.roadHeading;
	res.vehicleAngle = NormalizeAngle(headingDegrees);
	res.angleDiff = NormalizeAngleDiff(tRes.roadHeading - res.vehicleAngle);
	res.leftLanes = tRes.lane.totalLanesOnSide;
	res.rightLanes = tRes.lane.totalLanesOnSide;
	res.distanceToRoad = std::abs(tRes.lane.currentLateralDeviation);
	res.isCustomRoad = tRes.isCustomRoad;
	res.customObjectId = tRes.customRoadObjectId;
	res.side = tRes.lane.side;
	res.laneIndex = tRes.lane.laneIndex;

	return res;
}

RoadSideResult RoadsManager::CheckVehicle(CVehicle* pVehicle) const
{
	if (!pVehicle) {
		RoadSideResult emptyRes;
		return emptyRes;
	}

	CVector pos = pVehicle->GetPosition();
	float headingDeg = pVehicle->GetHeading() * BT_RAD_TO_DEG;

	RoadSideResult res = CheckSide(pos, headingDeg);

	// Dynamically integrate vehicle autopilot tracking from GTA SA engine
	const auto& ap = pVehicle->m_autoPilot;
	if (ap.m_nCurrentPathNodeInfo.m_nAreaId >= 0 &&
	    ap.m_nCurrentPathNodeInfo.m_nAreaId < NUM_PATH_MAP_AREAS &&
	    ThePaths.m_pNaviNodes[ap.m_nCurrentPathNodeInfo.m_nAreaId])
	{
		CCarPathLink* pLink = &ThePaths.m_pNaviNodes[ap.m_nCurrentPathNodeInfo.m_nAreaId][ap.m_nCurrentPathNodeInfo.m_nCarPathLinkId];
		if (pLink) {
			if (pLink->m_nNumLeftLanes > 0 || pLink->m_nNumRightLanes > 0) {
				res.leftLanes = pLink->m_nNumLeftLanes;
				res.rightLanes = pLink->m_nNumRightLanes;
				if (ap.m_nCurrentLane >= 0) {
					res.laneIndex = static_cast<uint8_t>(ap.m_nCurrentLane);
				}
			}
		}
	}

	return res;
}

void RoadsManager::Process()
{
	if (!m_bEnabled) return;

	uint32_t currentTick = GetTickCount();

	// Auto-scan world road objects periodically (every 2.5 seconds)
	if (currentTick - m_lastScanTick >= 2500) {
		m_lastScanTick = currentTick;
		AutoScanWorldRoadObjects();
	}

	CPed* pLocalPlayer = FindPlayerPed();
	if (!pLocalPlayer) return;

	if (!pLocalPlayer->bInVehicle) {
		m_lastStatus = ROAD_STATUS_INVALID_NODE;
		return;
	}

	CVehicle* pVehicle = FindPlayerVehicle(-1, false);
	if (!pVehicle) return;

	RoadSideResult res = CheckVehicle(pVehicle);

	// Status transition event dispatch
	if (res.status != m_lastStatus) {
		m_lastStatus = res.status;
		PacketHandler::SendRoadStatusEvent(res.status, res.closestNodeId, res.distanceToRoad, res.roadAngle, res.vehicleAngle);
	}

	// On-screen warning if driving on wrong side
	if (m_bAutoWarn && res.status == ROAD_STATUS_ON_WRONG_SIDE) {
		if (currentTick - m_lastWarnTick >= m_warnIntervalMs) {
			m_lastWarnTick = currentTick;
			CMessages::AddMessageJumpQ(const_cast<char*>("~r~You are on the wrong side of the road!"), 2000, 0, false);
		}
	}
}

void RoadsManager::Configure(bool enabled, bool autoWarn, uint32_t warnIntervalMs)
{
	m_bEnabled = enabled;
	m_bAutoWarn = autoWarn;
	m_warnIntervalMs = (warnIntervalMs > 500) ? warnIntervalMs : 500;
}
