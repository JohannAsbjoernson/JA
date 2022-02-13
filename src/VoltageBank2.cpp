#include "plugin.hpp"
#include "dsp/digital.hpp"
#include "inc/utils.hpp"
#include <algorithm>
#include <settings.hpp>
#include "components.hpp"

#define MODULE_WIDTH 2
#define ROWS 2
#define FADERS 16

struct ChannelSetting
{
    bool dirty = true;
    int channels = 8;
    void set(int c)
    {
        channels = c;
        dirty = true;
    }
    int update()
    {
        if (dirty)
        {
            dirty = false;
            return channels;
        }
        else
            return -1;
    }
};

enum ChannelSetMode
{
    BUTTON_ONLY,
    AUTO_COPY_INPUT,
    COPY_INPUT_ON_BUTTON,
    CHANNEL_SET_MODES
};

enum VoltageBank2SampleMode
{
    SAMPLE_ABSOLUTE,
    SAMPLE_BY_MODS,
    VoltageBank2_SAMPLE_MODES
};

struct TowerRow
{
    bool bipolar = false;
    float modded_values[FADERS] = {};
    int channels = FADERS;
    ChannelSetMode channel_set_mode = COPY_INPUT_ON_BUTTON;
    VoltageBank2SampleMode sample_mode = SAMPLE_ABSOLUTE;
    //VoltageBank2ViewMode view_mode = SLIDER;

    std::string name = "A";

    json_t *toJson()
    {
        json_t *rootJ = json_object();
        //json_object_set(rootJ, "view_mode", json_integer(view_mode));
        json_object_set(rootJ, "channel_set_mode", json_integer(channel_set_mode));
        json_object_set(rootJ, "sample_mode", json_integer(sample_mode));
        return rootJ;
    }

    void fromJson(json_t *rootJ)
    {
        //view_mode = (VoltageBank2ViewMode)json_integer_value(json_object_get(rootJ, "view_mode"));
        channel_set_mode = (ChannelSetMode)json_integer_value(json_object_get(rootJ, "channel_set_mode"));
        sample_mode = (VoltageBank2SampleMode)json_integer_value(json_object_get(rootJ, "sample_mode"));
    }

    TowerRow(std::string n)
    {
        name = n;
        for (int i = 0; i < FADERS; i++)
            modded_values[i] = 0.0f;
    }
};

struct VoltageBank2 : Module
{
    enum ParamIds
    {
        ENUMS(FADER_PARAMS, FADERS *ROWS),
        ENUMS(POLARITY_PARAM, 2),
        ENUMS(TRIM_PARAMS, 2),
        ENUMS(SAMPLE_PARAMS, 2),
        ENUMS(CHANNEL_PARAMS, 2),
        NUM_PARAMS
    };
    enum InputIds
    {
        ENUMS(FADER_INPUTS, 2),
        ENUMS(TRIM_INPUTS, 2),
        ENUMS(SAMPLE_INPUTS, 2),
        CV_INPUT,
        STEPINC_INPUT,
        STEPDEC_INPUT,
        RANDOM_INPUT,
        RESET_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        ENUMS(FADER_OUTPUTS, ROWS),
        MONOOUT_OUTPUT,
        TRG_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    ////////////////////////////N1
    dsp::ClockDivider connectionUpdater;

    dsp::SchmittTrigger stepIncreaseTrigger;
    dsp::SchmittTrigger stepDecreaseTrigger;
    dsp::SchmittTrigger stepRandomTrigger;
    dsp::SchmittTrigger resetTrigger;

    dsp::PulseGenerator triggerPulse1;
    bool trg_pulse1 = false;

    int channels = 0;
    int activeChannel = -1;
    int step = 0;
    float offset = 0.f; // Offset by CV Input

    bool cvConnected = false;
    bool increaseConnected = false;
    bool decreaseConnected = false;
    bool randomConnected = false;
    bool resetConnected = false;
    ////////////////////////////N1
    bool dirty = true;

    SampleDivider divider = SampleDivider(64);

    ChannelSetting channel_settings[2];

    TriggerSwitch triggers[ROWS];
    TriggerSwitch poly_triggers[ROWS][FADERS];

    TowerRow rows[2] = {TowerRow("A"), TowerRow("B")};

    void set_dirty()
    {
        dirty = true;
    }
    void onReset() override
    {
        color = 0.0f;
        divider.reset();
        set_dirty();
    }
    void toggle_mode(int r)
    {
        rows[r].bipolar = !rows[r].bipolar;
    }

    void set_channels(int r, int c)
    {
        channel_settings[r].channels = c;
        update_row(r);
    }
    void update_channel_param(int row, int c = -1)
    {
        if (c == -1)
            params[CHANNEL_PARAMS + row].setValue(channel_settings[row].channels);
        else
            params[CHANNEL_PARAMS + row].setValue(c);
    }
    void update_row(int r)
    {
        if (rows[r].channel_set_mode == AUTO_COPY_INPUT)
        {
            if (inputs[FADER_INPUTS + r].isConnected())
            {
                int c = inputs[FADER_INPUTS + r].getChannels();
                if (c != channel_settings[r].channels)
                {
                    channel_settings[r].set(c);
                    update_channel_param(r);
                }
            }
        }
        int _channels = (int)floor(params[CHANNEL_PARAMS + r].getValue());
        if (_channels != channel_settings[r].channels)
        {
            channel_settings[r].set(_channels);
            update_channel_param(r);
        }

        // if(channel_settings[r].update() > -1){
        //   set_channels(r, channel_settings[r].channels);
        //   set_dirty();
        //   return;
        // }

        bool bipolar = params[POLARITY_PARAM + r].value > 0;
        if (bipolar != rows[r].bipolar)
        {
            rows[r].bipolar = bipolar;
        }
        float _min = rows[r].bipolar ? -10.0f : 0.0f;
        float _max = _min + 20.0f;
        if (inputs[FADER_INPUTS + r].isConnected())
        {
            // if(outputs[r].isConnected()){
            outputs[r].setChannels(channel_settings[r].channels);
            int input_channels = inputs[FADER_INPUTS + r].getChannels();
            int row = r * FADERS;
            float offset = rows[r].bipolar ? -5.0f : 0.0f;
            if (inputs[TRIM_INPUTS + r].isConnected())
            {
                int trim_channels = inputs[TRIM_INPUTS + r].getChannels();
                for (int i = 0; i < channel_settings[r].channels; i++)
                {
                    rows[r].modded_values[i] = clamp(
                        params[FADER_PARAMS + i + row].value + offset + inputs[FADER_INPUTS + r].getVoltage(i % input_channels) * inputs[TRIM_INPUTS + r].getVoltage(i % trim_channels) * 0.2f * params[TRIM_PARAMS + r].value, _min, _max);
                    outputs[r].setVoltage(rows[r].modded_values[i], i);
                }
            }
            else
            {
                for (int i = 0; i < channel_settings[r].channels; i++)
                {
                    rows[r].modded_values[i] = clamp(
                        params[FADER_PARAMS + i + row].value + offset + params[TRIM_PARAMS + r].value * inputs[FADER_INPUTS + r].getVoltage(i % input_channels), _min, _max);
                    outputs[r].setVoltage(rows[r].modded_values[i], i);
                }
            }
            // }
        }
        else
        {
            // if(outputs[r].isConnected()){
            outputs[r].setChannels(channel_settings[r].channels);
            float offset = rows[r].bipolar ? -5.0f : 0.0f;
            int row = r * FADERS;
            for (int i = 0; i < channel_settings[r].channels; i++)
            {
                rows[r].modded_values[i] = clamp(params[FADER_PARAMS + i + row].value + offset, _min, _max);
                outputs[r].setVoltage(rows[r].modded_values[i], i);
            }
            // }
        }
    }
    void try_settings_channels(int r)
    {
        if (rows[r].channel_set_mode == COPY_INPUT_ON_BUTTON)
        {
            int c = inputs[FADER_INPUTS + r].getChannels();
            if (c != channel_settings[r].channels)
            {
                channel_settings[r].set(c);
                params[CHANNEL_PARAMS + r].setValue(channel_settings[r].channels);
                set_dirty();
            }
        }
    }
    void sample_absolute(int r)
    {
        try_settings_channels(r);
        int ic = inputs[FADER_INPUTS + r].getChannels();
        int row_offset = r * FADERS;
        for (int i = 0; i < FADERS; i++)
        {
            float cv = inputs[FADER_INPUTS + r].getVoltage(i % ic);
            params[i + row_offset].setValue(rows[r].bipolar ? cv + 5.0f : cv);
        }
    }
    void sample_by_mods(int r)
    {
        try_settings_channels(r);
        int row_offset = r * FADERS;
        float bioffset = rows[r].bipolar ? 5.0f : 0.0f;
        for (int i = 0; i < FADERS; i++)
        {
            params[i + row_offset].setValue(rows[r].modded_values[i] + bioffset);
        }
    }
    void randomize_row(int r, float mod_scaling)
    {
        try_settings_channels(r);
        if (rows[r].sample_mode == SAMPLE_ABSOLUTE)
        {
            if (rows[r].bipolar)
            {
                for (int i = 0; i < FADERS; i++)
                {
                    params[i + r * FADERS].setValue(5.0f + (1.0f - random::uniform() * 2.0f) * 5.0f * abs(mod_scaling));
                }
            }
            else
            {
                for (int i = 0; i < FADERS; i++)
                {
                    params[i + r * FADERS].setValue(random::uniform() * 10.0f * abs(mod_scaling));
                }
            }
        }
        else
        {
            float offset = rows[r].bipolar ? -5.0f : 0.0f;
            for (int i = 0; i < FADERS; i++)
            {
                float ra = offset + 10.0f * random::uniform();
                params[i + r * FADERS].setValue(ra);
            }
        }
    }
    void sample(int r)
    {
        switch (rows[r].sample_mode)
        {
        case SAMPLE_ABSOLUTE:
            sample_absolute(r);
            break;
        case SAMPLE_BY_MODS:
            sample_by_mods(r);
            break;
        default:
            break;
        }
    }
    void try_mutate_row(int r)
    {
        if (inputs[FADER_INPUTS + r].isConnected())
        {
            if (inputs[SAMPLE_INPUTS + r].isConnected())
            {
                int c = inputs[SAMPLE_INPUTS + r].getChannels();
                if (c == 1)
                {
                    // single channel copy trigger
                    triggers[r].update(inputs[SAMPLE_INPUTS + r].getVoltage(0) + params[SAMPLE_PARAMS + r].getValue());
                    if (triggers[r].state == PRESSED)
                    {
                        sample(r);
                    }
                }
                else
                {
                    // multi channel copy trigger
                    int tc = rows[r].channels;
                    if (rows[r].sample_mode == SAMPLE_ABSOLUTE)
                    {
                        for (int i = 0; i < tc; i++)
                        {
                            poly_triggers[r][i].update(inputs[SAMPLE_INPUTS + r].getVoltage(i % c) + params[SAMPLE_PARAMS + r].getValue());
                            if (poly_triggers[r][i].state == PRESSED)
                            {
                                int fc = inputs[FADER_INPUTS + r].getChannels();
                                float cv = inputs[FADER_INPUTS + r].getVoltage(i % fc);
                                params[i + r * FADERS].setValue(rows[r].bipolar ? cv + 5.0f : cv);
                            }
                        }
                    }
                    else
                    {
                        sample(r);
                    }
                }
            }
            else
            {
                // copy button only
                triggers[r].update(params[SAMPLE_PARAMS + r].getValue());
                if (triggers[r].state == PRESSED)
                    sample(r);
            }
        }
        else
        {
            // no input
            triggers[r].update(inputs[SAMPLE_INPUTS + r].getVoltage(0) + params[SAMPLE_PARAMS + r].getValue());
            if (triggers[r].state == PRESSED)
                randomize_row(r, params[TRIM_PARAMS + r].getValue());
        }
    }
    void process(const ProcessArgs &args) override
    {
        if (divider.step())
        {
            try_mutate_row(0);
            try_mutate_row(1);
            update_row(0);
            update_row(1);
        }
        ////////////////////////////////// N1
        int chnr = params[CHANNEL_PARAMS].getValue();
        //int chnr = inputs[FADER_INPUTS].getChannels();
        if (connectionUpdater.process())
            updateConnections();

        if (resetConnected && resetTrigger.process(inputs[RESET_INPUT].getVoltage()))
        {
            step = 0;
        }

        if (increaseConnected && stepIncreaseTrigger.process(inputs[STEPINC_INPUT].getVoltage()))
        {
            step++;
            triggerPulse1.trigger(1e-3f);
            if (step >= chnr)
                step = 0;
        }

        if (decreaseConnected && stepDecreaseTrigger.process(inputs[STEPDEC_INPUT].getVoltage()))
        {
            step--;
            triggerPulse1.trigger(1e-3f);
            if (step < 0)
                step = chnr - 1;
        }

        if (randomConnected && stepRandomTrigger.process(inputs[RANDOM_INPUT].getVoltage()))
        {
            step = std::floor(random::uniform() * chnr);
            triggerPulse1.trigger(1e-3f);
        }

        if (cvConnected && chnr > 1)
        {
            float cv = inputs[CV_INPUT].getVoltage();
            int stepOffset = std::floor(cv * (chnr / 10.f));
            activeChannel = step + stepOffset;
            if (activeChannel < 0)
                activeChannel = chnr + stepOffset;

            if (activeChannel >= chnr)
                activeChannel = stepOffset - (chnr - step);
        }
        else
        {
            activeChannel = step;
        }

        if (chnr <= 0)
            activeChannel = -1;

        if (activeChannel > -1)
        {
            outputs[MONOOUT_OUTPUT].setVoltage(outputs[FADER_OUTPUTS].getVoltage(activeChannel));
        }
        trg_pulse1 = triggerPulse1.process(2.0 / args.sampleRate);
        outputs[TRG_OUTPUT].setVoltage((trg_pulse1 ? 10.0f : 0.0f));
    }
    void updateConnections()
    {
        cvConnected = inputs[CV_INPUT].isConnected();
        increaseConnected = inputs[STEPINC_INPUT].isConnected();
        decreaseConnected = inputs[STEPDEC_INPUT].isConnected();
        randomConnected = inputs[RANDOM_INPUT].isConnected();
        resetConnected = inputs[RESET_INPUT].isConnected();
    }

    ///////////////////////////////////////////N1

    json_t *dataToJson() override
    {
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "step", json_integer(step));
        // json_t *csJ = json_array();
        json_t *rowsJ = json_array();
        for (int i = 0; i < ROWS; i++)
        {
            json_array_append(rowsJ, rows[i].toJson());
            // json_array_append(csJ, json_integer(channel_settings[i].channels));
        }
        // json_object_set(rootJ, "channels", csJ);
        json_object_set(rootJ, "rows", rowsJ);

        json_object_set(rootJ, "color", json_real(color));
        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override
    {
        json_t *rowsJ = json_object_get(rootJ, "rows");
        for (int i = 0; i < ROWS; i++)
        {
            // channel_settings[i].set(json_integer_value(json_array_get(csJ, i)));
            rows[i].fromJson(json_array_get(rowsJ, i));
        }
        divider.load(json_object_get(rootJ, "divider"));
        color = json_number_value(json_object_get(rootJ, "color"));

        json_t *legacy_channels = json_object_get(rootJ, "channels");
        if (legacy_channels)
        {
            update_channel_param(0, json_integer_value(json_array_get(legacy_channels, 0)));
            update_channel_param(1, json_integer_value(json_array_get(legacy_channels, 1)));
        }
        json_t *stepJ = json_object_get(rootJ, "step");
        if (stepJ)
        {
            step = json_integer_value(stepJ);
        }
    }

    void configFadersInRow(int r, bool bi)
    {
        for (int i = 0; i < FADERS; i++)
            configParam(FADER_PARAMS + i + r * FADERS, 0.0f, 10.0f, 5.0f, rows[r].name + ":" + to_string(i + 1));
    }
    float color = 0.0f;
    void offset_color()
    {
        color = float_wrap(color + 0.0713f);
        set_dirty();
    }

    VoltageBank2()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        connectionUpdater.setDivision(32);
        for (int r = 0; r < ROWS; r++)
        {
            configParam(TRIM_PARAMS + r, -1.0f, 1.0f, 0.0f, rows[r].name + ": Trimpot for CV in");
            configSwitch(POLARITY_PARAM + r, 0, 1, 1, rows[r].name + ": Polarity", {"0v to 10v", "-10v to 10v"});
            configParam(SAMPLE_PARAMS + r, 0, 1, 0, rows[r].name + ": Sample and Hold input Signal(s)");
            configParam(CHANNEL_PARAMS + r, 1, 16, 8, rows[r].name + ":poly-channels");
            configFadersInRow(r, false);
        }
        onReset();
    }
};
struct VoltageBank2Widget : ModuleWidget
{
    VoltageBank2Widget(VoltageBank2 *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpSH.svg")));

        addInput(createInputCentered<SmallPortC>(Vec(15, 23 * 1), module, VoltageBank2::FADER_INPUTS + 0 * 2));
        addParam(createParamCentered<SmallBtS>(Vec(15, 23 * 2), module, VoltageBank2::SAMPLE_PARAMS + 0 * 2));
        addParam(createParamCentered<SmallBtP>(Vec(15, 23 * 3), module, VoltageBank2::POLARITY_PARAM + 0 * 2));
        addInput(createInputCentered<SmallPortB>(Vec(15, 23 * 4), module, VoltageBank2::TRIM_INPUTS + 0 * 2));
        addParam(createParamCentered<TrimpotW>(Vec(15, 23 * 5), module, VoltageBank2::TRIM_PARAMS + 0 * 2));
        addInput(createInputCentered<SmallPortG>(Vec(15, 23 * 6), module, VoltageBank2::SAMPLE_INPUTS + 0 * 2));
        addOutput(createOutputCentered<SmallPortO>(Vec(15, 23 * 7), module, VoltageBank2::FADER_OUTPUTS + 0 * 2));
        addInput(createInputCentered<SmallPortCV>(Vec(15, 20 + 23 * 8), module, VoltageBank2::CV_INPUT));
        addInput(createInputCentered<SmallPortSP>(Vec(15, 20 + 23 * 9), module, VoltageBank2::STEPINC_INPUT));
        addInput(createInputCentered<SmallPortSM>(Vec(15, 20 + 23 * 10), module, VoltageBank2::STEPDEC_INPUT));
        addInput(createInputCentered<SmallPortRND>(Vec(15, 20 + 23 * 11), module, VoltageBank2::RANDOM_INPUT));
        addInput(createInputCentered<SmallPortRST>(Vec(15, 20 + 23 * 12), module, VoltageBank2::RESET_INPUT));
        addOutput(createOutputCentered<SmallPortSTEP>(Vec(15, 20 + 23 * 14), module, VoltageBank2::MONOOUT_OUTPUT));
        addOutput(createOutputCentered<SmallPortTRIG>(Vec(15, 20 + 23 * 15), module, VoltageBank2::TRG_OUTPUT));
    }
};

Model *modelVoltageBank2 = createModel<VoltageBank2, VoltageBank2Widget>("VoltageBank2");