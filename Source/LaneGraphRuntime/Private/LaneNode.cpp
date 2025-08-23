// Fill out your copyright notice in the Description page of Project Settings.

#include "LaneNode.h"


ULaneNode::ULaneNode()
{
}

void ULaneNode::Init(FVector InPos) { 
	Id = FGuid::NewGuid();
	Position = InPos;
}

void ULaneNode::addPenalty(FGuid NodeId, PenaltyLevel level)
{
	switch (level) {
	case LOW:
		LowPenalty.Add(NodeId);
	case MEDIUM:
		MediumPenalty.Add(NodeId);
	case HIGH:
		HighPenalty.Add(NodeId);
	}
}


void ULaneNode::addNeighbor(FGuid NodeId)
{
	Neighbors.Add(NodeId);
}
