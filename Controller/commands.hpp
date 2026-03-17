#pragma once

#include "../Model/Model.hpp"
#include "../Command/Command.hpp"


class Command_NextNegative : public Command {

    private:
    Model& model;

    public:

    Command_NextNegative(Model& model) : model(model) {}

    void execute() {
        this->model.nextNegative();
    }

    void undo() {
        this->model.previousNegative();
    }
};

class Command_PreviousNegative : public Command {

    private:
    Model& model;

    public:

    Command_PreviousNegative(Model& model) : model(model) {}

    void execute() {
        this->model.previousNegative();
    }

    void undo() {
        this->model.nextNegative();
    }
};

class Command_SetExposure: public Command {

    private:
    Model& model;
    float value;
    float oldValue;

    public:

    Command_SetExposure(Model& model, float value) :
        value(value),
        model(model),
        oldValue(0.0)
        {}

    void execute() {
        this->oldValue = this->model.getExposure();
        this->model.setExposure(this->value);
    }

    void undo() {
        this->model.setExposure(this->oldValue);
    }
};

class Command_SetRBalance : public Command {

    private:
    Model& model;
    float value;
    float oldValue;

    public:

    Command_SetRBalance(Model& model, float value) :
        value(value),
        model(model)
        {}

    void execute() {
        this->oldValue = this->model.getRBalance();
        this->model.setRBalance(this->value);
    }

    void undo() {
        this->model.setRBalance(this->oldValue);
    }
};

class Command_SetGBalance : public Command {

    private:
    Model& model;
    float value;
    float oldValue;

    public:

    Command_SetGBalance(Model& model, float value) :
        value(value),
        model(model)
        {}

    void execute() {
        this->oldValue = this->model.getGBalance();
        this->model.setGBalance(this->value);
    }

    void undo() {
        this->model.setGBalance(this->oldValue);
    }
};

class Command_SetBBalance : public Command {

    private:
    Model& model;
    float value;
    float oldValue;

    public:

    Command_SetBBalance(Model& model, float value) :
        value(value),
        model(model)
        {}

    void execute() {
        this->oldValue = this->model.getBBalance();
        this->model.setBBalance(this->value);
    }

    void undo() {
        this->model.setBBalance(this->oldValue);
    }
};


/*
Still left to implement:

void ButtonPressAddNegative();

void ButtonPressConvert();
*/