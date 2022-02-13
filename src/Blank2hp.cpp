#include "plugin.hpp"

struct Blank2hp : Module
{
    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        NUM_INPUTS
    };
    enum OutputIds
    {
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Blank2hp()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }
    void process(const ProcessArgs &args) override {}
};

struct Blank2hpWidget : ModuleWidget
{
    Blank2hpWidget(Blank2hp *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hp.svg")));
    }
};

Model *modelBlank2hp = createModel<Blank2hp, Blank2hpWidget>("Blank2hp");