#include "pathfind.h"

#include <queue>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <sstream>
#include <cmath>
#include "mapnode.h"
#include "nodedef.h"

// Hash and equality for v3s16
struct v3s16_hash {
	std::size_t operator()(const v3s16& v) const {
		return std::hash<int>()(v.X) ^ (std::hash<int>()(v.Y) << 1) ^ (std::hash<int>()(v.Z) << 2);
	}
};

struct v3s16_equal {
	bool operator()(const v3s16& a, const v3s16& b) const {
		return a.X == b.X && a.Y == b.Y && a.Z == b.Z;
	}
};

static v3f to_v3f(const v3s16 &pos) {
	return v3f(pos.X, pos.Y, pos.Z);
}

static v3s16 to_v3s16(const v3f &pos) {
	return v3s16(static_cast<s16>(std::round(pos.X)), static_cast<s16>(std::round(pos.Y)), static_cast<s16>(std::round(pos.Z)));
}

// Caches
static thread_local std::unordered_map<v3s16, s32, v3s16_hash, v3s16_equal> node_cost_cache;
static std::unordered_map<std::string, double> dig_time_cache;

static s32 get_g_cost(const v3f &a, const v3f &b) {
	return static_cast<s32>((a - b).getLength() * 10);
}

double check_tool_dig_time(const ToolCapabilities &toolcaps, const ItemGroupList &node_groups, double old_best_time) {
	double best_time = old_best_time;

	for (const auto &groupcap_pair : toolcaps.groupcaps) {
		const std::string &group = groupcap_pair.first;
		const ToolGroupCap &groupcap = groupcap_pair.second;

		auto it = node_groups.find(group);
		if (it != node_groups.end()) {
			int level = it->second;
			auto level_it = std::find_if(groupcap.times.begin(), groupcap.times.end(),
				[level](const std::pair<int, float> &p) { return p.first == level; });

			if (level_it != groupcap.times.end() && level_it->second < best_time)
				best_time = level_it->second;
		}
	}

	return best_time;
}

double get_best_dig_time(Client *client, const ItemGroupList &groups) {
	std::ostringstream oss;
	for (const auto &g : groups)
		oss << g.first << ":" << g.second << ",";
	std::string key = oss.str();

	auto it = dig_time_cache.find(key);
	if (it != dig_time_cache.end())
		return it->second;

	double best_time = std::numeric_limits<double>::infinity();

	InventoryLocation loc;
	loc.setCurrentPlayer();
	Inventory *inv = client->getInventory(loc);
	if (!inv) return best_time;

	InventoryList *main = inv->getList("main");
	if (!main) return best_time;

	for (u32 i = 0; i < main->getSize(); i++) {
		const ItemStack &stack = main->getItem(i);
		if (stack.empty()) continue;

		const ToolCapabilities &toolcaps = stack.getToolCapabilities(client->idef());
		best_time = check_tool_dig_time(toolcaps, groups, best_time);
	}

	dig_time_cache[key] = best_time;
	return best_time;
}

static s32 get_node_cost(const v3f &pos, Client *client, const NodeDefManager *ndef) {
	v3s16 p = to_v3s16(pos);
	auto it = node_cost_cache.find(p);
	if (it != node_cost_cache.end())
		return it->second;

	bool pos_ok;
	MapNode node = client->CSMGetNode(p, &pos_ok);
	const ContentFeatures &node_def = ndef->get(node);

	s32 cost;
	if (!pos_ok) {
		cost = std::numeric_limits<s32>::max();
	} else if (node_def.name == "air") {
		cost = 0;
	} else {
		cost = static_cast<s32>(get_best_dig_time(client, node_def.groups) * 500);
	}

	node_cost_cache[p] = cost;
	return cost;
}

static s32 get_a_cost(const v3f &pos, const v3f &from_pos, Client *client, const NodeDefManager *ndef) {
	s32 cost = 0;

	v3s16 from = to_v3s16(from_pos);
	v3s16 to = to_v3s16(pos);
	v3s16 delta = to - from;

	// Base + head clearance
	s32 base = get_node_cost(pos, client, ndef);
	if (base == std::numeric_limits<s32>::max()) return base;
	cost += base;

	s32 head = get_node_cost(pos + v3f(0, 1, 0), client, ndef);
	if (head == std::numeric_limits<s32>::max()) return head;
	cost += head;

	// Diagonal: side block clearance
	if (delta.X != 0 && delta.Z != 0) {
		for (const v3f &p : {
			from_pos + v3f(delta.X, 0, 0),
			from_pos + v3f(delta.X, 1, 0),
			from_pos + v3f(0, 0, delta.Z),
			from_pos + v3f(0, 1, delta.Z)
		}) {
			s32 c = get_node_cost(p, client, ndef);
			if (c == std::numeric_limits<s32>::max()) return c;
			cost += c;
		}
	}

	// Upward: 2 extra head clearance
	if (delta.Y > 0) {
		for (int i = 2; i <= 3; ++i) {
			v3f above = pos + v3f(0, i, 0);
			s32 c = get_node_cost(above, client, ndef);
			if (c == std::numeric_limits<s32>::max()) return c;
			cost += c;
		}
	}

	return cost;
}

PathNode::PathNode(const v3f &pos, const v3f &target, Client *client, const NodeDefManager *ndef, PathNode* owner)
	: position(pos), target_position(target), parent(owner)
{
	a_cost = get_a_cost(pos, owner ? owner->position : pos, client, ndef);
	g_cost = get_g_cost(pos, target);
	h_cost = owner ? owner->h_cost + get_g_cost(pos, owner->position) * 10 : 0;
	f_cost = g_cost + h_cost + a_cost;
}

struct NodeCompare {
	bool operator()(const PathNode* a, const PathNode* b) const {
		return a->f_cost > b->f_cost;
	}
};

static std::vector<v3s16> get_neighbors(const v3s16 &pos) {
	std::vector<v3s16> neighbors;
	for (s16 dx = -1; dx <= 1; ++dx)
		for (s16 dy = -1; dy <= 1; ++dy)
			for (s16 dz = -1; dz <= 1; ++dz)
				if (!(dx == 0 && dy == 0 && dz == 0))
					neighbors.emplace_back(pos.X + dx, pos.Y + dy, pos.Z + dz);
	return neighbors;
}

Pathfind::Pathfind() {}

std::vector<PathNode> Pathfind::get_path(v3f start, v3f end, Client *client, const NodeDefManager *ndef, int max_depth) {
	v3s16 start_pos = to_v3s16(start);
	v3s16 end_pos = to_v3s16(end);

	std::priority_queue<PathNode*, std::vector<PathNode*>, NodeCompare> open_set;
	std::unordered_map<v3s16, PathNode*, v3s16_hash, v3s16_equal> all_nodes;
	std::set<v3s16> closed_set;

	PathNode* start_node = new PathNode(start, end, client, ndef, nullptr);
	open_set.push(start_node);
	all_nodes[start_pos] = start_node;

	PathNode* best_node = start_node;
	int depth = 0;

	while (!open_set.empty() && depth < max_depth) {
		PathNode* current = open_set.top();
		open_set.pop();

		v3s16 current_pos = to_v3s16(current->position);

		if (current_pos == end_pos) {
			std::vector<PathNode> path;
			while (current) {
				path.push_back(*current);
				current = current->parent;
			}
			std::reverse(path.begin(), path.end());
			for (auto &[_, node] : all_nodes) delete node;
			node_cost_cache.clear();
			return path;
		}

		closed_set.insert(current_pos);
		++depth;

		if (current->g_cost < best_node->g_cost)
			best_node = current;

		for (const v3s16 &neighbor_pos : get_neighbors(current_pos)) {
			if (closed_set.count(neighbor_pos)) continue;

			v3f neighbor_f = to_v3f(neighbor_pos);
			PathNode* neighbor = new PathNode(neighbor_f, end, client, ndef, current);

			auto existing = all_nodes.find(neighbor_pos);
			if (existing == all_nodes.end() || neighbor->f_cost < existing->second->f_cost) {
				open_set.push(neighbor);
				all_nodes[neighbor_pos] = neighbor;
			} else {
				delete neighbor;
			}
		}
	}

	// Fallback path to closest reached node
	std::vector<PathNode> fallback_path;
	while (best_node) {
		fallback_path.push_back(*best_node);
		best_node = best_node->parent;
	}
	std::reverse(fallback_path.begin(), fallback_path.end());

	for (auto &[_, node] : all_nodes) delete node;
	node_cost_cache.clear();
	return fallback_path;
}
