// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <memory>
#include <vector>

#include "ui/Command.hpp"



namespace ui {
    
    class UndoStack {

        private:
            std::vector<std::unique_ptr<Command>> undo_stack;
            std::vector<std::unique_ptr<Command>> redo_stack;
        
        public:
            
            void execute(std::unique_ptr<Command> cmd) {
                cmd->execute();
                undo_stack.push_back(std::move(cmd));
                redo_stack.clear();
            }

            void undo() {
                if (undo_stack.empty()) { return ; }

                auto cmd = std::move(undo_stack.back());
                undo_stack.pop_back();
                cmd->undo();
                redo_stack.push_back(std::move(cmd));
            }   

            void redo() {
                if (redo_stack.empty()) { return; }

                auto cmd = std::move(redo_stack.back());
                redo_stack.pop_back();
                cmd->execute();
                undo_stack.push_back(std::move(cmd));   
            }

    };

} // end namespace ui