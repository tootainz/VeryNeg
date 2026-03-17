#pragma once

#include <deque>
#include <print>

#include "Command.hpp"

class CommandHistory {

    public:

    int maxLength;
    int currentCommand;
    std::deque<std::unique_ptr<Command>> commands;

    CommandHistory(int maxLength) :
        maxLength(maxLength),
        currentCommand(0)
        {}

    void addCommand(std::unique_ptr<Command> command) {
        if (this->currentCommand > 0) {
            for (int i = currentCommand-1; i >= 0; i--) {
                this->commands.pop_front();
            }
        }
        command->execute();
        commands.push_front(std::move(command));
        this->currentCommand = 0;

        if (commands.size() > maxLength) {
            commands.pop_back();
        }
    }

    bool undo() {
        if (!this->commands.empty() && this->currentCommand < this->commands.size()) {
            std::println("undoing history");
            this->commands[currentCommand]->undo();
            this->currentCommand++;
            return true;
        }
        std::println("cannot undo");
        return false;
    }

    bool redo() {
        if (!this->commands.empty() && this->currentCommand > 0) {
        std::println("redoing history");
            this->currentCommand--;
            this->commands[currentCommand]->execute();
            return true;
        }
        std::println("cannot redo");
        return false;
    }
};