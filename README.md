# LaneGraph
Unreal Street Lane Navigation Graph C++ Plugin for Teleograph Project

**Overview**
- **What:** A lightweight plugin that generates lane navigation graphs from spline-based street actors in-editor and exposes runtime navigation helpers. The end result is lanes of discrete points which can be used to calculate exact navigation routes (for instance for vehicles in a game or simulation).
- **Scope:** Editor tools to build graph data from `AStreetSplineActor` and a runtime `ULaneNavSubsystem` to query navigation nodes at runtime.

**Quick Install**
- **Add plugin:** Place the `LaneGraphPlugin.uplugin` folder in your project's `Plugins/` directory and enable it in the Editor's Plugins window.
- **Build:** Generate project files and build your project so the module binaries are available to the Editor and packaged builds.

**Editor Usage**
- **Street spline actor:** Place a `Street Spline Actor` (class `AStreetSplineActor`) in the level. Configure lane counts, `LaneWidth`, and `PointDensitySpacing` on the actor details panel.
- **Generate nodes:** In-editor you can call the actor's `GenerateLaneNodes()` or use the plugin's editor command to build the full graph across placed street spline actors (`ULaneGraphEditorSubsystem::BuildGraph`).
- **Resync & dirty handling:** The actor exposes `ReSync()` and the editor subsystem listens for dirty street spline events to know when a rebuild is needed. This activates a widget in the main Unreal top bar menu to refresh the graph. The finished graph is stored in a uasset for runtime interrogation.

**Runtime API (Navigation)**
Provided by the `ULaneNavSubsystem` (a `UWorldSubsystem`) to query graph data at runtime. Key public Blueprint-callable methods include:
- **`DebugNavData()`**: Validate or visualize nav data for the current world.
- **`GetClosestNode(FVector loc)`**: Returns the closest `ULaneNode*` to a world location.
- **`DrawDebugNodes(float Duration)`**: Draws debug markers for nodes for `Duration` seconds.
- **`GetPathPoints(FGuid Start, FGuid End)`**: Returns an array of node IDs forming a path between two node IDs.
- **`GetNode(FGuid Id)`**: Look up a `ULaneNode*` by ID.
- **`GetPositionsByIds(TArray<FGuid> Ids)`**: Convert node IDs to world positions.
- **`GetRandomNode()`**: Returns a random available node.
- **`HasNavData()`**: Whether nav data is loaded.

See : [LaneNavSubsystem.h](Source/LaneGraphRuntime/Public/LaneNavSubsystem.h).

**Runtime Data Types**
- **`ULaneNode`** (see [LaneNode.h](Source/LaneGraphRuntime/Public/LaneNode.h))
	- Properties: `Id`, `OriginalSplineId`, `NeighborDistance`, `Position`, `Forward`, `Neighbors`, penalty maps and lists.
	- Methods: `addNeighbor(FGuid NodeId)`, `addPenalty(FGuid NodeId, EPenaltyLevel level)`.
- **`UNodeDataAsset`** (see [NodeDataAsset.h](Source/LaneGraphRuntime/Public/NodeDataAsset.h))
	- Holds `TMap<FGuid, ULaneNode*> Nodes` and constants for asset path/prefix used when saving generated data.

**Editor Module & Tools**
- `FLaneGraphEditorModule` registers editor menus and exposes a `SystemBuildGraphCommand()` used by the editor UI. See [LaneGraphEditor.h](Source/LaneGraphEditor/Public/LaneGraphEditor.h).
- `ULaneGraphEditorSubsystem` provides `BuildGraph()` and editor integration (dirty indicators, toolbar widgets).

**Blueprint Notes**
- Many editor functions are `BlueprintCallable` to allow editor utilities to trigger graph generation.
- Runtime API functions on `ULaneNavSubsystem` are `BlueprintCallable` for gameplay scripting.

## License

Copyright © 2025 Teleograph, LLC.

Distributed under the Apache License version 2.0.