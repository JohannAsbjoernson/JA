#include "plugin.hpp"

struct CV2P : Module
{
    enum ParamIds
    {
        CH_PARAM,
        ENUMS(VAL_PARAM, 16),
        NUM_PARAMS
    };
    enum InputIds
    {
        TRG_A,
        MONO_A,
        CV_INPUT,
        STEPINC_INPUT,
        STEPDEC_INPUT,
        RANDOM_INPUT,
        RESET_INPUT,
        RESET2_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        POLY_OUT_A,
        POLY_OUT_B,
        MONOOUT_OUTPUT,
        TRG_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        ENUMS(VU_LIGHTS, 16 * 2),
        ENUMS(ST_LIGHTS, 16),
        NUM_LIGHTS
    };

    int count1 = 0;
    int lastChannel = 0;
    bool reset1 = false;
    dsp::ClockDivider lightDivider;
    dsp::SchmittTrigger trg_1;

    ////////////////////////////N1
    dsp::ClockDivider connectionUpdater;

    dsp::SchmittTrigger stepIncreaseTrigger;
    dsp::SchmittTrigger stepDecreaseTrigger;
    dsp::SchmittTrigger stepRandomTrigger;
    dsp::SchmittTrigger resetTrigger;
    dsp::SchmittTrigger reset2Trigger;

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
    bool reset2Connected = false;
    ////////////////////////////N1

    CV2P()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(CH_PARAM, 1.0f, 16.0f, 16.0f, "# of Channels");
        for (int i = 0; i < 16; i++)
        {
            configParam(VAL_PARAM + i, -10.0f, 10.0, 0.f);
        }
        lightDivider.setDivision(16);
    }

    void onReset() override
    {
        reset1 = true;
        activeChannel = -1;
        step = 0;
        outputs[POLY_OUT_B].setVoltage(0);

        for (int i = 0; i < 16; i++)
        {
            params[VAL_PARAM + i].setValue(0.f);
        }
    }

    void process(const ProcessArgs &args) override
    {
        int count_limit1 = params[CH_PARAM].getValue();
        outputs[POLY_OUT_A].setChannels(count_limit1);
        if (reset2Connected && reset2Trigger.process(inputs[RESET2_INPUT].getVoltage()))
        {
            reset1 = true;
            activeChannel = -1;
            step = 0;
            outputs[POLY_OUT_B].setVoltage(0);

            for (int i = 0; i < 16; i++)
            {
                params[VAL_PARAM + i].setValue(0.f);
            }
        }
        if (reset1 == false)
        {
            if (trg_1.process(inputs[TRG_A].getVoltage()) && count1 <= count_limit1)
            {
                count1++;
                //float val = inputs[MONO_A].getVoltage();
                params[VAL_PARAM + count1].setValue(inputs[MONO_A].getVoltage());
                outputs[POLY_OUT_A].setVoltage(params[VAL_PARAM + (count1)].getValue(), count1 - 1);
            }
            for (int i = 0; i <= count_limit1; i++)
            {
                outputs[POLY_OUT_B].setVoltage(params[VAL_PARAM + i].getValue(), i - 1);
            }
            outputs[POLY_OUT_B].setChannels(count_limit1);
        }
        if (count1 == count_limit1)
        {
            count1 = 0;
        }
        ////////////////////////////////// N1
        int chnr = params[CH_PARAM].getValue();
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
            outputs[MONOOUT_OUTPUT].setVoltage(outputs[POLY_OUT_B].getVoltage(activeChannel));
        }
        trg_pulse1 = triggerPulse1.process(2.0 / args.sampleRate);
        outputs[TRG_OUTPUT].setVoltage((trg_pulse1 ? 10.0f : 0.0f));

        lastChannel = params[CH_PARAM].getValue();
        float deltaTime = args.sampleTime * lightDivider.getDivision();

        for (int c = 0; c < 16; c++)
        {
            float v = outputs[POLY_OUT_B].getVoltage(c) / 10.f;
            lights[VU_LIGHTS + c * 2 + 0].setSmoothBrightness(v, deltaTime);
            lights[VU_LIGHTS + c * 2 + 1].setSmoothBrightness(-v, deltaTime);
        }
        float w = 0;
        for (int c = 0; c < 16; c++)
        {
            if (c == step)
                w = 1.f;
            else
                w = 0.f;
            lights[ST_LIGHTS + c].setSmoothBrightness(w, deltaTime);
        }
    }
    void updateConnections()
    {
        cvConnected = inputs[CV_INPUT].isConnected();
        increaseConnected = inputs[STEPINC_INPUT].isConnected();
        decreaseConnected = inputs[STEPDEC_INPUT].isConnected();
        randomConnected = inputs[RANDOM_INPUT].isConnected();
        resetConnected = inputs[RESET_INPUT].isConnected();
    }
    json_t *dataToJson() override
    {
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "step", json_integer(step));
        return rootJ;
    }
    void dataFromJson(json_t *rootJ) override
    {
        json_t *stepJ = json_object_get(rootJ, "step");
        if (stepJ)
        {
            step = json_integer_value(stepJ);
        }
    }
    //////////////////////////N1
};
struct CV2PWidget : ModuleWidget
{
    CV2PWidget(CV2P *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpCV2P.svg")));

        {
            addInput(createInputCentered<SmallPortC>(Vec(15, 23), module, CV2P::MONO_A));
            addInput(createInputCentered<SPTRG>(Vec(15, 23 * 2), module, CV2P::TRG_A));
            addParam(createParamCentered<TrimpotWs>(Vec(15, 23 * 3), module, CV2P::CH_PARAM));
            //addOutput(createOutputCentered<MicroPortAO>(Vec(15, 23 * 5), module, CV2P::POLY_OUT_A));
            addOutput(createOutputCentered<MicroPortAO>(Vec(15, 23 * 4), module, CV2P::POLY_OUT_B));

            int x1 = 2.5;
            int y1 = 23 * 4.5;
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 1, 5.00 + y1), module, CV2P::VU_LIGHTS + 0 * 2));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 2, 5.00 + y1), module, CV2P::VU_LIGHTS + 1 * 2));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 3, 5.00 + y1), module, CV2P::VU_LIGHTS + 2 * 2));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 4, 5.00 + y1), module, CV2P::VU_LIGHTS + 3 * 2));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 1, 10.0 + y1), module, CV2P::VU_LIGHTS + 4 * 2));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 2, 10.0 + y1), module, CV2P::VU_LIGHTS + 5 * 2));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 3, 10.0 + y1), module, CV2P::VU_LIGHTS + 6 * 2));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 4, 10.0 + y1), module, CV2P::VU_LIGHTS + 7 * 2));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 1, 15.0 + y1), module, CV2P::VU_LIGHTS + 8 * 2));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 2, 15.0 + y1), module, CV2P::VU_LIGHTS + 9 * 2));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 3, 15.0 + y1), module, CV2P::VU_LIGHTS + 10 * 2));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 4, 15.0 + y1), module, CV2P::VU_LIGHTS + 11 * 2));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 1, 20.0 + y1), module, CV2P::VU_LIGHTS + 12 * 2));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 2, 20.0 + y1), module, CV2P::VU_LIGHTS + 13 * 2));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 3, 20.0 + y1), module, CV2P::VU_LIGHTS + 14 * 2));
            addChild(createLightCentered<TinyLight<WhiteBlueLight>>(Vec(x1 + 5 * 4, 20.0 + y1), module, CV2P::VU_LIGHTS + 15 * 2));

            addInput(createInputCentered<SmallPortCV>(Vec(15, 20 + 23 * 8), module, CV2P::CV_INPUT));
            addInput(createInputCentered<SmallPortSP>(Vec(15, 20 + 23 * 9), module, CV2P::STEPINC_INPUT));
            addInput(createInputCentered<SmallPortSM>(Vec(15, 20 + 23 * 10), module, CV2P::STEPDEC_INPUT));
            addInput(createInputCentered<SmallPortRND>(Vec(15, 20 + 23 * 11), module, CV2P::RANDOM_INPUT));
            addInput(createInputCentered<SmallPortRST>(Vec(15, 20 + 23 * 12), module, CV2P::RESET_INPUT));

            int x2 = 2.5;
            int y2 = 20 + 23 * 12.5;
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 1, 5.00 + y2), module, CV2P::ST_LIGHTS + 0));
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 2, 5.00 + y2), module, CV2P::ST_LIGHTS + 1));
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 3, 5.00 + y2), module, CV2P::ST_LIGHTS + 2));
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 4, 5.00 + y2), module, CV2P::ST_LIGHTS + 3));
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 1, 10.0 + y2), module, CV2P::ST_LIGHTS + 4));
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 2, 10.0 + y2), module, CV2P::ST_LIGHTS + 5));
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 3, 10.0 + y2), module, CV2P::ST_LIGHTS + 6));
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 4, 10.0 + y2), module, CV2P::ST_LIGHTS + 7));
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 1, 15.0 + y2), module, CV2P::ST_LIGHTS + 8));
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 2, 15.0 + y2), module, CV2P::ST_LIGHTS + 9));
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 3, 15.0 + y2), module, CV2P::ST_LIGHTS + 10));
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 4, 15.0 + y2), module, CV2P::ST_LIGHTS + 11));
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 1, 20.0 + y2), module, CV2P::ST_LIGHTS + 12));
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 2, 20.0 + y2), module, CV2P::ST_LIGHTS + 13));
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 3, 20.0 + y2), module, CV2P::ST_LIGHTS + 14));
            addChild(createLightCentered<TinyLight<WhitheLight>>(Vec(x2 + 5 * 4, 20.0 + y2), module, CV2P::ST_LIGHTS + 15));

            addOutput(createOutputCentered<SmallPortSTEP>(Vec(15, 20 + 23 * 14), module, CV2P::MONOOUT_OUTPUT));
            addOutput(createOutputCentered<SmallPortTRIG>(Vec(15, 20 + 23 * 15), module, CV2P::TRG_OUTPUT));
        }
    }
};
Model *modelCV2P = createModel<CV2P, CV2PWidget>("CV2P");