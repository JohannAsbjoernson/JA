#include "plugin.hpp"
#include "inc/FrequencyDivider.hpp"
#include "inc/GateProcessor.hpp"
#include "inc/Utility.hpp"
#include "components.hpp"

struct CRG : Module
{
    enum ParamIds
    {
        ENUMS(PROB_PARAM, 16),
        CHANNEL_PARAM,
        MODE_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        CLOCK_INPUT,
        RESET_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        ENUMS(GATE_OUTPUT, 16),
        ENUMS(TRIG_OUTPUT, 16),
        ENUMS(CLOCK_OUTPUT, 16),
        POLY_GATE_OUTPUT,
        POLY_TRIG_OUTPUT,
        POLY_CLOCK_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        ENUMS(GATE_LIGHT, 16 * 2),
        ENUMS(TRIG_LIGHT, 16 * 2),
        ENUMS(CLOCK_LIGHT, 16 * 2),
        NUM_LIGHTS
    };

    GateProcessor gateClock[16];
    GateProcessor gateReset;

    float probabilities[16] = {};
    bool outcomes[16] = {};
    bool triggers[16] = {};

    dsp::PulseGenerator pgTrig[16];

    CRG()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configSwitch(MODE_PARAM, 0.0f, 1.0f, 0.0f, "Mode", {"Multi (gates run independent)", "Single (one gate at a time)"});
        configParam(CHANNEL_PARAM, 1, 16, 8, "Number of Channels");

        for (int i = 0; i < 16; i++)
        {
            configParam(PROB_PARAM + i, 0.0f, 1.0f, 0.5f, "Probability " + std::to_string(i + 1), " %", 0.0f, 100.0f, 0.0f);
        }
        getParamQuantity(MODE_PARAM)->randomizeEnabled = false;
        getParamQuantity(CHANNEL_PARAM)->randomizeEnabled = false;
        getParamQuantity(PROB_PARAM)->randomizeEnabled = false;
    }

    void onReset() override
    {
        for (int i = 0; i < 16; i++)
            gateClock[i].reset();

        gateReset.reset();

        resetOutputs();
    }

    void resetOutputs()
    {
        for (int i = 0; i < 16; i++)
        {
            outcomes[i] = false;
            pgTrig[i].reset();
        }
    }

    void process(const ProcessArgs &args) override
    {

        // determine the mode and polyphony
        bool singleMode = params[MODE_PARAM].getValue() > 0.5f;
        bool isPolyphonic = inputs[CLOCK_INPUT].isPolyphonic();

        // process clock input
        int numPolyChannels = params[CHANNEL_PARAM].getValue();
        if (singleMode)
        {
            // single mode can only ever work on 1 clock so always use channel 1
            gateClock[0].set(inputs[CLOCK_INPUT].getNormalVoltage(0.0f));

            // if we do have a poly clock input - we'll limit the single mode outputs to the given number of channels
            if (isPolyphonic)
                numPolyChannels = inputs[CLOCK_INPUT].getChannels();
        }
        else if (isPolyphonic)
        {
            // make sure we handle poly signals the way we want to - we'll wrap around if number of channels < 8 to ensure we always have 8 on the individual outputs
            for (int i = 0; i < numPolyChannels; i++)
            {
                gateClock[i].set(inputs[CLOCK_INPUT].getVoltage(i));
            }
        }
        else
        {
            // non-poly use single clock always and keep the channels as 8
            gateClock[0].set(inputs[CLOCK_INPUT].getNormalVoltage(0.0f));
        }

        // reset input
        gateReset.set(inputs[RESET_INPUT].getNormalVoltage(0.0f));

        // handle the reset - takes precedence over everything else
        if (gateReset.leadingEdge())
            resetOutputs();
        else
        {

            if (singleMode)
            {
                // single mode - determine outcome for random output accepts single clock input only. If a poly clock is presented, only channel 1 is used

                if (gateClock[0].leadingEdge())
                {

                    float probSum = 0.0f;
                    for (int i = 0; i < numPolyChannels; i++)
                    {
                        probabilities[i] = clamp(params[PROB_PARAM + i].getValue(), 0.f, 1.f);
                        probSum = probSum + probabilities[i];
                    }

                    float r = random::uniform() * probSum;

                    float tLow = 0.0f, tHigh = 0.0f;
                    for (int i = 0; i < numPolyChannels; i++)
                    {
                        // make sure we don't use any outs beyond the number of input channels
                        if (i < numPolyChannels)
                        {
                            // upper threshold for this output
                            tHigh += probabilities[i];

                            bool prevOutcome = outcomes[i];
                            outcomes[i] = (probabilities[i] > 0.0f && r > tLow && r <= tHigh);

                            // fire the trigger here if we've flipped from 0 to 1 on the outcome
                            if (outcomes[i] && !prevOutcome)
                                pgTrig[i].trigger(1e-3f);

                            // upper threshold for this output becomes lower threshold for next output
                            tLow = tHigh;
                        }
                        else
                            outcomes[i] = false;
                    }
                }
            }
            else
            {
                // multi mode - determine the outcome for each output individually
                for (int i = 0; i < numPolyChannels; i++)
                {
                    int j = (isPolyphonic ? i : 0);

                    if (gateClock[j].leadingEdge())
                    {
                        float r = random::uniform();
                        float threshold = clamp(params[PROB_PARAM + i].getValue(), 0.f, 1.f);

                        bool prevOutcome = outcomes[i];
                        outcomes[i] = (r < threshold);

                        // fire the trigger here if we've flipped from 0 to 1 on the outcome
                        if (outcomes[i] && !prevOutcome)
                            pgTrig[i].trigger(1e-3f);
                    }
                    else
                        outcomes[i] = false;
                }
            }
        }

        // now process the outputs and lights

        outputs[POLY_GATE_OUTPUT].setChannels(numPolyChannels);
        outputs[POLY_TRIG_OUTPUT].setChannels(numPolyChannels);
        outputs[POLY_CLOCK_OUTPUT].setChannels(numPolyChannels);

        float gate, trig, clock;
        for (int i = 0; i < 16; i++)
        {
            int j = (singleMode || !isPolyphonic ? 0 : i);

            triggers[i] = pgTrig[i].remaining > 0.0f;
            pgTrig[i].process(args.sampleTime);

            gate = boolToGate(outcomes[i]);
            trig = boolToGate(triggers[i]);
            clock = boolToGate(outcomes[i] && gateClock[j].high());

            lights[GATE_LIGHT + i].setBrightness(boolToLight(outcomes[i]));
            outputs[GATE_OUTPUT + i].setVoltage(gate);

            lights[TRIG_LIGHT + i * 2 + 0].setSmoothBrightness(boolToLight(triggers[i]), args.sampleTime);
            lights[TRIG_LIGHT + i * 2 + 1].setSmoothBrightness(-1.f, args.sampleTime);
            outputs[TRIG_OUTPUT + i].setVoltage(trig);

            lights[CLOCK_LIGHT + i * 2 + 0].setBrightness(boolToLight2(outcomes[i] && gateClock[j].high()));
            lights[CLOCK_LIGHT + i * 2 + 1].setBrightness(0.3f);
            outputs[CLOCK_OUTPUT + i].setVoltage(clock);

            // set the poly outputs
            if (i < numPolyChannels)
            {
                outputs[POLY_GATE_OUTPUT].setVoltage(gate, i);
                outputs[POLY_TRIG_OUTPUT].setVoltage(trig, i);
                outputs[POLY_CLOCK_OUTPUT].setVoltage(clock, i);
            }
        }
    }
};

struct CRGWidget : ModuleWidget
{

    std::string panelName;

    CRGWidget(CRG *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpCRG.svg")));

        // clock and reset input
        addInput(createInputCentered<SmallPortClk>(Vec(15, 23 * 1), module, CRG::CLOCK_INPUT));
        addInput(createInputCentered<SmallPortRes>(Vec(15, 23 * 2), module, CRG::RESET_INPUT));

        // mode switch
        addParam(createParamCentered<SmallBtMS>(Vec(15, 23 * 3), module, CRG::MODE_PARAM));
        addParam(createParamCentered<MTrimBs>(Vec(15, 23 * 3.9), module, CRG::CHANNEL_PARAM));

        for (int i = 0; i < 8; i++)
        {
            addParam(createParamCentered<TrimpotW>(Vec(15, 23 * (i + 4.8)), module, CRG::PROB_PARAM + i));
            //addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(26, 23 * (i + 5.1)), module, CRG::CLOCK_LIGHT + i * 2));
        }

        addOutput(createOutputCentered<SmallPortPG>(Vec(15, 10 + 23 * 13), module, CRG::POLY_GATE_OUTPUT));
        addOutput(createOutputCentered<SmallPortPT>(Vec(15, 10 + 23 * 14), module, CRG::POLY_TRIG_OUTPUT));
        addOutput(createOutputCentered<SmallPortPC>(Vec(15, 10 + 23 * 15), module, CRG::POLY_CLOCK_OUTPUT));
    }
};
Model *modelCRG = createModel<CRG, CRGWidget>("CRG");