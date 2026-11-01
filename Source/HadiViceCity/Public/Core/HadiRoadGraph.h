// HADI: VICE CITY - Simulation Core
// Road network graph used by GPS navigation (and later by traffic AI in Phase 6).
#pragma once

#include "Core/HadiMath.h"

#include <vector>

namespace hadi
{
struct RoadNode
{
	Vec2 pos;
};

struct RoadEdge
{
	int a = 0;
	int b = 0;
	float length = 0.0f;
};

class RoadGraph
{
public:
	int AddNode(Vec2 pos);
	// Returns the existing node within tolerance or creates a new one.
	int FindOrAddNode(Vec2 pos, float tolerance);
	void AddEdge(int a, int b);

	int NumNodes() const { return static_cast<int>(nodes.size()); }
	const RoadNode& Node(int index) const { return nodes[static_cast<size_t>(index)]; }
	const std::vector<RoadEdge>& Edges() const { return edges; }

	int FindNearestNode(Vec2 pos) const;
	// A* shortest path between two nodes. Returns false when unreachable.
	bool FindPath(int start, int goal, std::vector<int>& outNodes) const;
	// Full GPS route as a polyline: from -> road network -> to.
	bool BuildRoute(Vec2 from, Vec2 to, std::vector<Vec2>& outPoints) const;
	// True when every node can reach every other node.
	bool IsConnected() const;

private:
	std::vector<RoadNode> nodes;
	std::vector<RoadEdge> edges;
	std::vector<std::vector<int>> adjacency; // edge indices per node
};
} // namespace hadi
