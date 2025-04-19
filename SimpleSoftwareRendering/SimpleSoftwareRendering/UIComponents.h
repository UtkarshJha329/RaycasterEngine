#pragma once

#include <any>
#include <vector>
#include <functional>

#include "UIGeometry.h"

struct Node {

	int id;
	int parentNodeId;
	
	std::vector<int> childNodesId;

	std::function<void(std::any)> func;

};