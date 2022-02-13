#include "plugin.hpp"
#include "inc/FrequencyDivider.hpp"
#include "inc/GateProcessor.hpp"
#include "inc/Utility.hpp"

using namespace std;

struct Divider : Module
{
    enum ParamIds
    {
        MODE_PARAM,
        ENUMS(DIVISION_PARAM, 4),
        ENUMS(ON_SWITCH, 4),
        AON_SWITCH,
        AOFF_SWITCH,
        THRESHOLD_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        CLOCK_INPUT,
        ON_TRIG,
        NUM_INPUTS
    };
    enum OutputIds
    {
        TRIG_OUTPUT,
        TRIG_OUTPUT2,
        CLOCK_OUTPUT,
        NUM_OUTPUTS
    };

    enum LighIds
    {
        ENUMS(LIGHT_S1, 4),
        ENUMS(LIGHT_S2, 2),
        LIGHT_S3,
        NUM_LIGHTS
    };

    int clock1Count = 0;
    int clock2Count = 0;
    int clock3Count = 0;
    int clock4Count = 0;

    int divider1 = 0;
    int divider2 = 0;
    int divider3 = 0;
    int divider4 = 0;

    dsp::PulseGenerator clk1;
    dsp::PulseGenerator clk2;
    dsp::PulseGenerator clk3;
    dsp::PulseGenerator clk4;

    bool pulse1 = false;
    bool pulse2 = false;
    bool pulse3 = false;
    bool pulse4 = false;

    bool aswitch = false;

    dsp::SchmittTrigger clk;
    dsp::SchmittTrigger trg;
    GateProcessor gateClock;

    float r, threshold;
    bool outcome = false, outcome2 = false, outcome3 = false, outcome4 = false;

    Divider()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for (int i = 0; i < 4; i++)
        {
            configParam(DIVISION_PARAM + i, 1, 32, 1.0, "Div " + std::to_string(i + 1));
            configParam(ON_SWITCH + i, 0.0, 1.0, 1.0, "Div " + std::to_string(i + 1) + " ON/OFF");
            configLight(LIGHT_S1 + i, "Division Trigger " + std::to_string(i + 1));
        }
        configParam(AON_SWITCH, 0.f, 1.f, 0.f, "All On");
        configParam(AOFF_SWITCH, 0.f, 1.f, 0.f, "All Off");
        configParam(THRESHOLD_PARAM, 0.f, 1.f, 1.f, "Probability", "%", 0.f, 100.f);
        configInput(CLOCK_INPUT, "Clock");
        configOutput(TRIG_OUTPUT, "OR");
        configOutput(TRIG_OUTPUT2, "XOR");
        configLight(LIGHT_S2 + 0, "OR");
        configLight(LIGHT_S2 + 1, "XOR");
        configLight(LIGHT_S3, "Clock");
    }

    void process(const ProcessArgs &args) override
    {

        divider1 = round(params[DIVISION_PARAM].getValue());
        divider2 = round(params[DIVISION_PARAM + 1].getValue());
        divider3 = round(params[DIVISION_PARAM + 2].getValue());
        divider4 = round(params[DIVISION_PARAM + 3].getValue());

        threshold = params[THRESHOLD_PARAM].getValue();

        if (clk.process(inputs[CLOCK_INPUT].getVoltage()))
        {
            clock1Count++;
            clock2Count++;
            clock3Count++;
            clock4Count++;

            r = random::uniform();
            outcome = (r < threshold);
        }
        lights[LIGHT_S3].setSmoothBrightness(inputs[CLOCK_INPUT].getVoltage() / 10.f, args.sampleTime);
        lights[LIGHT_S1 + 0].setSmoothBrightness(clock1Count == 0 ? 1.f : 0.0, args.sampleTime);
        lights[LIGHT_S1 + 1].setSmoothBrightness(clock2Count == 0 ? 1.f : 0.0, args.sampleTime);
        lights[LIGHT_S1 + 2].setSmoothBrightness(clock3Count == 0 ? 1.f : 0.0, args.sampleTime);
        lights[LIGHT_S1 + 3].setSmoothBrightness(clock4Count == 0 ? 1.f : 0.0, args.sampleTime);

        /////////////////////////////////////////////////////////////////
        if (params[AON_SWITCH].getValue() == 1 || trg.process(inputs[ON_TRIG].getVoltage(4)))
        {
            for (int i = 0; i < 4; i++)
            {
                params[ON_SWITCH + i].setValue(1);
            }
        }
        if (params[AOFF_SWITCH].getValue() == 1 || trg.process(inputs[ON_TRIG].getVoltage(5)))
        {
            for (int i = 0; i < 4; i++)
            {
                params[ON_SWITCH + i].setValue(0);
            }
        }

        for (int c = 0; c < 4; c++)
        {
            // gateClock.set(inputs[ON_TRIG].getVoltage(c));
            bool val1 = params[ON_SWITCH + c].getValue();
            if (inputs[ON_TRIG].getVoltage(c) > 1.f && val1)
                params[ON_SWITCH + c].setValue(0);
            else if (inputs[ON_TRIG].getVoltage(c) > 1.f && val1 == false)
                params[ON_SWITCH + c].setValue(1);
        }
        if (params[ON_SWITCH + 0].getValue())
        {
            if (clock1Count == divider1 && outcome)
                clk1.trigger(1e-3);
            if (clock1Count >= divider1)
                clock1Count = 0;
        }

        if (params[ON_SWITCH + 1].getValue())
        {
            if (clock2Count == divider2 && outcome)
                clk2.trigger(1e-3);
            if (clock2Count >= divider2)
                clock2Count = 0;
        }

        if (params[ON_SWITCH + 2].getValue())
        {
            if (clock3Count == divider3 && outcome)
                clk3.trigger(1e-3);
            if (clock3Count >= divider3)
                clock3Count = 0;
        }
        if (params[ON_SWITCH + 3].getValue())
        {
            if (clock4Count == divider4 && outcome)
                clk4.trigger(1e-3);
            if (clock4Count >= divider4)
                clock4Count = 0;
        }

        //////////////////////////////////////////////////////////////////
        pulse1 = clk1.process(1.0f / APP->engine->getSampleTime());
        pulse2 = clk2.process(1.0f / APP->engine->getSampleTime());
        pulse3 = clk3.process(1.0f / APP->engine->getSampleTime());
        pulse4 = clk4.process(1.0f / APP->engine->getSampleTime());

        //////////////////////////////////////////////////////////////////
        outputs[TRIG_OUTPUT].setVoltage((((pulse1 || pulse2) || pulse3) || pulse4) ? 10.0f : 0.0f);

        bool xora, xorb = false;
        xora = pulse1 == pulse2;
        xorb = pulse3 == pulse4;

        outputs[TRIG_OUTPUT2].setVoltage(xora == xorb ? 0.0f : 10.0f);

        outputs[CLOCK_OUTPUT].setVoltage(inputs[CLOCK_INPUT].getVoltage());
        lights[LIGHT_S2 + 0].setSmoothBrightness((((pulse1 || pulse2) || pulse3) || pulse4) ? 1.f : 0.f, args.sampleTime);
        lights[LIGHT_S2 + 1].setSmoothBrightness(xora == xorb ? 0.f : 1.f, args.sampleTime);
    }
};
struct BtON : SVGSwitch
{
    BtON()
    {
        momentary = false;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtON0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtON1.svg")));
        shadow->opacity = 0.f;
    }
};
struct BtAON : SVGSwitch
{
    BtAON()
    {
        momentary = true;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtAON3.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtAON4.svg")));
        shadow->opacity = 0.f;
    }
};
struct BtAOFF : SVGSwitch
{
    BtAOFF()
    {
        momentary = true;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtAON0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtAON1.svg")));
        shadow->opacity = 0.f;
    }
};
struct SPOR : app::SvgPort
{
    SPOR()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPOR.svg")));
        shadow->opacity = 0.f;
    }
};
struct SPXOR : app::SvgPort
{
    SPXOR()
    {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPXOR.svg")));
        shadow->opacity = 0.f;
    }
};
struct DividerWidget : ModuleWidget
{
    DividerWidget(Divider *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpDIV.svg")));

        addInput(createInputCentered<SPCLK>(Vec(15, 23 * 1), module, Divider::CLOCK_INPUT));
        addOutput(createOutputCentered<SPCLKout>(Vec(15, 23 * 2), module, Divider::CLOCK_OUTPUT));
        addChild(createLightCentered<TinyLight<BlueLight>>(Vec(25, 23 * 1 + 8), module, Divider::LIGHT_S3));

        addParam(createParamCentered<TrimpotWs>(Vec(15, 23 * 4), module, Divider::DIVISION_PARAM + 0));
        addParam(createParamCentered<TrimpotWs>(Vec(15, 23 * 5.5), module, Divider::DIVISION_PARAM + 1));
        addParam(createParamCentered<TrimpotWs>(Vec(15, 23 * 7), module, Divider::DIVISION_PARAM + 2));
        addParam(createParamCentered<TrimpotWs>(Vec(15, 23 * 8.5), module, Divider::DIVISION_PARAM + 3));

        addParam(createParamCentered<BtON>(Vec(15, 23 * 4.7), module, Divider::ON_SWITCH + 0));
        addParam(createParamCentered<BtON>(Vec(15, 23 * 6.2), module, Divider::ON_SWITCH + 1));
        addParam(createParamCentered<BtON>(Vec(15, 23 * 7.7), module, Divider::ON_SWITCH + 2));
        addParam(createParamCentered<BtON>(Vec(15, 23 * 9.2), module, Divider::ON_SWITCH + 3));

        addParam(createParamCentered<TrimpotB>(Vec(15, 23 * 10.5), module, Divider::THRESHOLD_PARAM));

        addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(25, 23 * 4 + 8), module, Divider::LIGHT_S1 + 0));
        addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(25, 23 * 5.5 + 8), module, Divider::LIGHT_S1 + 1));
        addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(25, 23 * 7 + 8), module, Divider::LIGHT_S1 + 2));
        addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(25, 23 * 8.5 + 8), module, Divider::LIGHT_S1 + 3));

        addOutput(createOutputCentered<SPOR>(Vec(15, 23 * 12), module, Divider::TRIG_OUTPUT));
        addOutput(createOutputCentered<SPXOR>(Vec(15, 23 * 13), module, Divider::TRIG_OUTPUT2));
        addInput(createInputCentered<SPCVin>(Vec(15, 23 * 14.5), module, Divider::ON_TRIG));
        addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(25, 23 * 12 + 8), module, Divider::LIGHT_S2 + 0));
        addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(25, 23 * 13 + 8), module, Divider::LIGHT_S2 + 1));

        addParam(createParamCentered<BtAON>(Vec(15, 23 * 15.6), module, Divider::AON_SWITCH));
        addParam(createParamCentered<BtAOFF>(Vec(15, 23 * 16), module, Divider::AOFF_SWITCH));
    }
};

Model *modelDivider = createModel<Divider, DividerWidget>("Divider");