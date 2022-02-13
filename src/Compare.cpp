#include "plugin.hpp"

struct Compare : Module
{
    enum ParamIds
    {
        THRESHOLD_PARAM,
        THRESHOLD_PARAM2,
        REMOVE_EMPTY,
        REMOVE_EMPTY2,
        NUM_PARAMS
    };
    enum InputIds
    {
        THRESHOLD_INPUT,
        THRESHOLD_INPUT2,
        COMPARE_INPUT,
        COMPARE_INPUT2,
        NUM_INPUTS
    };
    enum OutputIds
    {
        COMPARE_OUTPUT,
        COMPARE_OUTPUT2,
        INV_OUTPUT,
        INV_OUTPUT2,
        UP_OUTPUT,
        UP_OUTPUT2,
        DN_OUTPUT,
        DN_OUTPUT2,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        OVER_LIGHT,
        OVER_LIGHT2,
        UNDER_LIGHT,
        UNDER_LIGHT2,
        ENUMS(CV_LIGHT, 4 * 2),
        NUM_LIGHTS
    };

    dsp::ClockDivider lightDivider;
    bool state = 0;
    bool state2 = 0;

    Compare()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THRESHOLD_PARAM, -10.0, 10.0, 0.0, "Threshold", " v");
        configParam(THRESHOLD_PARAM2, -10.0, 10.0, 0.0, "Threshold", " v");
        configSwitch(REMOVE_EMPTY, 0.0, 1.0, 0.0, "Sort Poly out by removing empty Channels", {"NO", "YES"});
        configSwitch(REMOVE_EMPTY2, 0.0, 1.0, 0.0, "Sort Poly out by removing empty Channels", {"NO", "YES"});
        configInput(THRESHOLD_INPUT, "Threshold CV 1");
        configInput(THRESHOLD_INPUT2, "Threshold CV 2");
        configInput(COMPARE_INPUT, "(Poly or Mono) Signal to Compare 1");
        configInput(COMPARE_INPUT2, "(Poly or Mono) Signal to Compare 2");
        configOutput(COMPARE_OUTPUT, "Signal Above Threshold (Gate) 1");
        configOutput(COMPARE_OUTPUT2, "Signal Above Threshold (Gate) 2");
        configOutput(INV_OUTPUT, "Signal Below Threshold (Gate) 1");
        configOutput(INV_OUTPUT2, "Signal Below Threshold (Gate) 2");
        configOutput(UP_OUTPUT, "Signal Above Threshold (Voltage) 1");
        configOutput(UP_OUTPUT2, "Signal Above Threshold (Voltage) 2");
        configOutput(DN_OUTPUT, "Signal Below Threshold (Voltage) 1");
        configOutput(DN_OUTPUT2, "Signal Below Threshold (Voltage) 2");
        configLight(OVER_LIGHT, "(Monophonic) Above Threshold 1");
        configLight(OVER_LIGHT2, "(Monophonic) Above Threshold 2");
        configLight(UNDER_LIGHT, "(Monophonic) Below Threshold 1");
        configLight(UNDER_LIGHT2, "(Monophonic) Below Threshold 2");
        lightDivider.setDivision(16);
    }

    void process(const ProcessArgs &args) override
    {

        // Compute the threshold from the threshold parameter and input
        int count1 = -1;
        int counta = 0;
        int count2 = -1;
        int countb = 0;
        int count3 = -1;
        int countc = 0;
        int count4 = -1;
        int countd = 0;
        float compare;
        float compare2;
        float threshold = params[THRESHOLD_PARAM].getValue();
        float threshold2 = params[THRESHOLD_PARAM2].getValue();
        threshold += inputs[THRESHOLD_INPUT].getVoltage();
        threshold2 += inputs[THRESHOLD_INPUT2].getVoltage();
        float chnr_a = inputs[COMPARE_INPUT].getChannels();
        float chnr_b = inputs[COMPARE_INPUT2].getChannels();
        float deltaTime = args.sampleTime * lightDivider.getDivision();
        float v = 0.f;
        float w = 0.f;
        for (int i = 0; i < chnr_a; i++)
        {
            compare = inputs[COMPARE_INPUT].getVoltage(i);
            if (params[REMOVE_EMPTY].getValue() == 1)
            {
                // compare
                state = (compare > threshold);
                if (state)
                {
                    count1++;
                    counta++;
                    if (countb == chnr_a)
                    {
                        counta = 0;
                        count1++;
                    }
                    // set the output high
                    outputs[COMPARE_OUTPUT].setVoltage(10.0f, count1);
                    outputs[UP_OUTPUT].setVoltage(compare, count1);
                    //outputs[DN_OUTPUT].setVoltage(0.0f, count1);
                    //outputs[INV_OUTPUT].setVoltage(0.0f, count1);
                    outputs[COMPARE_OUTPUT].setChannels(counta);
                    outputs[UP_OUTPUT].setChannels(counta);
                    // Set light off
                    lights[OVER_LIGHT].setBrightness(1.0f);
                    lights[UNDER_LIGHT].setBrightness(0.0f);

                    if (inputs[COMPARE_INPUT].isConnected())
                        v = outputs[UP_OUTPUT].getVoltage() / 10.f;
                    else
                        v = 0.f;
                    lights[CV_LIGHT + 2 * 2 + 0].setSmoothBrightness(v, deltaTime);
                    lights[CV_LIGHT + 2 * 2 + 1].setSmoothBrightness(-v, deltaTime);

                    onReset();
                }
                else
                {
                    count2++;
                    countb++;
                    if (counta == chnr_a)
                    {
                        countb = 0;
                        count2++;
                    }
                    // set the output low
                    //outputs[COMPARE_OUTPUT].setVoltage(0.0f, count2);
                    outputs[DN_OUTPUT].setVoltage(compare, count2);
                    outputs[INV_OUTPUT].setVoltage(10.0f, count2);
                    //outputs[UP_OUTPUT].setVoltage(0.0f, count2);
                    outputs[INV_OUTPUT].setChannels(countb);
                    outputs[DN_OUTPUT].setChannels(countb);
                    // Set light off
                    lights[UNDER_LIGHT].setBrightness(1.0f);
                    lights[OVER_LIGHT].setBrightness(0.0f);

                    if (inputs[COMPARE_INPUT].isConnected())
                        w = outputs[DN_OUTPUT].getVoltage() / 10.f;
                    else
                        w = 0.f;
                    lights[CV_LIGHT + 4 * 2 + 0].setSmoothBrightness(w, deltaTime);
                    lights[CV_LIGHT + 4 * 2 + 1].setSmoothBrightness(-w, deltaTime);

                    onReset();
                }
            }
            if (params[REMOVE_EMPTY].getValue() == 0)
            {
                outputs[COMPARE_OUTPUT].setChannels(chnr_a);
                outputs[UP_OUTPUT].setChannels(chnr_a);
                outputs[INV_OUTPUT].setChannels(chnr_a);
                outputs[DN_OUTPUT].setChannels(chnr_a);

                state = (compare > threshold);
                if (state)
                {
                    // set the output high
                    outputs[COMPARE_OUTPUT].setVoltage(10.0f, i);
                    outputs[UP_OUTPUT].setVoltage(compare, i);
                    outputs[DN_OUTPUT].setVoltage(0.0f, i);
                    outputs[INV_OUTPUT].setVoltage(0.0f, i);
                    // Set light off
                    lights[OVER_LIGHT].setBrightness(1.0f);
                    lights[UNDER_LIGHT].setBrightness(0.0f);

                    if (inputs[COMPARE_INPUT].isConnected())
                        v = outputs[UP_OUTPUT].getVoltage() / 10.f;
                    else
                        v = 0.f;
                    lights[CV_LIGHT + 2 * 2 + 0].setSmoothBrightness(v, deltaTime);
                    lights[CV_LIGHT + 2 * 2 + 1].setSmoothBrightness(-v, deltaTime);
                }
                else
                {
                    // set the output low
                    outputs[COMPARE_OUTPUT].setVoltage(0.0f, i);
                    outputs[DN_OUTPUT].setVoltage(compare, i);
                    outputs[INV_OUTPUT].setVoltage(10.0f, i);
                    outputs[UP_OUTPUT].setVoltage(0.0f, i);
                    // Set light off
                    lights[UNDER_LIGHT].setBrightness(1.0f);
                    lights[OVER_LIGHT].setBrightness(0.0f);

                    if (inputs[COMPARE_INPUT].isConnected())
                        w = outputs[DN_OUTPUT].getVoltage() / 10.f;
                    else
                        w = 0.f;
                    lights[CV_LIGHT + 4 * 2 + 0].setSmoothBrightness(w, deltaTime);
                    lights[CV_LIGHT + 4 * 2 + 1].setSmoothBrightness(-w, deltaTime);

                    onReset();
                }
            }
        }
        for (int k = 0; k < chnr_b; k++)
        {
            compare2 = inputs[COMPARE_INPUT2].getVoltage(k);
            if (params[REMOVE_EMPTY2].getValue() == 1)
            {
                // compare
                state2 = (compare2 > threshold2);
                if (state2)
                {
                    count3++;
                    countc++;
                    if (countd == chnr_b)
                    {
                        countc = 0;
                    }
                    // set the output high
                    outputs[COMPARE_OUTPUT2].setVoltage(10.0f, count3);
                    outputs[UP_OUTPUT2].setVoltage(compare2, count3);
                    //outputs[DN_OUTPUT].setVoltage(0.0f, count1);
                    //outputs[INV_OUTPUT].setVoltage(0.0f, count1);
                    outputs[COMPARE_OUTPUT2].setChannels(countc);
                    outputs[UP_OUTPUT2].setChannels(countc);
                    // Set light off
                    lights[OVER_LIGHT2].setBrightness(1.0f);
                    lights[UNDER_LIGHT2].setBrightness(0.0f);

                    if (inputs[COMPARE_INPUT2].isConnected())
                        v = outputs[UP_OUTPUT2].getVoltage() / 10.f;
                    else
                        v = 0.f;
                    lights[CV_LIGHT + 2 * 2 + 0].setSmoothBrightness(v, deltaTime);
                    lights[CV_LIGHT + 2 * 2 + 1].setSmoothBrightness(-v, deltaTime);
                }
                else
                {
                    count4++;
                    countd++;
                    if (countc == chnr_b)
                    {
                        countd = 0;
                    }
                    // set the output low
                    //outputs[COMPARE_OUTPUT].setVoltage(0.0f, count2);
                    outputs[DN_OUTPUT2].setVoltage(compare2, count4);
                    outputs[INV_OUTPUT2].setVoltage(10.0f, count4);
                    //outputs[UP_OUTPUT].setVoltage(0.0f, count2);
                    outputs[INV_OUTPUT2].setChannels(countd);
                    outputs[DN_OUTPUT2].setChannels(countd);
                    // Set light off
                    lights[UNDER_LIGHT2].setBrightness(1.0f);
                    lights[OVER_LIGHT2].setBrightness(0.0f);

                    if (inputs[COMPARE_INPUT2].isConnected())
                        w = outputs[DN_OUTPUT2].getVoltage() / 10.f;
                    else
                        w = 0.f;
                    lights[CV_LIGHT + 4 * 2 + 0].setSmoothBrightness(w, deltaTime);
                    lights[CV_LIGHT + 4 * 2 + 1].setSmoothBrightness(-w, deltaTime);
                }
            }
            if (params[REMOVE_EMPTY2].getValue() == 0)
            {
                outputs[COMPARE_OUTPUT2].setChannels(chnr_b);
                outputs[UP_OUTPUT2].setChannels(chnr_b);
                outputs[INV_OUTPUT2].setChannels(chnr_b);
                outputs[DN_OUTPUT2].setChannels(chnr_b);

                state2 = (compare2 > threshold2);
                if (state2)
                {
                    // set the output high
                    outputs[COMPARE_OUTPUT2].setVoltage(10.0f, k);
                    outputs[UP_OUTPUT2].setVoltage(compare2, k);
                    outputs[DN_OUTPUT2].setVoltage(0.0f, k);
                    outputs[INV_OUTPUT2].setVoltage(0.0f, k);
                    // Set light off
                    lights[OVER_LIGHT2].setBrightness(1.0f);
                    lights[UNDER_LIGHT2].setBrightness(0.0f);
                    if (inputs[COMPARE_INPUT2].isConnected())
                        v = outputs[UP_OUTPUT2].getVoltage() / 10.f;
                    else
                        v = 0.f;
                    lights[CV_LIGHT + 2 * 2 + 0].setSmoothBrightness(v, deltaTime);
                    lights[CV_LIGHT + 2 * 2 + 1].setSmoothBrightness(-v, deltaTime);
                }
                else
                {
                    // set the output low
                    outputs[COMPARE_OUTPUT2].setVoltage(0.0f, k);
                    outputs[DN_OUTPUT2].setVoltage(compare2, k);
                    outputs[INV_OUTPUT2].setVoltage(10.0f, k);
                    outputs[UP_OUTPUT2].setVoltage(0.0f, k);
                    // Set light off
                    lights[UNDER_LIGHT2].setBrightness(1.0f);
                    lights[OVER_LIGHT2].setBrightness(0.0f);
                    if (inputs[COMPARE_INPUT2].isConnected())
                        w = outputs[DN_OUTPUT2].getVoltage() / 10.f;
                    else
                        w = 0.f;
                    lights[CV_LIGHT + 4 * 2 + 0].setSmoothBrightness(w, deltaTime);
                    lights[CV_LIGHT + 4 * 2 + 1].setSmoothBrightness(-w, deltaTime);
                }
            }
        }
    }
};
struct SORT : SvgSwitch
{
    SORT()
    {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtSORT0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtSORT1.svg")));
        shadow->opacity = 0.0f;
    }
};
struct CompareWidget : ModuleWidget
{

    CompareWidget(Compare *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpCOM.svg")));

        addParam(createParamCentered<TrimpotW>(Vec(15, 23 * 1), module, Compare::THRESHOLD_PARAM));
        addInput(createInputCentered<SmallPortCV>(Vec(15, 23 * 2), module, Compare::THRESHOLD_INPUT));
        addInput(createInputCentered<SmallPortC>(Vec(15, 23 * 3), module, Compare::COMPARE_INPUT));
        addParam(createParamCentered<SORT>(Vec(15, 23 * 3.75), module, Compare::REMOVE_EMPTY));
        addChild(createLightCentered<SmallLight<BlueLight>>(Vec(25, 23 * 5), module, Compare::OVER_LIGHT));
        addOutput(createOutputCentered<SmallPortUP>(Vec(15, 23 * 4.5), module, Compare::COMPARE_OUTPUT));
        addOutput(createOutputCentered<SmallPortDN>(Vec(15, 23 * 5.5), module, Compare::INV_OUTPUT));
        addChild(createLightCentered<SmallLight<BlueLight>>(Vec(25, 23 * 6), module, Compare::UNDER_LIGHT));
        //addChild(createLightCentered<SmallLight<WhiteBlueLight>>(Vec(25, 23 * 7), module, Compare::CV_LIGHT + 0));
        addOutput(createOutputCentered<SmallPortUP2>(Vec(15, 23 * 6.5), module, Compare::UP_OUTPUT));
        addOutput(createOutputCentered<SmallPortDN2>(Vec(15, 23 * 7.5), module, Compare::DN_OUTPUT));
        //addChild(createLightCentered<SmallLight<WhiteBlueLight>>(Vec(25, 23 * 8), module, Compare::CV_LIGHT + 1));

        addParam(createParamCentered<TrimpotW>(Vec(15, 23 * 9), module, Compare::THRESHOLD_PARAM2));
        addInput(createInputCentered<SmallPortCV>(Vec(15, 23 * 10), module, Compare::THRESHOLD_INPUT2));
        addInput(createInputCentered<SmallPortC>(Vec(15, 23 * 11), module, Compare::COMPARE_INPUT2));
        addParam(createParamCentered<SORT>(Vec(15, 23 * 11.75), module, Compare::REMOVE_EMPTY2));
        addChild(createLightCentered<SmallLight<BlueLight>>(Vec(25, 23 * 13), module, Compare::OVER_LIGHT2));
        addOutput(createOutputCentered<SmallPortUP>(Vec(15, 23 * 12.5), module, Compare::COMPARE_OUTPUT2));
        addOutput(createOutputCentered<SmallPortDN>(Vec(15, 23 * 13.5), module, Compare::INV_OUTPUT2));
        addChild(createLightCentered<SmallLight<BlueLight>>(Vec(25, 23 * 14), module, Compare::UNDER_LIGHT2));
        //addChild(createLightCentered<SmallLight<WhiteBlueLight>>(Vec(25, 23 * 15), module, Compare::CV_LIGHT + 2));
        addOutput(createOutputCentered<SmallPortUP2>(Vec(15, 23 * 14.5), module, Compare::UP_OUTPUT2));
        addOutput(createOutputCentered<SmallPortDN2>(Vec(15, 23 * 15.5), module, Compare::DN_OUTPUT2));
        //addChild(createLightCentered<SmallLight<WhiteBlueLight>>(Vec(25, 23 * 16), module, Compare::CV_LIGHT + 3));
    }
};

Model *modelCompare = createModel<Compare, CompareWidget>("Compare");