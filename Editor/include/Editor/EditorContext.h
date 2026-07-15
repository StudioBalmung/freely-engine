#pragma once

#include <Freely/ECS/Scene.h>
#include <Freely/Project/Project.h>
#include <memory>
#include <filesystem>
#include <deque>

#include "Editor/Commands/ICommand.h"

namespace FreelyEditor {

enum class EditorState {
    Edit = 0,
    Play = 1,
    Simulate = 2
};

struct EditorContext {
    // Current Active Scene
    std::shared_ptr<Freely::Scene> ActiveScene;
    std::shared_ptr<Freely::Scene> EditorScene;

    // Selection state
    entt::entity SelectedEntity = entt::null;

    // Gizmo state
    int GizmoType = -1; // -1 = none, 0 = translate, 1 = rotate, 2 = scale
    bool GizmoWorldSpace = true; // true = ImGuizmo::WORLD, false = ImGuizmo::LOCAL

    // Editor layout state
    EditorState State = EditorState::Edit;

    // File system state
    std::filesystem::path CurrentContentBrowserDirectory;

    // --- Undo / Redo ----------------------------------------------------
    // ExecuteCommand runs the command immediately and pushes it on the undo
    // stack. If the new command can merge with the command already on top of
    // the stack (same entity, same kind of edit), it is folded into it instead
    // of growing the stack, so a single mouse drag in the viewport produces
    // one undo step rather than dozens.
    void ExecuteCommand(std::unique_ptr<ICommand> command) {
        command->Execute();

        if (!m_UndoStack.empty() && m_UndoStack.back()->CanMerge(*command)) {
            m_UndoStack.back()->MergeWith(*command);
        } else {
            m_UndoStack.push_back(std::move(command));
            if (m_UndoStack.size() > kMaxUndoDepth) {
                m_UndoStack.pop_front();
            }
        }

        m_RedoStack.clear();
    }

    bool CanUndo() const { return !m_UndoStack.empty(); }
    bool CanRedo() const { return !m_RedoStack.empty(); }

    const char* PeekUndoName() const { return m_UndoStack.empty() ? "" : m_UndoStack.back()->GetName(); }
    const char* PeekRedoName() const { return m_RedoStack.empty() ? "" : m_RedoStack.back()->GetName(); }

    void Undo() {
        if (m_UndoStack.empty()) return;
        std::unique_ptr<ICommand> command = std::move(m_UndoStack.back());
        m_UndoStack.pop_back();
        command->Undo();
        m_RedoStack.push_back(std::move(command));
    }

    void Redo() {
        if (m_RedoStack.empty()) return;
        std::unique_ptr<ICommand> command = std::move(m_RedoStack.back());
        m_RedoStack.pop_back();
        command->Execute();
        m_UndoStack.push_back(std::move(command));
    }

    void ClearHistory() {
        m_UndoStack.clear();
        m_RedoStack.clear();
    }

private:
    std::deque<std::unique_ptr<ICommand>> m_UndoStack;
    std::deque<std::unique_ptr<ICommand>> m_RedoStack;
    static constexpr size_t kMaxUndoDepth = 256;
};

} // namespace FreelyEditor
