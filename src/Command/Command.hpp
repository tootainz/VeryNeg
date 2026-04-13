#pragma once


// A small abstract class for implementing the command architecturtal pattern
// Enables undo and redo
// The actual Commands have derive from this and implement the execute and undo functions
class Command {
public:

    virtual ~Command() = default; // Apaprently a virtual destructor is needed for some reason
    virtual void execute() = 0;
    virtual void undo() = 0;
};