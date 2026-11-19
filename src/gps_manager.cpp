#include "gps_manager.h"
#include "roads_manager.h"
#include "packet_handler.h"
#include "utils.h"

#include <CPathFind.h>
#include <CPathNode.h>
#include <CNodeAddress.h>
#include <CRadar.h>
#include <CSprite2d.h>
#include <CHud.h>
#include <CMessages.h>
#include <CPed.h>
#include <CVehicle.h>
#include <common.h>

#include <algorithm>
#include <cmath>

GPSManager& GPSManager::Instance()
{
	static GPSManager s_instance;
	return s_instance;
}

bool GPSManager::Initialize()
{
	if (m_bInitialized) return true;

	ClientLog(LogLevel::Info, "GPSManager: Initialized with CPathFind engine.");
	m_bInitialized = true;
	return true;
}

void GPSManager::Shutdown()
{
	CancelDestination();
	m_bInitialized = false;
	ClientLog(LogLevel::Info, "GPSManager: Shutdown complete.");
}

int32_t GPSManager::FindClosestGpsNode(const CVector& pos, float maxRadius, CVector* outNodePos) const
{
	// 1. Check RoadsManager spatial grid first
	float roadDist = maxRadius;
	int32_t roadNode = RoadsManager::Instance().FindClosestNode(pos, maxRadius, &roadDist);
	if (roadNode != -1) {
		if (outNodePos) {
			const RoadNode* rn = RoadsManager::Instance().GetNode(roadNode);
			if (rn) *outNodePos = rn->pos;
		}
		return roadNode;
	}

	// 2. Query ThePaths directly across map areas around pos
	int centerAreaX = std::clamp(static_cast<int>((pos.x + 3000.0f) / 750.0f), 0, 7);
	int centerAreaY = std::clamp(static_cast<int>((pos.y + 3000.0f) / 750.0f), 0, 7);

	float bestDistSq = maxRadius * maxRadius;
	CPathNode* bestNode = nullptr;
	int32_t bestNodeIndex = -1;

	// Check 3x3 surrounding areas in ThePaths
	for (int dx = -1; dx <= 1; ++dx) {
		int ax = centerAreaX + dx;
		if (ax < 0 || ax >= 8) continue;

		for (int dy = -1; dy <= 1; ++dy) {
			int ay = centerAreaY + dy;
			if (ay < 0 || ay >= 8) continue;

			int areaId = ax + ay * 8;
			if (!ThePaths.m_pPathNodes[areaId]) continue;

			unsigned int numVehNodes = ThePaths.m_dwNumVehicleNodes[areaId];
			for (unsigned int n = 0; n < numVehNodes; ++n) {
				CPathNode* pNode = &ThePaths.m_pPathNodes[areaId][n];
				if (!pNode || pNode->WaterNode) continue;

				CVector nPos = pNode->GetNodeCoors();
				float dX = nPos.x - pos.x;
				float dY = nPos.y - pos.y;
				float dZ = nPos.z - pos.z;
				float dSq = dX * dX + dY * dY + dZ * dZ;

				if (dSq < bestDistSq) {
					bestDistSq = dSq;
					bestNode = pNode;
					bestNodeIndex = static_cast<int32_t>(n);
				}
			}
		}
	}

	if (bestNode && outNodePos) {
		*outNodePos = bestNode->GetNodeCoors();
	}

	return bestNodeIndex;
}

bool GPSManager::CalculateRoute(const CVector& startPos, const CVector& targetPos, GPSRoute& outRoute) const
{
	outRoute.waypoints.clear();
	outRoute.nodeIds.clear();
	outRoute.totalDistance = 0.0f;
	outRoute.currentWaypointIndex = 0;
	outRoute.active = false;

	// Tell GTA SA path streaming to ensure path nodes are available at endpoints
	ThePaths.SetPathsNeededAtPosition(startPos);
	ThePaths.SetPathsNeededAtPosition(targetPos);
	ThePaths.UpdateStreaming(true);

	static constexpr int MAX_PATH_NODES = 512;
	CNodeAddress resultNodes[MAX_PATH_NODES];
	short nodesCount = 0;
	float routeDistance = 0.0f;

	// Call native GTA:SA pathfinding engine (0x4515D0)
	ThePaths.DoPathSearch(
		0,                  // 0 = Car path
		startPos,           // Origin position
		CNodeAddress(),     // Origin address (empty to auto-find closest)
		targetPos,          // Destination position
		resultNodes,        // Result buffer
		&nodesCount,        // Result nodes count
		MAX_PATH_NODES,     // Buffer capacity
		&routeDistance,     // Total route distance output
		999999.0f,          // Max search distance
		nullptr,            // Target address
		999999.0f,          // Max limit
		false,              // One side only
		CNodeAddress(),     // Forbidden node
		false,              // Include nodes without links
		false               // Water path
	);

	if (nodesCount <= 0) {
		ClientLog(LogLevel::Warning, "GPSManager: Native ThePaths.DoPathSearch found no path between coordinates.");
		return false;
	}

	outRoute.totalDistance = routeDistance;
	outRoute.waypoints.reserve(nodesCount + 1);
	outRoute.nodeIds.reserve(nodesCount);

	for (int i = 0; i < nodesCount; ++i) {
		outRoute.nodeIds.push_back(static_cast<uint32_t>(resultNodes[i].Index));
		CPathNode* pNode = ThePaths.GetPathNode(resultNodes[i]);
		if (pNode) {
			outRoute.waypoints.push_back(pNode->GetNodeCoors());
		}
	}

	// Terminate route at exact destination coordinate
	outRoute.waypoints.push_back(targetPos);
	outRoute.active = true;

	ClientLog(LogLevel::Info, std::format("GPSManager: Dynamic route calculated via ThePaths. {} waypoints, total distance: {:.1f}m",
		outRoute.waypoints.size(), outRoute.totalDistance));
	return true;
}

bool GPSManager::SetDestination(const CVector& targetPos, uint8_t flags)
{
	CPed* pPlayer = FindPlayerPed();
	if (!pPlayer) return false;

	CVector startPos = pPlayer->GetPosition();
	CVehicle* pVehicle = FindPlayerVehicle(-1, false);
	if (pVehicle) {
		startPos = pVehicle->GetPosition();
	}

	m_destination = targetPos;
	m_navFlags = flags;

	if (!CalculateRoute(startPos, targetPos, m_activeRoute)) {
		PacketHandler::SendGPSEvent(GPS_EVENT_PATH_NOT_FOUND, 0.0f);
		return false;
	}

	// 3D Destination Radar Blip
	if (m_navFlags & GPS_NAV_FLAG_WORLD_MARKER) {
		if (m_blipHandle != -1) {
			CRadar::ClearBlip(m_blipHandle);
			m_blipHandle = -1;
		}
		// Set coordinate blip (YELLOW / DESTINATION)
		m_blipHandle = CRadar::SetCoordBlip(BLIP_COORD, targetPos, 0, BLIP_DISPLAY_BOTH, nullptr);
		if (m_blipHandle != -1) {
			CRadar::ChangeBlipColour(m_blipHandle, 0xFFFF00FF);
		}
	}

	ClientLog(LogLevel::Info, std::format("GPSManager: Destination set to ({:.1f}, {:.1f}, {:.1f})",
		targetPos.x, targetPos.y, targetPos.z));
	return true;
}

void GPSManager::CancelDestination()
{
	if (m_blipHandle != -1) {
		CRadar::ClearBlip(m_blipHandle);
		m_blipHandle = -1;
	}

	m_activeRoute.active = false;
	m_activeRoute.waypoints.clear();
	m_activeRoute.totalDistance = 0.0f;
	m_activeRoute.currentWaypointIndex = 0;
	ClientLog(LogLevel::Info, "GPSManager: Destination cancelled.");
}

float GPSManager::GetRemainingDistance() const
{
	if (!m_activeRoute.active || m_activeRoute.waypoints.empty()) return 0.0f;

	CPed* pPlayer = FindPlayerPed();
	if (!pPlayer) return 0.0f;

	CVector playerPos = pPlayer->GetPosition();
	size_t currentIdx = m_activeRoute.currentWaypointIndex;
	if (currentIdx >= m_activeRoute.waypoints.size()) return 0.0f;

	float dist = (playerPos - m_activeRoute.waypoints[currentIdx]).Magnitude();
	for (size_t i = currentIdx; i + 1 < m_activeRoute.waypoints.size(); ++i) {
		dist += (m_activeRoute.waypoints[i + 1] - m_activeRoute.waypoints[i]).Magnitude();
	}
	return dist;
}

void GPSManager::Process()
{
	if (!m_activeRoute.active || m_activeRoute.waypoints.empty()) return;

	CPed* pPlayer = FindPlayerPed();
	if (!pPlayer) return;

	CVector playerPos = pPlayer->GetPosition();
	CVehicle* pVehicle = FindPlayerVehicle(-1, false);
	if (pVehicle) {
		playerPos = pVehicle->GetPosition();
	}

	// 1. Check destination arrival (within 12 meters of target)
	float distToDestination = (playerPos - m_destination).Magnitude();
	if (distToDestination <= 12.0f) {
		ClientLog(LogLevel::Info, "GPSManager: Destination reached!");
		CMessages::AddMessageJumpQ(const_cast<char*>("~g~You have reached your destination!"), 3500, 0, false);
		PacketHandler::SendGPSEvent(GPS_EVENT_DESTINATION_REACHED, 0.0f);
		CancelDestination();
		return;
	}

	// 2. Advance waypoint progression as player approaches next node
	size_t currentIdx = m_activeRoute.currentWaypointIndex;
	if (currentIdx < m_activeRoute.waypoints.size()) {
		float distToCurrentWp = (playerPos - m_activeRoute.waypoints[currentIdx]).Magnitude();
		if (distToCurrentWp < 15.0f && currentIdx + 1 < m_activeRoute.waypoints.size()) {
			m_activeRoute.currentWaypointIndex++;
		}
	}

	// 3. Auto-reroute if strayed more than 80m off the upcoming waypoint
	if (m_navFlags & GPS_NAV_FLAG_AUTO_REROUTE) {
		uint32_t currentTick = GetTickCount();
		if (currentTick - m_lastRerouteTick >= 3000) {
			if (currentIdx < m_activeRoute.waypoints.size()) {
				float distToCurrentWp = (playerPos - m_activeRoute.waypoints[currentIdx]).Magnitude();
				if (distToCurrentWp > 80.0f) {
					m_lastRerouteTick = currentTick;
					ClientLog(LogLevel::Info, "GPSManager: Player drifted off route; recalculating dynamically via ThePaths...");
					CalculateRoute(playerPos, m_destination, m_activeRoute);
					PacketHandler::SendGPSEvent(GPS_EVENT_REROUTED, GetRemainingDistance());
				}
			}
		}
	}
}

void GPSManager::RenderRadarRoute()
{
	if (!m_activeRoute.active || m_activeRoute.waypoints.size() < 2) return;
	if (!(m_navFlags & GPS_NAV_FLAG_RADAR_LINE)) return;

	size_t startIdx = m_activeRoute.currentWaypointIndex;
	if (startIdx >= m_activeRoute.waypoints.size()) return;

	CRGBA lineColor(255, 60, 60, 220); // Bright GPS Red/Coral line
	float lineWidth = 4.0f;

	for (size_t i = startIdx; i + 1 < m_activeRoute.waypoints.size(); ++i) {
		const auto& p1 = m_activeRoute.waypoints[i];
		const auto& p2 = m_activeRoute.waypoints[i + 1];

		CVector2D radar1, radar2;
		CRadar::TransformRealWorldPointToRadarSpace(radar1, CVector2D(p1.x, p1.y));
		CRadar::TransformRealWorldPointToRadarSpace(radar2, CVector2D(p2.x, p2.y));

		CVector2D screen1, screen2;
		CRadar::TransformRadarPointToScreenSpace(screen1, radar1);
		CRadar::TransformRadarPointToScreenSpace(screen2, radar2);

		// Compute perpendicular offset for line width
		CVector2D dir = screen2 - screen1;
		float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
		if (len < 0.001f) continue;

		CVector2D norm = { -dir.y / len * (lineWidth * 0.5f), dir.x / len * (lineWidth * 0.5f) };

		float x1 = screen1.x - norm.x;
		float y1 = screen1.y - norm.y;
		float x2 = screen2.x - norm.x;
		float y2 = screen2.y - norm.y;
		float x3 = screen2.x + norm.x;
		float y3 = screen2.y + norm.y;
		float x4 = screen1.x + norm.x;
		float y4 = screen1.y + norm.y;

		CSprite2d::Draw2DPolygon(x1, y1, x2, y2, x3, y3, x4, y4, lineColor);
	}
}
