

#include "LaneNode.h"


ULaneNode::ULaneNode()
{
}

void ULaneNode::Init(FVector InPos, FGuid SplineId, float SplineNeighborDistance) {
	Id = FGuid::NewGuid();
	OriginalSplineId = SplineId;
	NeighborDistance = SplineNeighborDistance;
	Position = InPos;
}

// Always keep the highest penalty level
void ULaneNode::addPenalty(FGuid NodeId, EPenaltyLevel NewLevel)
{
	if (EPenaltyLevel* OldLevel = Penalties.Find(NodeId))
		Penalties.Emplace(NodeId, (*OldLevel >= NewLevel) ? *OldLevel : NewLevel);
	
	Penalties.Emplace(NodeId, NewLevel);
}


void ULaneNode::addNeighbor(FGuid NodeId)
{
	Neighbors.Add(NodeId);
}
