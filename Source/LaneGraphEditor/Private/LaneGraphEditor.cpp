// Copyright Epic Games, Inc. All Rights Reserved.

#include "LaneGraphEditor.h"
#include "ToolMenus.h"
#include "LaneGraphEditorSubsystem.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "FLaneGraphEditorModule"

void FLaneGraphEditorModule::StartupModule()
{
	// Make sure ToolMenus are initialized
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLaneGraphEditorModule::RegisterMenus));
}

void FLaneGraphEditorModule::RegisterMenus()
{
    // Get or extend the "LevelEditor.MainMenu.Window" menu
    FToolMenuOwnerScoped OwnerScoped(this);
    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");

    FToolMenuSection& Section = Menu->AddSection("Teleograph", FText::FromString("Teleograph"));

    Section.AddMenuEntry(
        "Build Lane Graph",
        FText::FromString("Build Lane Graph"),
        FText::FromString("Aggregates and processes Lane Nodes for Lane Navigation System"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FLaneGraphEditorModule::SystemBuildGraphCommand))
    );
}

void FLaneGraphEditorModule::SystemBuildGraphCommand()
{
    if (GIsEditor) // Ensure you are in an editor build
    {
        // GEditor is a global pointer to the UEditorEngine instance
        UEditorEngine* EditorInstance = GEditor;

        if (EditorInstance)
        {
            ULaneGraphEditorSubsystem* LGESubsystem = GEditor->GetEditorSubsystem<ULaneGraphEditorSubsystem>();
            if (LGESubsystem)
            {
                LGESubsystem->BuildGraph();
            }
        }
    }
}

void FLaneGraphEditorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLaneGraphEditorModule, LaneGraphEditor)