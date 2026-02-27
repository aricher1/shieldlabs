// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once


namespace ui {

    struct Command {
        
        virtual ~Command() = default;
        virtual void execute() = 0;
        virtual void undo() = 0;

    };

} // end namespace ui