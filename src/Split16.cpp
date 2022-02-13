#include "plugin.hpp"

struct Split16 : Module
{
    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        POLY_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        ENUMS(MONO_OUTPUTS, 16),
        NUM_OUTPUTS
    };
    enum LightIds
    {
        ENUMS(CHANNEL_LIGHTS, 16),
        NUM_LIGHTS
    };

    int lastChannels = 0;
    dsp::ClockDivider lightDivider;

    Split16()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configInput(POLY_INPUT, "Polyphonic");
        for (int i = 0; i < 16; i++)
            configOutput(MONO_OUTPUTS + i, string::f("Channel %d", i + 1));

        lightDivider.setDivision(512);
    }

    void process(const ProcessArgs &args) override
    {
        for (int c = 0; c < 16; c++)
        {
            lastChannels = inputs[POLY_INPUT].getChannels();
            float w = 0.f;
            float v = inputs[POLY_INPUT].getVoltage(c);
            // To allow users to debug buggy modules, don't assume that undefined channel voltages are 0V.
            outputs[MONO_OUTPUTS + c].setVoltage(v);
            if (c < lastChannels && inputs[POLY_INPUT].isConnected())
                w = 1.f;
            else
                w = 0.f;
            float deltaTime = args.sampleTime * lightDivider.getDivision();
            lights[CHANNEL_LIGHTS + c].setSmoothBrightness(w, deltaTime);
        }
    }
};

struct Split16Widget : ModuleWidget
{
    Split16Widget(Split16 *module)
    {
        setModule(module);

        setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Panel2hpS16.svg")));

        for (int i = 0; i < 16; i++)
        {
            addOutput(createOutputCentered<MicroPortA>(mm2px(Vec(5.08, 6.80 + 7.35 * i)), module, Split16::MONO_OUTPUTS + i));
            addChild(createLightCentered<TinyLight<WhitheLight>>(mm2px(Vec(8.08, 8.80 + 7.35 * i)), module, Split16::CHANNEL_LIGHTS + i));
        }
        addInput(createInputCentered<MicroPortBO>(mm2px(Vec(5.08, 124.40)), module, Split16::POLY_INPUT));
    }
};

Model *modelSplit16 = createModel<Split16, Split16Widget>("Split16");