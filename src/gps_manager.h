#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <CVector.h>
#include "defs.h"

struct GPSRoute {
	std::vector<CVector> waypoints;
	std::vector<uint32_t> nodeIds;
	float totalDistance = 0.0f;
	size_t currentWaypointIndex = 0;
	bool active = false;
};

enum GPSNavFlags : uint8_t {
	GPS_NAV_FLAG_RADAR_LINE   = (1 << 0), // Render route line on radar
	GPS_NAV_FLAG_WORLD_MARKER = (1 << 1), // Place 3D destination blip
	GPS_NAV_FLAG_AUTO_REROUTE = (1 << 2), // Auto reroute when off-path
	GPS_NAV_FLAG_ALL          = 0x07
};

class GPSManager {
public:
	static GPSManager& Instance();

	bool Initialize();
	void Shutdown();

	// Dynamic Pathfinding & Spatial Search via plugin-sdk's ThePaths (CPathFind)
	int32_t FindClosestGpsNode(const CVector& pos, float maxRadius = 300.0f, CVector* outNodePos = nullptr) const;
	bool CalculateRoute(const CVector& startPos, const CVector& targetPos, GPSRoute& outRoute) const;

	// Navigation controls
	bool SetDestination(const CVector& targetPos, uint8_t flags = GPS_NAV_FLAG_ALL);
	void CancelDestination();
	bool IsNavigating() const { return m_activeRoute.active; }

	const GPSRoute& GetActiveRoute() const { return m_activeRoute; }
	CVector GetDestination() const { return m_destination; }
	float GetRemainingDistance() const;

	// Per-frame game process & rendering
	void Process();
	void RenderRadarRoute(bool inMenu = false);
	void InstallRadarHooks();

private:
	GPSManager() = default;
	~GPSManager() = default;

	bool m_bInitialized = false;
	GPSRoute m_activeRoute;
	CVector m_destination = { 0.0f, 0.0f, 0.0f };
	uint8_t m_navFlags = GPS_NAV_FLAG_ALL;
	int32_t m_blipHandle = -1;
	uint32_t m_lastRerouteTick = 0;
};
