#include "plugin.hpp"
#include "inc/PulseModifier.hpp"
#include "inc/GateProcessor.hpp"
#include "inc/Utility.hpp"

struct Seq16 : Module
{
    enum ParamIds
    {
        STEPMAX,
        ENUMS(STEPZ, 16),
        PLAYMODE,
        CVMODE,
        BGRESET,
        TESTL,
        NUM_PARAMS
    };
    enum InputIds
    {
        CLK_IN,
        POLY_IN,
        RESET_IN,
        NUM_INPUTS
    };
    enum OutputIds
    {
        GT_OUT,
        TRG_OUT,
        EOC_OUT,
        CV_OUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        ENUMS(STEP_LIGHTS, 16 * 2),
        ENUMS(CV_LIGHTS, 16 * 2),
        NUM_LIGHTS
    };

    dsp::SchmittTrigger stepTrigger;
    dsp::SchmittTrigger resTrigger;
    dsp::PulseGenerator stepTRG;
    dsp::PulseGenerator EOC;
    dsp::ClockDivider lightDivider;
    float sh = 0.000f;
    int step = -1;
    int max = 8;
    int pulseTRG = 0;
    int EOCTRG = 0;
    bool mode1 = true;
    bool mode2 = false;
    bool mode3 = false;

    Seq16()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(STEPMAX, 1.f, 16.f, 16.f, "", " Steps");
        configSwitch(PLAYMODE, 0.f, 2.f, 0.f, "Playmode", {"Forward", "Reverse", "Random"});
        for (int c = 0; c < 16; c++)
        {
            configParam(STEPZ + c, 0.f, 1.f, 0.f, "Step " + std::to_string(c + 1));
        }
        configParam(BGRESET, 0.f, 1.f, 0.f, "Manual Reset Button");
        configSwitch(CVMODE, 0.f, 1.f, 0.f, "CV mode", {"S&H grabs CV in on CLK", "Switch-style continous CV in"});
        getParamQuantity(STEPMAX)->randomizeEnabled = false;
        getParamQuantity(BGRESET)->randomizeEnabled = false;
        getParamQuantity(CVMODE)->randomizeEnabled = false;

        lightDivider.setDivision(16);
    }
    void process(const ProcessArgs &args) override
    {
        float w = 0;
        float deltaTime = args.sampleTime * lightDivider.getDivision();

        max = (params[STEPMAX].getValue() - 1);

        if (resTrigger.process(inputs[RESET_IN].getVoltage() > 0.5) || resTrigger.process(params[BGRESET].getValue() > 0.5))
        {
            step = -1;
            outputs[GT_OUT].setVoltage(0.f);
            outputs[TRG_OUT].setVoltage(0.f);
            outputs[CV_OUT].setVoltage(0.f);
        }

        if (stepTrigger.process(inputs[CLK_IN].getVoltage() > 0.5))
        {
            if (params[PLAYMODE].getValue() == 0.f)
            {
                step++;
                mode1 = true;
                mode2 = false;
                mode3 = false;
            }
            else if (params[PLAYMODE].getValue() == 1.f)
            {
                step--;
                mode2 = true;
                mode1 = false;
                mode3 = false;
            }
            else if (params[PLAYMODE].getValue() == 2.f)
            {
                step = std::floor(random::uniform() * max);
                mode3 = true;
                mode1 = false;
                mode2 = false;
            }

            if (step <= max && step >= 0 && params[STEPZ + step].getValue() > 0.5)
            {
                outputs[GT_OUT].setVoltage(10.f);
                stepTRG.trigger(1e-3f);
            }
            else if (step <= max && params[STEPZ + step].getValue() < 0.5)
            {
                outputs[GT_OUT].setVoltage(0.f);
                outputs[CV_OUT].setVoltage(0.f);
            }

            if (mode1 == true && step > max)
            {
                step = 0;
                EOC.trigger(1e-3f);
                outputs[GT_OUT].setVoltage(0.f);
                if (step == 0 && params[STEPZ + step].getValue() > 0.5)
                {
                    outputs[GT_OUT].setVoltage(10.f);
                    stepTRG.trigger(1e-3f);
                }
            }
            else if (mode2 == true && step < 0)
            {
                step = max;
                EOC.trigger(1e-3f);
                outputs[GT_OUT].setVoltage(0.f);
                if (step == max && params[STEPZ + step].getValue() > 0.5)
                {
                    outputs[GT_OUT].setVoltage(10.f);
                    stepTRG.trigger(1e-3f);
                }
            }
            if (params[CVMODE].getValue() == 0.f && params[STEPZ + step].getValue() > 0.5)
                outputs[CV_OUT].setVoltage(inputs[POLY_IN].getVoltage(step));
        }
        if (params[CVMODE].getValue() == 1.f && params[STEPZ + step].getValue() > 0.5)
            outputs[CV_OUT].setVoltage(inputs[POLY_IN].getVoltage(step));

        pulseTRG = stepTRG.process(5.0 / args.sampleRate);
        EOCTRG = EOC.process(2.0 / args.sampleRate);

        outputs[TRG_OUT].setVoltage(pulseTRG ? 10.f : 0.f);
        outputs[EOC_OUT].setVoltage(EOCTRG ? 10.f : 0.f);

        float x = 0.f;
        for (int c = 0; c < 16; c++)
        {
            if (inputs[POLY_IN].isConnected())
            {
                float fm = inputs[POLY_IN].getVoltage(c) / 10.f;
                if (c < max && fm < 0.f)
                    x = -1.f;
                else if (c < max && fm > 0.f)
                    x = 1.f;
                else if (c < max && fm == 0)
                    x = 0.f;
                if (c > max && c > 0)
                    x = 0.f;
            }

            lights[CV_LIGHTS + c * 2 + 0].setSmoothBrightness(x, deltaTime);
            lights[CV_LIGHTS + c * 2 + 1].setSmoothBrightness(-x, deltaTime);

            if (c == step)
                w = 1.f;

            else if (c > max && c > 0)
                w = -0.2f;

            else
                w = -1.f;

            lights[STEP_LIGHTS + c * 2 + 0].setSmoothBrightness(w, deltaTime);
            lights[STEP_LIGHTS + c * 2 + 1].setSmoothBrightness(-w, deltaTime);
        }
    }
};
struct SBT : SVGSwitch
{
    SBT()
    {
        momentary = false;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtSTEP0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtSTEP2.svg")));
        shadow->opacity = 0.f;
    }
};

struct CVMOD : SVGSwitch
{
    CVMOD()
    {
        momentary = false;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtCVMODE0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtCVMODE1.svg")));
        shadow->opacity = 0.f;
    }
};
struct Seq16Widget : ModuleWidget
{
    Seq16Widget(Seq16 *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpSEQ.svg")));

        addInput(createInputCentered<SPCLK>(Vec(15, 23), module, Seq16::CLK_IN));
        addInput(createInputCentered<SPCVin>(Vec(15, 23 * 2), module, Seq16::POLY_IN));
        addInput(createInputCentered<SPRES>(Vec(15, 23 * 3), module, Seq16::RESET_IN));
        addParam(createParamCentered<TrimpotWs>(Vec(15, 23 * 4), module, Seq16::STEPMAX));
        addParam(createParamCentered<PMOD>(Vec(15, 23 * 4.7), module, Seq16::PLAYMODE));
        addParam(createParamCentered<CVMOD>(Vec(15, 23 * 5.1), module, Seq16::CVMODE));

        for (int i = 0; i < 16; i++)
        {
            addParam(createParamCentered<SBT>(Vec(15, 20 * ((i * 0.4) + 7)), module, Seq16::STEPZ + i));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(3, 20 * ((i * 0.4) + 7)), module, Seq16::STEP_LIGHTS + i * 2));
            addChild(createLightCentered<TinyLight<GreenRedLight>>(Vec(27, 20 * ((i * 0.4) + 7)), module, Seq16::CV_LIGHTS + i * 2));
        }
        addOutput(createOutputCentered<SPOUT>(Vec(15, 23 * 13), module, Seq16::CV_OUT));
        addOutput(createOutputCentered<SPGTout>(Vec(15, 23 * 14), module, Seq16::GT_OUT));
        addOutput(createOutputCentered<SPTRGout>(Vec(15, 23 * 15), module, Seq16::TRG_OUT));
        addOutput(createOutputCentered<SPEOC>(Vec(15, 23 * 16), module, Seq16::EOC_OUT));
    }
};

Model *modelSeq16 = createModel<Seq16, Seq16Widget>("Seq16");