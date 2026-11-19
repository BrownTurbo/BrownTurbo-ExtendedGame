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
#include <CMenuManager.h>
#include <RenderWare.h>
#include <common.h>

#include <algorithm>
#include <cmath>
#include <format>

using RadarOverlay_t = void(__cdecl*)(bool inMenu);
static RadarOverlay_t s_origRadarOverlay = nullptr;
static RadarOverlay_t s_origMenuMapOverlay = nullptr;

static void __cdecl Hooked_RadarOverlay(bool inMenu)
{
	if (s_origRadarOverlay) {
		s_origRadarOverlay(inMenu);
	}
	GPSManager::Instance().RenderRadarRoute(inMenu);
}

static void __cdecl Hooked_MenuMapOverlay(bool inMenu)
{
	if (s_origMenuMapOverlay) {
		s_origMenuMapOverlay(inMenu);
	}
	GPSManager::Instance().RenderRadarRoute(inMenu);
}

GPSManager& GPSManager::Instance()
{
	static GPSManager s_instance;
	return s_instance;
}

bool GPSManager::Initialize()
{
	if (m_bInitialized) return true;

	InstallRadarHooks();
	ClientLog(LogLevel::Info, "GPSManager: Initialized with CPathFind engine and radar hooks.");
	m_bInitialized = true;
	return true;
}

void GPSManager::InstallRadarHooks()
{
	// 1. Hook 0x5869BF (HUD minimap gang overlay call inside CRadar::DrawRadarMap)
	uint8_t* pRadarCall = reinterpret_cast<uint8_t*>(0x5869BF);
	if (*pRadarCall == 0xE8) {
		int32_t curRel = *reinterpret_cast<int32_t*>(pRadarCall + 1);
		uintptr_t curDest = reinterpret_cast<uintptr_t>(pRadarCall + 5 + curRel);
		if (curDest != reinterpret_cast<uintptr_t>(&Hooked_RadarOverlay)) {
			s_origRadarOverlay = reinterpret_cast<RadarOverlay_t>(curDest);
			DWORD oldProtect = 0;
			if (VirtualProtect(pRadarCall, 5, PAGE_EXECUTE_READWRITE, &oldProtect)) {
				int32_t newRel = static_cast<int32_t>(reinterpret_cast<uintptr_t>(&Hooked_RadarOverlay) - (reinterpret_cast<uintptr_t>(pRadarCall) + 5));
				*reinterpret_cast<int32_t*>(pRadarCall + 1) = newRel;
				VirtualProtect(pRadarCall, 5, oldProtect, &oldProtect);
				ClientLog(LogLevel::Info, std::format("GPSManager: Installed RadarOverlay hook at 0x5869BF (target=0x{:X})", curDest));
			}
		}
	}

	// 2. Hook 0x5759E4 (Pause menu map gang overlay call inside CMenuManager::PrintMap)
	uint8_t* pMenuCall = reinterpret_cast<uint8_t*>(0x5759E4);
	if (*pMenuCall == 0xE8) {
		int32_t curRel = *reinterpret_cast<int32_t*>(pMenuCall + 1);
		uintptr_t curDest = reinterpret_cast<uintptr_t>(pMenuCall + 5 + curRel);
		if (curDest != reinterpret_cast<uintptr_t>(&Hooked_MenuMapOverlay)) {
			s_origMenuMapOverlay = reinterpret_cast<RadarOverlay_t>(curDest);
			DWORD oldProtect = 0;
			if (VirtualProtect(pMenuCall, 5, PAGE_EXECUTE_READWRITE, &oldProtect)) {
				int32_t newRel = static_cast<int32_t>(reinterpret_cast<uintptr_t>(&Hooked_MenuMapOverlay) - (reinterpret_cast<uintptr_t>(pMenuCall) + 5));
				*reinterpret_cast<int32_t*>(pMenuCall + 1) = newRel;
				VirtualProtect(pMenuCall, 5, oldProtect, &oldProtect);
				ClientLog(LogLevel::Info, std::format("GPSManager: Installed MenuMapOverlay hook at 0x5759E4 (target=0x{:X})", curDest));
			}
		}
	}
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
	// Periodically verify hooks remain installed (in case SA-MP or another plugin re-patched)
	static uint32_t s_lastHookCheckTick = 0;
	uint32_t currentTick = GetTickCount();
	if (currentTick - s_lastHookCheckTick >= 3000) {
		s_lastHookCheckTick = currentTick;
		InstallRadarHooks();
	}

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

void GPSManager::RenderRadarRoute(bool inMenu)
{
	if (!m_activeRoute.active || m_activeRoute.waypoints.empty()) return;
	if (!(m_navFlags & GPS_NAV_FLAG_RADAR_LINE)) return;

	CPed* pPlayer = FindPlayerPed(-1);
	if (!pPlayer) return;

	bool bDrawingMenuMap = inMenu || FrontEndMenuManager.m_bDrawRadarOrMap;
	if (!bDrawingMenuMap && pPlayer->m_nAreaCode != 0) return;

	CVector playerPos = pPlayer->GetPosition();
	CVehicle* pVehicle = FindPlayerVehicle(-1, false);
	if (pVehicle) {
		playerPos = pVehicle->GetPosition();
	}

	size_t startIdx = m_activeRoute.currentWaypointIndex;
	if (startIdx >= m_activeRoute.waypoints.size()) return;

	// Build sequence of points starting directly from current player/vehicle 2D position
	std::vector<CVector2D> points;
	points.reserve(m_activeRoute.waypoints.size() - startIdx + 2);
	points.push_back(CVector2D(playerPos.x, playerPos.y));

	for (size_t i = startIdx; i < m_activeRoute.waypoints.size(); ++i) {
		points.push_back(CVector2D(m_activeRoute.waypoints[i].x, m_activeRoute.waypoints[i].y));
	}

	CVector2D dest2D(m_destination.x, m_destination.y);
	if ((points.back() - dest2D).Magnitude() > 3.0f) {
		points.push_back(dest2D);
	}

	if (points.size() < 2) return;

	// Configure RenderWare 2D render states
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)NULL);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);

	struct RenderSegment {
		CVector2D a;
		CVector2D b;
		CVector2D norm;
	};
	std::vector<RenderSegment> segments;
	segments.reserve(points.size());

	for (size_t i = 0; i + 1 < points.size(); ++i) {
		const auto& ptA = points[i];
		const auto& ptB = points[i + 1];

		if (bDrawingMenuMap) {
			// Pause Menu Map: full-screen projection without circular clipping
			CVector2D rA, rB;
			CRadar::TransformRealWorldPointToRadarSpace(rA, ptA);
			CRadar::TransformRealWorldPointToRadarSpace(rB, ptB);

			CVector2D scA, scB;
			CRadar::TransformRadarPointToScreenSpace(scA, rA);
			CRadar::TransformRadarPointToScreenSpace(scB, rB);

			CVector2D dir = scB - scA;
			float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
			if (len < 0.5f) continue;

			CVector2D norm = { -dir.y / len, dir.x / len };
			segments.push_back({ scA, scB, norm });
		} else {
			// HUD Minimap: radar is circular (radius = 1.0f in radar space)
			CVector2D rA, rB;
			CRadar::TransformRealWorldPointToRadarSpace(rA, ptA);
			CRadar::TransformRealWorldPointToRadarSpace(rB, ptB);

			CVector2D d = rB - rA;
			float A_sq = d.x * d.x + d.y * d.y;
			if (A_sq < 0.000001f) continue;

			// Line-circle intersection against unit circle R = 1.0f
			float B_coeff = 2.0f * (rA.x * d.x + rA.y * d.y);
			float C_coeff = (rA.x * rA.x + rA.y * rA.y) - 1.0f;

			float t0 = 0.0f;
			float t1 = 1.0f;

			float discr = B_coeff * B_coeff - 4.0f * A_sq * C_coeff;
			if (discr < 0.0f) {
				if (C_coeff > 0.0f) continue; // Completely outside circle
			} else {
				float sqrtDiscr = std::sqrt(discr);
				float tEntry = (-B_coeff - sqrtDiscr) / (2.0f * A_sq);
				float tExit  = (-B_coeff + sqrtDiscr) / (2.0f * A_sq);

				t0 = (std::max)(0.0f, tEntry);
				t1 = (std::min)(1.0f, tExit);
				if (t0 >= t1) continue;
			}

			CVector2D clipped_rA = rA + d * t0;
			CVector2D clipped_rB = rA + d * t1;

			CVector2D scA, scB;
			CRadar::TransformRadarPointToScreenSpace(scA, clipped_rA);
			CRadar::TransformRadarPointToScreenSpace(scB, clipped_rB);

			CVector2D dir = scB - scA;
			float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
			if (len < 0.5f) continue;

			CVector2D norm = { -dir.y / len, dir.x / len };
			segments.push_back({ scA, scB, norm });
		}
	}

	if (segments.empty()) return;

	// Pass 1: Dark outline for contrast across all radar textures
	const float outlineHalfWidth = 6.0f * 0.5f;
	const CRGBA outlineColor(10, 10, 10, 200);
	for (const auto& seg : segments) {
		CVector2D n = seg.norm * outlineHalfWidth;
		CSprite2d::Draw2DPolygon(
			seg.a.x - n.x, seg.a.y - n.y,
			seg.b.x - n.x, seg.b.y - n.y,
			seg.b.x + n.x, seg.b.y + n.y,
			seg.a.x + n.x, seg.a.y + n.y,
			outlineColor
		);
	}

	// Pass 2: High-visibility GPS Coral-Red core line
	const float coreHalfWidth = 4.0f * 0.5f;
	const CRGBA coreColor(255, 60, 60, 235);
	for (const auto& seg : segments) {
		CVector2D n = seg.norm * coreHalfWidth;
		CSprite2d::Draw2DPolygon(
			seg.a.x - n.x, seg.a.y - n.y,
			seg.b.x - n.x, seg.b.y - n.y,
			seg.b.x + n.x, seg.b.y + n.y,
			seg.a.x + n.x, seg.a.y + n.y,
			coreColor
		);
	}

	// Pass 3: Draw joint caps to connect segment corners seamlessly
	for (const auto& seg : segments) {
		CSprite2d::DrawRect(CRect(seg.a.x - 2.0f, seg.a.y - 2.0f, seg.a.x + 2.0f, seg.a.y + 2.0f), coreColor);
	}
	if (!segments.empty()) {
		const auto& endPt = segments.back().b;
		CSprite2d::DrawRect(CRect(endPt.x - 2.0f, endPt.y - 2.0f, endPt.x + 2.0f, endPt.y + 2.0f), coreColor);
	}
}
