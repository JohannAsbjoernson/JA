#include "plugin.hpp"

struct Merge16 : Module
{
    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        ENUMS(MONO_INPUTS, 16),
        NUM_INPUTS
    };
    enum OutputIds
    {
        POLY_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        ENUMS(CHANNEL_LIGHTS, 16),
        NUM_LIGHTS
    };

    dsp::ClockDivider lightDivider;
    int channels = -1;
    int automaticChannels = 0;

    Merge16()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        for (int i = 0; i < 16; i++)
            configInput(MONO_INPUTS + i, string::f("Channel %d", i + 1));
        configOutput(POLY_OUTPUT, "Polyphonic");

        lightDivider.setDivision(512);
        onReset();
    }

    void onReset() override
    {
        channels = -1;
    }

    void process(const ProcessArgs &args) override
    {
        float w = 0.f;
        int lastChannel = -1;
        for (int c = 0; c < 16; c++)
        {
            float v = 0.f;
            if (inputs[MONO_INPUTS + c].isConnected())
            {
                lastChannel = c;
                v = inputs[MONO_INPUTS + c].getVoltage();
                w = 1.f;
            }
            else
                w = 0.f;
            outputs[POLY_OUTPUT].setVoltage(v, c);
            float deltaTime = args.sampleTime * lightDivider.getDivision();
            lights[CHANNEL_LIGHTS + c].setSmoothBrightness(w, deltaTime);
            outputs[POLY_OUTPUT].setVoltage(v, c);
        }
        automaticChannels = lastChannel + 1;

        // In order to allow 0 channels, modify `channels` directly instead of using `setChannels()`
        outputs[POLY_OUTPUT].channels = (channels >= 0) ? channels : automaticChannels;
    }

    json_t *dataToJson() override
    {
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "channels", json_integer(channels));
        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override
    {
        json_t *channelsJ = json_object_get(rootJ, "channels");
        if (channelsJ)
            channels = json_integer_value(channelsJ);
    }
};

struct Merge16Widget : ModuleWidget
{
    Merge16Widget(Merge16 *module)
    {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/panels/Panel2hpM16.svg")));

        for (int i = 0; i < 16; i++)
        {
            addInput(createInputCentered<MicroPortB>(mm2px(Vec(5.08, 6.80 + 7.35 * i)), module, Merge16::MONO_INPUTS + i));
            addChild(createLightCentered<TinyLight<WhitheLight>>(mm2px(Vec(8.08, 8.80 + 7.35 * i)), module, Merge16::CHANNEL_LIGHTS + i));
        }
        addOutput(createOutputCentered<MicroPortAO>(mm2px(Vec(5.08, 124.40)), module, Merge16::POLY_OUTPUT));
    }

    void appendContextMenu(Menu *menu) override
    {
        Merge16 *module = dynamic_cast<Merge16 *>(this->module);

        menu->addChild(new MenuSeparator);

        std::vector<std::string> channelLabels;
        channelLabels.push_back(string::f("Automatic (%d)", module->automaticChannels));
        for (int i = 0; i <= 16; i++)
        {
            channelLabels.push_back(string::f("%d", i));
        }
        menu->addChild(createIndexSubmenuItem(
            "Channels", channelLabels,
            [=]()
            { return module->channels + 1; },
            [=](int i)
            { module->channels = i - 1; }));
    }
};

Model *modelMerge16 = createModel<Merge16, Merge16Widget>("Merge16");