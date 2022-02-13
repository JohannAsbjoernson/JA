#include "plugin.hpp"

struct PolySwitch : Module
{
    enum ParamIds
    {
        TRG,
        NUM_PARAMS
    };
    enum InputIds
    {
        IN_T,
        IN_A,
        IN_B,
        IN_C,
        IN_D,
        IN_E,
        IN_F,
        NUM_INPUTS
    };
    enum OutputIds
    {
        OUT_A,
        OUT_B,
        OUT_C,
        OUT_D,
        OUT_E,
        OUT_F,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        LED_A,
        LED_B,
        NUM_LIGHTS
    };

    int nil[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    PolySwitch()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configButton(TRG, "Manual Trigger Button");
        configBypass(IN_B, OUT_A), configBypass(IN_D, OUT_B), configBypass(IN_E, OUT_C), configBypass(IN_F, OUT_E);

        configInput(IN_T, "Poly TRG/CLK/GT (global)");
        configInput(IN_A, "Switch 1 (2:1): A");
        configInput(IN_B, "Switch 1 (2:1): B");
        configInput(IN_C, "Switch 2 (2:1): A");
        configInput(IN_D, "Switch 2 (2:1): B");
        configInput(IN_E, "Switch 3 (1:2): <-");
        configInput(IN_F, "Switch 4 (1:2): <-");

        configOutput(OUT_A, "Switch 1 (2:1): ->");
        configOutput(OUT_B, "Switch 2 (2:1): ->");
        configOutput(OUT_C, "Switch 3 (1:2): A");
        configOutput(OUT_D, "Switch 3 (1:2): B");
        configOutput(OUT_E, "Switch 4 (1:2): A");
        configOutput(OUT_F, "Switch 4 (1:2): B");

        configLight(LED_A, "A");
        configLight(LED_B, "B");
    }

    void process(const ProcessArgs &args) override
    {
        int cha, chb, chc, chd, che, chf, cht;
        cha = inputs[IN_A].getChannels(), chb = inputs[IN_B].getChannels(), chc = inputs[IN_C].getChannels();
        chd = inputs[IN_D].getChannels(), che = inputs[IN_E].getChannels(), chf = inputs[IN_F].getChannels();

        if (inputs[IN_T].isConnected())
            cht = inputs[IN_T].getChannels();
        else
            cht = 1;

        for (int c = 0; c < cht; c++)
        {
            if ((inputs[IN_T].isConnected() && inputs[IN_T].getVoltage(c) > 1.f) || params[TRG].getValue() > 0.5)
            {
                lights[LED_A].setBrightness(1.0f), lights[LED_B].setBrightness(0.0f);

                if (cht == 1)
                {
                    outputs[OUT_A].setChannels(cha), outputs[OUT_B].setChannels(chc), outputs[OUT_C].setChannels(che), outputs[OUT_E].setChannels(chf);
                    outputs[OUT_D].setChannels(che), outputs[OUT_F].setChannels(chf);
                    outputs[OUT_A].writeVoltages(inputs[IN_A].getVoltages());
                    outputs[OUT_B].writeVoltages(inputs[IN_C].getVoltages());
                    outputs[OUT_C].writeVoltages(inputs[IN_E].getVoltages());
                    outputs[OUT_E].writeVoltages(inputs[IN_F].getVoltages());
                    for (int i = 0; i < che; i++)
                        outputs[OUT_D].setVoltage(nil[i], i);
                    for (int i = 0; i < chf; i++)
                        outputs[OUT_F].setVoltage(nil[i], i);
                }
                else
                {
                    outputs[OUT_A].setChannels(cht), outputs[OUT_B].setChannels(cht), outputs[OUT_C].setChannels(cht), outputs[OUT_E].setChannels(cht);
                    outputs[OUT_D].setChannels(cht), outputs[OUT_F].setChannels(cht);
                    outputs[OUT_A].setVoltage(inputs[IN_A].getPolyVoltage(c), c);
                    outputs[OUT_B].setVoltage(inputs[IN_C].getPolyVoltage(c), c);
                    outputs[OUT_C].setVoltage(inputs[IN_E].getPolyVoltage(c), c);
                    outputs[OUT_E].setVoltage(inputs[IN_F].getPolyVoltage(c), c);
                    outputs[OUT_D].setVoltage(nil[c], c);
                    outputs[OUT_F].setVoltage(nil[c], c);
                }
            }
            else
            {
                lights[LED_B].setBrightness(1.0f), lights[LED_A].setBrightness(0.0f);

                if (cht == 1)
                {
                    outputs[OUT_A].setChannels(chb), outputs[OUT_B].setChannels(chd), outputs[OUT_D].setChannels(che), outputs[OUT_F].setChannels(chf);
                    outputs[OUT_C].setChannels(che), outputs[OUT_E].setChannels(chf);

                    outputs[OUT_A].writeVoltages(inputs[IN_B].getVoltages());
                    outputs[OUT_B].writeVoltages(inputs[IN_D].getVoltages());
                    outputs[OUT_D].writeVoltages(inputs[IN_E].getVoltages());
                    outputs[OUT_F].writeVoltages(inputs[IN_F].getVoltages());
                    for (int i = 0; i < che; i++)
                        outputs[OUT_C].setVoltage(nil[i], i);
                    for (int i = 0; i < chf; i++)
                        outputs[OUT_E].setVoltage(nil[i], i);
                }
                else
                {
                    outputs[OUT_A].setChannels(cht), outputs[OUT_B].setChannels(cht), outputs[OUT_D].setChannels(cht), outputs[OUT_F].setChannels(cht);
                    outputs[OUT_C].setChannels(cht), outputs[OUT_E].setChannels(cht);

                    outputs[OUT_A].setVoltage(inputs[IN_B].getPolyVoltage(c), c);
                    outputs[OUT_B].setVoltage(inputs[IN_D].getPolyVoltage(c), c);
                    outputs[OUT_D].setVoltage(inputs[IN_E].getPolyVoltage(c), c);
                    outputs[OUT_F].setVoltage(inputs[IN_F].getPolyVoltage(c), c);
                    outputs[OUT_C].setVoltage(nil[c], c);
                    outputs[OUT_E].setVoltage(nil[c], c);
                }
            }
        }
    }
};

struct PolySwitchWidget : ModuleWidget
{
    PolySwitchWidget(PolySwitch *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpSWITCH.svg")));
        addChild(createLightCentered<SmallLight<RedLight>>(Vec(8, 23 * 0.7), module, PolySwitch::LED_A));
        addChild(createLightCentered<SmallLight<GreenLight>>(Vec(22, 23 * 0.7), module, PolySwitch::LED_B));
        addInput(createInputCentered<SmallPortPT>(Vec(15, 23 * 1.3), module, PolySwitch::IN_T));
        addParam(createParamCentered<BtTRIG>(Vec(15, 23 * 2), module, PolySwitch::TRG));
        addInput(createInputCentered<SmallPortBER1>(Vec(15, 23 * 3), module, PolySwitch::IN_A));
        addInput(createInputCentered<SmallPortBER2>(Vec(15, 23 * 4), module, PolySwitch::IN_B));
        addOutput(createOutputCentered<SmallPortPC>(Vec(15, 23 * 5.2), module, PolySwitch::OUT_A));
        addInput(createInputCentered<SmallPortBER1>(Vec(15, 23 * 6.6), module, PolySwitch::IN_C));
        addInput(createInputCentered<SmallPortBER2>(Vec(15, 23 * 7.6), module, PolySwitch::IN_D));
        addOutput(createOutputCentered<SmallPortPC>(Vec(15, 23 * 8.8), module, PolySwitch::OUT_B));
        addInput(createInputCentered<SmallPortBER1>(Vec(15, 23 * 10.1), module, PolySwitch::IN_E));
        addOutput(createOutputCentered<SmallPortC>(Vec(15, 23 * 11.2), module, PolySwitch::OUT_C));
        addOutput(createOutputCentered<SmallPortO>(Vec(15, 23 * 12.2), module, PolySwitch::OUT_D));
        addInput(createInputCentered<SmallPortBER1>(Vec(15, 23 * 13.5), module, PolySwitch::IN_F));
        addOutput(createOutputCentered<SmallPortC>(Vec(15, 23 * 14.6), module, PolySwitch::OUT_E));
        addOutput(createOutputCentered<SmallPortO>(Vec(15, 23 * 15.6), module, PolySwitch::OUT_F));
    }
};

Model *modelPolySwitch = createModel<PolySwitch, PolySwitchWidget>("PolySwitch");