#pragma once

#include "../Model/Model.hpp"
#include "../Command/Command.hpp"


class Command_Lambda : public Command {

private:

    std::function<void()> executeFunction;
    std::function<void()> undoFunction;

public:

    Command_Lambda(std::function<void()> executeFunction, std::function<void()> undoFunction) :
        executeFunction(executeFunction),
        undoFunction(undoFunction)
    {}

    void execute() override { this->executeFunction(); }
    void undo() override { this->undoFunction(); }
};


// A class that takes getter and setter lambdas
class Command_SetValue : public Command {

private:

    Model& model;
    float value;
    float oldValue;
    std::function<float()> getter;
    std::function<float(float)> setter;

public:

    Command_SetValue(Model& model, float value, std::function<float(float)> setter, std::function<float()> getter) :
        model(model),
        value(value),
        getter(getter),
        setter(setter)
    {}

    void execute() override {
        oldValue = getter();
        this->setter(this->value);
        return;
    }

    void undo() override {
        this->setter(this->oldValue);
    }
};