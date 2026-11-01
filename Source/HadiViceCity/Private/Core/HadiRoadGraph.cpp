// HADI: VICE CITY - Simulation Core
#include "Core/HadiRoadGraph.h"

#include <algorithm>
#include <limits>
#include <queue>

namespace hadi
{
int RoadGraph::AddNode(Vec2 pos)
{
	nodes.push_back(RoadNode{pos});
	adjacency.emplace_back();
	return static_cast<int>(nodes.size()) - 1;
}

int RoadGraph::FindOrAddNode(Vec2 pos, float tolerance)
{
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (Distance(nodes[i].pos, pos) <= tolerance)
		{
			return static_cast<int>(i);
		}
	}
	return AddNode(pos);
}

void RoadGraph::AddEdge(int a, int b)
{
	if (a == b || a < 0 || b < 0 || a >= NumNodes() || b >= NumNodes())
	{
		return;
	}
	for (int edgeIndex : adjacency[static_cast<size_t>(a)])
	{
		const RoadEdge& existing = edges[static_cast<size_t>(edgeIndex)];
		if ((existing.a == a && existing.b == b) || (existing.a == b && existing.b == a))
		{
			return;
		}
	}
	RoadEdge edge;
	edge.a = a;
	edge.b = b;
	edge.length = Distance(nodes[static_cast<size_t>(a)].pos, nodes[static_cast<size_t>(b)].pos);
	edges.push_back(edge);
	const int newIndex = static_cast<int>(edges.size()) - 1;
	adjacency[static_cast<size_t>(a)].push_back(newIndex);
	adjacency[static_cast<size_t>(b)].push_back(newIndex);
}

int RoadGraph::FindNearestNode(Vec2 pos) const
{
	int best = -1;
	float bestDist = std::numeric_limits<float>::max();
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		const float d = Distance(nodes[i].pos, pos);
		if (d < bestDist)
		{
			bestDist = d;
			best = static_cast<int>(i);
		}
	}
	return best;
}

bool RoadGraph::FindPath(int start, int goal, std::vector<int>& outNodes) const
{
	outNodes.clear();
	if (start < 0 || goal < 0 || start >= NumNodes() || goal >= NumNodes())
	{
		return false;
	}
	if (start == goal)
	{
		outNodes.push_back(start);
		return true;
	}

	const size_t count = nodes.size();
	std::vector<float> gScore(count, std::numeric_limits<float>::max());
	std::vector<int> cameFrom(count, -1);
	std::vector<bool> closed(count, false);

	using QueueItem = std::pair<float, int>; // (fScore, node)
	std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<QueueItem>> open;

	const Vec2 goalPos = nodes[static_cast<size_t>(goal)].pos;
	gScore[static_cast<size_t>(start)] = 0.0f;
	open.push(QueueItem(Distance(nodes[static_cast<size_t>(start)].pos, goalPos), start));

	while (!open.empty())
	{
		const int current = open.top().second;
		open.pop();
		if (closed[static_cast<size_t>(current)])
		{
			continue;
		}
		if (current == goal)
		{
			for (int n = goal; n != -1; n = cameFrom[static_cast<size_t>(n)])
			{
				outNodes.push_back(n);
			}
			std::reverse(outNodes.begin(), outNodes.end());
			return true;
		}
		closed[static_cast<size_t>(current)] = true;

		for (int edgeIndex : adjacency[static_cast<size_t>(current)])
		{
			const RoadEdge& edge = edges[static_cast<size_t>(edgeIndex)];
			const int neighbour = edge.a == current ? edge.b : edge.a;
			if (closed[static_cast<size_t>(neighbour)])
			{
				continue;
			}
			const float tentative = gScore[static_cast<size_t>(current)] + edge.length;
			if (tentative < gScore[static_cast<size_t>(neighbour)])
			{
				gScore[static_cast<size_t>(neighbour)] = tentative;
				cameFrom[static_cast<size_t>(neighbour)] = current;
				const float h = Distance(nodes[static_cast<size_t>(neighbour)].pos, goalPos);
				open.push(QueueItem(tentative + h, neighbour));
			}
		}
	}
	return false;
}

bool RoadGraph::BuildRoute(Vec2 from, Vec2 to, std::vector<Vec2>& outPoints) const
{
	outPoints.clear();
	const int startNode = FindNearestNode(from);
	const int goalNode = FindNearestNode(to);
	std::vector<int> path;
	if (!FindPath(startNode, goalNode, path))
	{
		return false;
	}
	outPoints.push_back(from);
	for (int n : path)
	{
		outPoints.push_back(nodes[static_cast<size_t>(n)].pos);
	}
	outPoints.push_back(to);
	return true;
}

bool RoadGraph::IsConnected() const
{
	if (nodes.empty())
	{
		return true;
	}
	std::vector<bool> seen(nodes.size(), false);
	std::vector<int> stack;
	stack.push_back(0);
	seen[0] = true;
	size_t visited = 0;
	while (!stack.empty())
	{
		const int n = stack.back();
		stack.pop_back();
		++visited;
		for (int edgeIndex : adjacency[static_cast<size_t>(n)])
		{
			const RoadEdge& edge = edges[static_cast<size_t>(edgeIndex)];
			const int other = edge.a == n ? edge.b : edge.a;
			if (!seen[static_cast<size_t>(other)])
			{
				seen[static_cast<size_t>(other)] = true;
				stack.push_back(other);
			}
		}
	}
	return visited == nodes.size();
}
} // namespace hadi
