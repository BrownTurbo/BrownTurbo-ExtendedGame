#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <CVector.h>
#include "defs.h"

class CVehicle;
class CEntity;

struct RoadLink {
	int32_t targetNodeId = -1;
	int32_t naviId = -1;
};

struct RoadNavi {
	int32_t from = -1;
	int32_t to = -1;
	uint8_t leftLanes = 0;
	uint8_t rightLanes = 0;
};

struct RoadNode {
	uint32_t id = 0;
	CVector pos = { 0.0f, 0.0f, 0.0f };
	RoadLink links[4];
	uint8_t linkCount = 0;
};

// Custom Road Piece geometry representing a mapped road object or segment
struct CustomRoadSegment {
	uint32_t id = 0;
	uint32_t objectId = 0;       // SA-MP or GTA object ID (0 if virtual)
	uint16_t modelId = 0;
	CVector startPos = { 0.0f, 0.0f, 0.0f };
	CVector endPos = { 0.0f, 0.0f, 0.0f };
	CVector centerPos = { 0.0f, 0.0f, 0.0f };
	CVector surfaceNormal = { 0.0f, 0.0f, 1.0f }; // Incline/pitch/roll normal
	CVector forward = { 0.0f, 1.0f, 0.0f };       // Tangent direction
	CVector right = { 1.0f, 0.0f, 0.0f };         // Normal direction (right side)
	float length = 0.0f;
	float width = 8.0f;
	float laneWidth = 4.0f;
	uint8_t leftLanes = 1;
	uint8_t rightLanes = 1;
	bool isOneWay = false;
	int32_t nextSegmentId = -1;
	int32_t prevSegmentId = -1;
};

// Model geometry template for auto-detecting road objects
struct RoadModelProfile {
	uint16_t modelId = 0;
	float length = 20.0f;
	float width = 8.0f;
	uint8_t leftLanes = 1;
	uint8_t rightLanes = 1;
	int forwardAxis = 1; // 0 = X, 1 = Y (standard), 2 = Z
};

// Dynamic ground/road mesh raycast result
struct RoadRaycastResult {
	bool hit = false;
	CVector surfacePos = { 0.0f, 0.0f, 0.0f };
	CVector surfaceNormal = { 0.0f, 0.0f, 1.0f };
	float inclineDeg = 0.0f;    // Slope angle in degrees from horizontal (0 = flat)
	uint16_t hitModelId = 0;
	uint32_t hitObjectId = 0;
	bool isCustomRoadObject = false;
};

// Detailed lane information for a vehicle or position
struct TrafficLaneInfo {
	int8_t side = ROAD_SIDE_UNKNOWN; // -1: Left side, +1: Right side
	uint8_t laneIndex = 0;           // 0 = innermost (near centerline), 1, 2 = outer
	uint8_t totalLanesOnSide = 0;
	float laneWidth = 3.75f;
	float lateralOffsetFromCenter = 0.0f; // Signed offset from road centerline to lane center
	float currentLateralDeviation = 0.0f; // Distance from vehicle to lane center
	CVector laneCenterPos = { 0.0f, 0.0f, 0.0f };      // 3D position of lane center projected at current progress
	CVector laneForwardTangent = { 0.0f, 0.0f, 0.0f }; // Unit vector of traffic flow direction
	float legalHeading = 0.0f;          // Compass heading (0..360) for legal traffic flow in this lane
	bool isAlignedWithTraffic = true;   // True if vehicle heading matches lane traffic direction
};

// Lookahead waypoint for traffic steering / AI path following
struct TrafficSteeringTarget {
	CVector targetPos = { 0.0f, 0.0f, 0.0f };   // Lookahead 3D position along current lane
	CVector tangent = { 0.0f, 0.0f, 0.0f };     // Tangent vector at lookahead target
	float targetHeading = 0.0f;                 // Compass heading in degrees
	float distanceToTarget = 0.0f;
	float curvature = 0.0f;                     // Rate of curve (for braking/steering angle)
};

// Branching turn options at intersections for traffic AI
struct TrafficTurnOption {
	int32_t targetNodeId = -1;
	int32_t naviId = -1;
	uint8_t maneuver = TRAFFIC_MANEUVER_STRAIGHT; // Straight, Right Turn, Left Turn, U-Turn
	float turnAngle = 0.0f;                       // Angle change in degrees (-180..+180)
	uint8_t targetLaneCount = 0;
};

// Comprehensive traffic query result
struct TrafficQueryResult {
	uint8_t status = ROAD_STATUS_INVALID_NODE;
	int32_t currentNodeId = -1;
	int32_t nextNodeId = -1;
	int32_t naviId = -1;
	bool isCustomRoad = false;
	uint32_t customRoadObjectId = 0;

	// Road Segment geometry
	CVector segmentStart = { 0.0f, 0.0f, 0.0f };
	CVector segmentEnd = { 0.0f, 0.0f, 0.0f };
	float segmentLength = 0.0f;
	float longitudinalProgress = 0.0f; // [0.0..1.0] along segment
	float roadHeading = 0.0f;          // Compass angle of road vector

	// Subsystem results
	TrafficLaneInfo lane;
	TrafficSteeringTarget steering;
	RoadRaycastResult raycast;
	std::vector<TrafficTurnOption> turnOptions;
};

// Legacy/Compatibility Road Side Result
struct RoadSideResult {
	uint8_t status = ROAD_STATUS_INVALID_NODE;
	int32_t closestNodeId = -1;
	int32_t connectingNodeId = -1;
	int32_t naviId = -1;
	float roadAngle = 0.0f;        // GTA facing angle of road vector (0 = North)
	float vehicleAngle = 0.0f;     // GTA facing angle of vehicle (0 = North)
	float angleDiff = 0.0f;        // Difference between road angle and vehicle heading (-180..180)
	float roadToPlayerDiff = 0.0f; // Difference between road vector and road-to-player vector
	uint8_t leftLanes = 0;
	uint8_t rightLanes = 0;
	float distanceToRoad = 0.0f;
	bool isCustomRoad = false;
	uint32_t customObjectId = 0;
	int8_t side = ROAD_SIDE_UNKNOWN;
	uint8_t laneIndex = 0;
};

class RoadsManager {
public:
	static RoadsManager& Instance();

	bool Initialize(const std::string& customDir = "");
	void Shutdown();

	// Spatial lookups for default roads
	int32_t FindClosestNode(const CVector& pos, float maxRadius = 150.0f, float* outDist = nullptr) const;
	const RoadNode* GetNode(uint32_t id) const;
	const RoadNavi* GetNavi(uint32_t id) const;
	size_t GetTotalNodes() const { return m_nodes.size(); }
	size_t GetTotalNavis() const { return m_navis.size(); }
	bool IsLoaded() const { return m_bLoaded; }

	// Custom Roads Subsystem
	bool RegisterCustomRoadObject(uint32_t objectId, uint16_t modelId, const CVector& pos, const CVector& rot,
		uint8_t leftLanes = 1, uint8_t rightLanes = 1, float width = 8.0f);
	bool RegisterCustomRoadSegment(const CVector& startPos, const CVector& endPos,
		uint8_t leftLanes = 1, uint8_t rightLanes = 1, float laneWidth = 4.0f);
	void ClearCustomRoads();
	size_t GetCustomRoadCount() const { return m_customSegments.size(); }
	const CustomRoadSegment* GetCustomRoad(uint32_t id) const;
	int32_t FindClosestCustomRoad(const CVector& pos, float maxRadius = 80.0f, float* outDist = nullptr) const;
	void RegisterRoadModelProfile(uint16_t modelId, float length, float width,
		uint8_t leftLanes = 1, uint8_t rightLanes = 1, int forwardAxis = 1);
	void AutoScanWorldRoadObjects();

	// Dynamic Ground & Road Surface Raycast
	bool RaycastRoadSurface(const CVector& testPos, RoadRaycastResult& outResult) const;

	// Traffic System Queries
	TrafficQueryResult QueryTraffic(const CVector& pos, float headingDeg, float lookaheadDist = 12.0f) const;
	TrafficQueryResult QueryVehicleTraffic(CVehicle* pVehicle, float lookaheadDist = 12.0f) const;
	std::vector<TrafficTurnOption> GetIntersectionTurnOptions(int32_t fromNodeId, int32_t atNodeId) const;
	bool GetLaneWaypoint(int32_t fromNodeId, int32_t toNodeId, int8_t side, uint8_t laneIdx,
		float progress, CVector& outPos, CVector& outTangent) const;

	// Side and direction checking
	RoadSideResult CheckSide(const CVector& pos, float headingDegrees) const;
	RoadSideResult CheckVehicle(CVehicle* pVehicle) const;

	// Per-frame game process
	void Process();

	// Configuration
	void Configure(bool enabled, bool autoWarn, uint32_t warnIntervalMs);
	bool IsEnabled() const { return m_bEnabled; }

private:
	RoadsManager() = default;
	~RoadsManager() = default;

	bool LoadFromNodesDat(const std::string& customDir);
	bool LoadFromEnginePaths();
	void BuildSpatialGrid();
	void InitDefaultModelProfiles();
	void AddCustomSegmentToGrid(uint32_t segIdx);

	std::vector<RoadNode> m_nodes;
	std::vector<RoadNavi> m_navis;

	// Custom Road Storage
	std::vector<CustomRoadSegment> m_customSegments;
	std::unordered_map<uint16_t, RoadModelProfile> m_modelProfiles;
	std::unordered_map<uint32_t, uint32_t> m_objectIdToCustomIdx;

	// Spatial 2D Grid: cell size 100m, map [-3000, 3000]
	static constexpr float GRID_CELL_SIZE = 100.0f;
	static constexpr float GRID_OFFSET = 3000.0f;
	static constexpr int32_t GRID_DIM = 60; // 6000 / 100
	std::vector<uint32_t> m_grid[GRID_DIM][GRID_DIM];
	std::vector<uint32_t> m_customGrid[GRID_DIM][GRID_DIM];

	bool m_bLoaded = false;
	bool m_bEnabled = true;
	bool m_bAutoWarn = true;
	uint32_t m_warnIntervalMs = 3000;
	uint32_t m_lastWarnTick = 0;
	uint32_t m_lastScanTick = 0;
	uint8_t m_lastStatus = ROAD_STATUS_INVALID_NODE;
};
