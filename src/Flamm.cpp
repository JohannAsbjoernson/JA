#include "plugin.hpp"
#include "inc/Utility.hpp"
#include "inc/ClockOscillator.hpp"
#include "inc/GateProcessor.hpp"

struct Flamm : Module
{
    enum ParamIds
    {
        PULSES_PARAM,
        RATE_PARAM,
        RANGE_PARAM,
        RETRIGGER_PARAM,
        MANUAL_PARAM,
        PROBABILITY_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        CLOCK_INPUT,
        RATECV_INPUT,
        TRIGGER_INPUT,
        PULSESCV_INPUT,
        PROBABILITYCV_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        PULSES_OUTPUT,
        START_OUTPUT,
        DURATION_OUTPUT,
        END_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        CLOCK_LIGHT,
        MANUAL_PARAM_LIGHT,
        NUM_LIGHTS
    };

    int counter = -1;
    bool bursting = false;
    bool prevBursting = false;
    bool startBurst = false;
    bool prob = true;

    bool state = false;
    float clockFreq = 1.0f;

    ClockOscillator clock;
    GateProcessor gpClock;
    GateProcessor gpTrig;
    dsp::PulseGenerator pgStart;
    dsp::PulseGenerator pgEnd;

    Flamm()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(RATE_PARAM, 0.0f, 5.0f, 0.0f, "Burst Rate (internal)");
        configSwitch(RANGE_PARAM, 0.0f, 1.0f, 0.0f, "Internal Rate", {"Slow", "Fast"});
        configSwitch(RETRIGGER_PARAM, 0.0f, 1.0f, 0.0f, "Retrigger", {"OFF", "ON"});
        configParam(PULSES_PARAM, 1.0f, 24.0f, 1.0f, "# of Pulses (Burst length)");
        configParam(MANUAL_PARAM, 0.0f, 1.0f, 0.0f, "Manual Trigger");
        configParam(PROBABILITY_PARAM, 0.0f, 10.0f, 10.0f, "Pulse Probability", " %", 0.0f, 10.0f, 0.0f);
        configInput(CLOCK_INPUT, "CLOCK");
        configLight(CLOCK_LIGHT, "CLOCK");
        configInput(RATECV_INPUT, "RATE CV");
        configInput(PULSESCV_INPUT, "PULSE CV");
        configInput(PROBABILITYCV_INPUT, "PROBABILITY CV");
        configInput(TRIGGER_INPUT, "TRIGGER BURST");
        configOutput(PULSES_OUTPUT, "BURST");
        configOutput(START_OUTPUT, "BEGINNING OF BURST (TRIGGER)");
        configOutput(DURATION_OUTPUT, "DURATION OF BURST (GATE)");
        configOutput(END_OUTPUT, "END OF BURST (TRIGGER)");
        getParamQuantity(RANGE_PARAM)->randomizeEnabled = false;
        getParamQuantity(RETRIGGER_PARAM)->randomizeEnabled = false;
        getParamQuantity(MANUAL_PARAM)->randomizeEnabled = false;
    }

    void onReset() override
    {
        gpClock.reset();
        gpTrig.reset();
        pgStart.reset();
        pgEnd.reset();
        clock.reset();
        bursting = false;
        counter = -1;
    }

    void process(const ProcessArgs &args) override
    {

        // grab the current burst count taking CV into account and ensuring we don't go below 1
        int pulseCV = clamp(inputs[PULSESCV_INPUT].getVoltage(), -10.0f, 10.0f) * 2.4;
        int pulses = (int)fmaxf(params[PULSES_PARAM].getValue() + pulseCV, 1.0f);
        if (pulses > 24)
        {
            pulses = 24;
        }
        // determine clock rate
        float rateCV = clamp(inputs[RATECV_INPUT].getVoltage(), -10.0f, 10.0f);
        float rate = params[RATE_PARAM].getValue() + rateCV;
        float range = params[RANGE_PARAM].getValue();
        if (range > 0.0f)
        {
            rate = 3.0f + (rate * 2.0f);
        }

        // now set it
        clock.setPitch(rate);

        // set the trigger input value
        gpTrig.set(fmaxf(inputs[TRIGGER_INPUT].getVoltage(), params[MANUAL_PARAM].getValue() * 10.0f));
        bool retrigAllowed = params[RETRIGGER_PARAM].getValue() > 0.5f;

        // leading edge of trigger input fires off the burst if we can
        if (gpTrig.leadingEdge())
        {
            if (!bursting || (bursting && retrigAllowed))
            {
                gpClock.reset();
                clock.reset();

                // set the burst to go off
                startBurst = true;
                counter = -1;
            }
        }

        // tick the internal clock over here as we could have reset the clock above
        clock.step(args.sampleTime);

        // get the clock value we want to use (internal or external)
        float internalClock = 5.0f * clock.sqr();
        float clockState = inputs[CLOCK_INPUT].getNormalVoltage(internalClock);
        gpClock.set(clockState);

        // process the burst logic based on the results of the above
        if (gpClock.leadingEdge())
        {

            // determine probability of pulse firing
            float pCV = clamp(inputs[PROBABILITYCV_INPUT].getVoltage(), -10.0f, 10.0f);
            prob = (random::uniform() <= clamp((params[PROBABILITY_PARAM].getValue() + pCV) / 10.0f, 0.0f, 1.0f));

            // process burst count
            if (startBurst || bursting)
            {
                if (++counter >= pulses)
                {
                    counter = -1;
                    bursting = false;
                }
                else
                {
                    bursting = true;
                }

                startBurst = false;
            }
        }

        // end the duration after the last pulse, not at the next clock cycle
        if (gpClock.trailingEdge() && counter + 1 >= pulses)
            bursting = false;

        // set the duration start trigger if we've changed from not bursting to bursting
        bool startOut = false;
        if (!prevBursting && bursting)
        {
            pgStart.trigger(1e-3f);
        }
        else
            startOut = pgStart.process(args.sampleTime);

        // set the duration end trigger if we've changed from bursting to not bursting
        bool endOut = false;
        if (prevBursting && !bursting)
        {
            pgEnd.trigger(1e-3f);
        }
        else
            endOut = pgEnd.process(args.sampleTime);

        // finally set the outputs as required
        outputs[PULSES_OUTPUT].setVoltage(boolToGate(bursting && prob && gpClock.high()));
        outputs[DURATION_OUTPUT].setVoltage(boolToGate(bursting));
        outputs[START_OUTPUT].setVoltage(boolToGate(startOut));
        outputs[END_OUTPUT].setVoltage(boolToGate(endOut));

        // blink the clock light according to the clock rate
        lights[CLOCK_LIGHT].setSmoothBrightness(gpClock.light(), args.sampleTime);

        // save bursting state for next step
        prevBursting = bursting;
    }
};
struct RSW : SvgSwitch
{
    RSW()
    {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/TLburst6.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/TLburst7.svg")));
        shadow->opacity = 0.0f;
    }
};
struct RSW2 : SvgSwitch
{
    RSW2()
    {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/TLburst2.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/TLburst3.svg")));
        shadow->opacity = 0.0f;
    }
};
struct BT1 : SvgSwitch
{
    BT1()
    {
        momentary = true;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/TLburst4.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/TLburst5.svg")));
        shadow->opacity = 0.0f;
    }
};

struct FlammWidget : ModuleWidget
{

    std::string panelName;

    FlammWidget(Flamm *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpFLAMM.svg")));

        // controls
        addInput(createInputCentered<SPCLK>(Vec(15, 23 * 1), module, Flamm::CLOCK_INPUT));
        addParam(createParamCentered<TrimpotW>(Vec(15, 23 * 2), module, Flamm::RATE_PARAM));
        addChild(createLightCentered<SmallLight<BlueLight>>(Vec(15, 23 * 2.8), module, Flamm::CLOCK_LIGHT));
        addInput(createInputCentered<SPCVR>(Vec(15, 23 * 3.5), module, Flamm::RATECV_INPUT));
        addParam(createParamCentered<RSW>(Vec(15, 23 * 4.3), module, Flamm::RANGE_PARAM));
        addParam(createParamCentered<TrimpotRs>(Vec(15, 23 * 5.3), module, Flamm::PULSES_PARAM));
        addInput(createInputCentered<SPCVN>(Vec(15, 23 * 6.3), module, Flamm::PULSESCV_INPUT));
        addParam(createParamCentered<TrimpotG>(Vec(15, 23 * 7.5), module, Flamm::PROBABILITY_PARAM));
        addInput(createInputCentered<SPCVP>(Vec(15, 23 * 8.5), module, Flamm::PROBABILITYCV_INPUT));
        addParam(createParamCentered<RSW2>(Vec(15, 23 * 9.3), module, Flamm::RETRIGGER_PARAM));
        addInput(createInputCentered<SPTRG>(Vec(15, 23 * 10.1), module, Flamm::TRIGGER_INPUT));
        addOutput(createOutputCentered<SPOUT>(Vec(15, 23 * 11.2), module, Flamm::PULSES_OUTPUT));
        addOutput(createOutputCentered<SPSOC>(Vec(15, 23 * 12.5), module, Flamm::START_OUTPUT));
        addOutput(createOutputCentered<SPDUR>(Vec(15, 23 * 13.5), module, Flamm::DURATION_OUTPUT));
        addOutput(createOutputCentered<SPEOC>(Vec(15, 23 * 14.5), module, Flamm::END_OUTPUT));
        addParam(createParamCentered<BT1>(Vec(15, 23 * 15.8), module, Flamm::MANUAL_PARAM));
    }
};

Model *modelFlamm = createModel<Flamm, FlammWidget>("Flamm");