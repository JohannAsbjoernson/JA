#include "plugin.hpp"
#include "inc/Filter6pButter.h"

struct PolyRND : Module
{
    enum ParamIds
    {
        RATE_PARAM,
        AMP_PARAM,
        CUTOFF_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        CLOCK_INPUT,
        RATE_INPUT,
        AMP_INPUT,
        CUTOFF_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        RANDOM_OUTPUT,
        NOISE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    dsp::SchmittTrigger clockTrigger;

    float dust = 0;
    float noise = 0;
    float rndClock = 0.f;
    float phase = 0.f;

    Filter6PButter filter[16];

    void noiseGen(float amplitude)
    {
        noise = 4.f * amplitude * random::normal();
        noise = clamp(noise, -5.f, 5.f);
        dust = noise;
        rndClock = noise > 0.f ? 10.f : 0.f;
        //rndClock *= amplitude;
    }
    PolyRND()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(RATE_PARAM, 0.f, 12.f, 0.03f, "Noise Rate");
        configParam(AMP_PARAM, 0.f, 5.f, 1.f, "Noise Amplitude x");
        configParam(CUTOFF_PARAM, 0.01f, 10.f, 10.f, "Cutoff", "%", 0, 10);

        configInput(CLOCK_INPUT, "external Clock/Gate/Trigger");
        configInput(RATE_INPUT, "Rate CV (internal Clk)");
        configInput(AMP_INPUT, "Amplitude CV");
        configInput(CUTOFF_INPUT, "Cutoff CV");

        configOutput(RANDOM_OUTPUT, "Random Clock");
        configOutput(NOISE_OUTPUT, "Gated Noise");
    }

    void process(const ProcessArgs &args) override
    {

        float amplitude = params[AMP_PARAM].getValue();
        float cutcv, cutof;
        float cutpara = params[CUTOFF_PARAM].getValue();

        dust = 0.f;

        // External Clock
        if (inputs[CLOCK_INPUT].isConnected())
        {
            int chans = inputs[CLOCK_INPUT].getChannels();
            for (int i = 0; i < chans; i++)
            {
                cutcv = clamp(inputs[CUTOFF_INPUT].getVoltage(i), -10.f, 10.f);
                cutof = cutpara + cutcv;
                cutof = clamp(cutof, 0.1f, 10.f);
                filter[i].setCutoffFreq(cutof / 48.f);
                amplitude += 0.1 * inputs[AMP_INPUT].getVoltage(i);
                amplitude = clamp(amplitude, -1.f, 2.f);
                if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage(i)))
                {
                    noiseGen(amplitude);
                    outputs[NOISE_OUTPUT].setVoltage(clamp(filter[i].process(dust), -5.f, 5.f), i);
                    outputs[RANDOM_OUTPUT].setVoltage(10.f, i);
                }
                else
                {
                    outputs[RANDOM_OUTPUT].setVoltage(0.f, i);
                }
            }
            outputs[NOISE_OUTPUT].setChannels(chans);
            outputs[RANDOM_OUTPUT].setChannels(chans);
        }
        // Internal Clock
        else
        {

            int chana = inputs[RATE_INPUT].getChannels();
            int chanb = inputs[AMP_INPUT].getChannels();
            int chan;
            if ((chana >= chanb && chana > 1 && chanb > 1) || (chana < chanb && chana <= 1 && chanb > 1))
                chan = chanb;
            else if ((chana > chanb && chana > 1 && chanb <= 1) || (chana < chanb && chana > 1 && chanb > 1))
                chan = chana;
            else
                chan = 1;
            outputs[NOISE_OUTPUT].setChannels(chan);
            outputs[RANDOM_OUTPUT].setChannels(chan);

            for (int j = 0; j < chan; j++)
            {
                cutcv = clamp(inputs[CUTOFF_INPUT].getVoltage(j), -10.f, 10.f);
                cutof = cutpara + cutcv;
                cutof = clamp(cutof, 0.1f, 10.f);
                filter[j].setCutoffFreq(cutof / 48.f);
                amplitude += 0.1 * inputs[AMP_INPUT].getVoltage(j);
                amplitude = clamp(amplitude, -5.f, 5.f);
                float rate = params[RATE_PARAM].getValue();
                rate += inputs[RATE_INPUT].getVoltage(j);
                rate = clamp(rate, 0.f, 12.f);
                rate = 5.f * rack::dsp::approxExp2_taylor5(rate);
                phase += rate * args.sampleTime;
                if (phase >= 0.5f)
                {
                    phase -= 1.f;
                    noiseGen(amplitude);
                }
                else if (phase <= -0.5f)
                {
                    phase += 1.f;
                }
                outputs[RANDOM_OUTPUT].setVoltage(rndClock, j);
                outputs[NOISE_OUTPUT].setVoltage(clamp(filter[j].process(dust), -5.f, 5.f), j);
            }
        }
    }
};

struct PolyRNDWidget : ModuleWidget
{
    PolyRNDWidget(PolyRND *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpRND.svg")));

        addInput(createInputCentered<SPCLK>(Vec(15, 23), module, PolyRND::CLOCK_INPUT));
        addParam(createParamCentered<TrimpotR>(Vec(15, 23 * 2), module, PolyRND::RATE_PARAM));
        addInput(createInputCentered<SPCVin>(Vec(15, 23 * 3), module, PolyRND::RATE_INPUT));
        addParam(createParamCentered<TrimpotG>(Vec(15, 23 * 4), module, PolyRND::AMP_PARAM));
        addInput(createInputCentered<SPCVN>(Vec(15, 23 * 5), module, PolyRND::AMP_INPUT));
        addParam(createParamCentered<TrimpotB>(Vec(15, 23 * 6), module, PolyRND::CUTOFF_PARAM));
        addInput(createInputCentered<SPCVin>(Vec(15, 23 * 7), module, PolyRND::CUTOFF_INPUT));
        addOutput(createOutputCentered<SmallPortTRIG>(Vec(15, 23 * 8.2), module, PolyRND::RANDOM_OUTPUT));
        addOutput(createOutputCentered<SPOUT>(Vec(15, 23 * 9.2), module, PolyRND::NOISE_OUTPUT));
    }
};

Model *modelPolyRND = createModel<PolyRND, PolyRNDWidget>("PolyRND");
