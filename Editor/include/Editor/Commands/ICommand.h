#pragma once

#include <string>

namespace FreelyEditor {

// Base interface for every undoable editor action. A command must be able to
// re-apply its own effect (Execute) and fully reverse it (Undo). Commands are
// owned by EditorContext's undo/redo stacks once pushed.
class ICommand {
public:
    virtual ~ICommand() = default;

    virtual void Execute() = 0;
    virtual void Undo() = 0;

    // Used for menu labels ("Undo Create Entity") and for merging consecutive
    // edits of the same kind (e.g. dragging a gizmo produces many small deltas
    // that should collapse into a single undo step).
    virtual const char* GetName() const = 0;

    // Returns true if this command and 'other' represent the same continuous
    // edit (same entity, same kind of change) and 'other' can be folded into
    // this one instead of being pushed as a separate undo step.
    virtual bool CanMerge(const ICommand& /*other*/) const { return false; }
    virtual void MergeWith(const ICommand& /*other*/) {}
};

} // namespace FreelyEditor
