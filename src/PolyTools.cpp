#include "plugin.hpp"

struct PolyTools : Module
{
    enum ParamIds
    {
        CLONER,
        LEVEL,
        SPREAD,
        MODE,
        CHANS,
        NUM_PARAMS
    };
    enum InputIds
    {
        POLY_A,
        POLY_B,
        POLY_C,
        POLY_D,
        POLY_E,
        POLY_F,
        MONO_A,
        NUM_INPUTS
    };
    enum OutputIds
    {
        POLY_OUT_A,
        POLY_OUT_B,
        POLY_OUT_C,
        POLY_OUT_D,
        POLY_OUT_E,
        MONO_OUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    PolyTools()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(CLONER, 1.0f, 16.0f, 16.0f, "# of Clones");
        configParam(SPREAD, -10.0f, 10.0f, 0.0f, "Add", " V");
        configSwitch(MODE, 0.f, 2.f, 0.f, "Mode", {"1:1 Clone", "Offset (for all Clones)", "Fan/Spread (on each Clone)"});
        configParam(CHANS, 1.0f, 16.0f, 16.0f, "# of Channels");
        configParam(LEVEL, 0.f, 1.f, 0.1f, "Level", "%", 0.f, 100.f);

        configInput(POLY_A, "Poly Cable Joiner A");
        configInput(POLY_B, "Poly Cable Joiner B");
        configInput(POLY_C, "Poly Cable Joiner C");
        configOutput(POLY_OUT_A, "Merged Cables");

        configInput(MONO_A, "Mono Signal to Clone");
        configOutput(POLY_OUT_B, "Poly Signal Clones");

        configInput(POLY_D, "Poly Stereo Signal");
        configOutput(POLY_OUT_C, "Left Stereo Signal");
        configOutput(POLY_OUT_D, "Right Stereo Signal");

        configInput(POLY_E, "Poly Signal to Sum");
        configOutput(MONO_OUT, "Mono Signal Sum");

        configInput(POLY_F, "Poly Signal to redact");
        configOutput(POLY_OUT_E, "Redacted Poly Signal");
    }

    void process(const ProcessArgs &args) override
    {
        //Poly Joiner
        float ch1 = 0.f;
        float ch2 = 0.f;
        float ch3 = 0.f;
        float v = 0.f;
        float v2 = 0.f;

        ch1 = inputs[POLY_A].getChannels();
        ch2 = inputs[POLY_B].getChannels();
        ch3 = inputs[POLY_C].getChannels();

        v = ch1 + ch2;
        v2 = v + ch3;

        if (v >= 16)
        {
            ch2 = 16 - ch1;
            v2 = ch1 + ch2;
        }
        else if (v < 16 && v2 >= 16)
        {
            ch3 = 16 - v;
            v2 = ch1 + ch2 + ch3;
        }

        float val1;
        float val2;
        float val3;
        float val4;

        outputs[POLY_OUT_A].setChannels(v2);

        for (int i = 0; i < v2; i++)
        {
            for (int a = 0; a < ch1; a++)
            {
                if (inputs[POLY_A].isConnected())
                {
                    val1 = inputs[POLY_A].getVoltage(a);
                    outputs[POLY_OUT_A].setVoltage(val1, a);
                }
                for (int b = 0; b < ch2; b++)
                {
                    if (inputs[POLY_B].isConnected())
                    {
                        val2 = inputs[POLY_B].getVoltage(b);
                        outputs[POLY_OUT_A].setVoltage(val2, (ch1 + b));
                    }
                    for (int c = 0; c < ch3; c++)
                    {
                        if (inputs[POLY_C].isConnected())
                        {
                            val3 = inputs[POLY_C].getVoltage(c);
                            outputs[POLY_OUT_A].setVoltage(val3, ((ch1 + ch2) + c));
                        }
                    }
                }
            }
        }

        //Mono to Poly Cloner
        float clones = params[CLONER].getValue();
        float spread = params[SPREAD].getValue();
        outputs[POLY_OUT_B].setChannels(clones);

        for (int i = 0; i < clones; i++)
        {
            if (params[MODE].getValue() == 0)
                val4 = inputs[MONO_A].getVoltage();
            else if (params[MODE].getValue() == 1)
                val4 = inputs[MONO_A].getVoltage() + spread;
            else if (params[MODE].getValue() == 2)
                val4 = inputs[MONO_A].getVoltage() + (spread * (i + 1));
            outputs[POLY_OUT_B].setVoltage(clamp(val4, -10.0f, 10.0f), i);
        }

        //Poly Splitter L/R
        float chnr = inputs[POLY_D].getChannels();
        float chni = chnr / 2;
        float chno = round(chni);
        float chnu = floor(chni);

        for (int s = 0; s < chnr; s++)
        {
            if (s % 2 == 0)
            {
                float chc1 = s / 2;
                float val5 = inputs[POLY_D].getVoltage(s);
                outputs[POLY_OUT_C].setVoltage(val5, chc1);
            }
            else if (s % 2 == 1)
            {
                float chc2 = s / 2;
                float val6 = inputs[POLY_D].getVoltage(s);
                outputs[POLY_OUT_D].setVoltage(val6, chc2);
            }
        }
        outputs[POLY_OUT_C].setChannels(chno);
        outputs[POLY_OUT_D].setChannels(chnu);

        //Poly Summer to Mono Out
        float sum = inputs[POLY_E].getVoltageSum();
        sum *= params[LEVEL].getValue();
        outputs[MONO_OUT].setVoltage(sum);

        //Channel Trim/Add
        float chn2 = params[CHANS].getValue();
        outputs[POLY_OUT_E].setChannels(chn2);

        for (int f = 0; f < chn2; f++)
        {
            float val6 = inputs[POLY_F].getVoltage(f);
            outputs[POLY_OUT_E].setVoltage(val6, f);
        }
    }
};
struct BtCLONE : SVGSwitch
{
    BtCLONE()
    {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtCLONE0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtCLONE1.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtCLONE2.svg")));
        shadow->opacity = 0.f;
    }
};
struct PolyToolsWidget : ModuleWidget
{
    PolyToolsWidget(PolyTools *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpTOOL.svg")));

        {
            addInput(createInputCentered<MicroPortB>(Vec(15, 18), module, PolyTools::POLY_A));
            addInput(createInputCentered<MicroPortB>(Vec(15, 18 * 2), module, PolyTools::POLY_B));
            addInput(createInputCentered<MicroPortB>(Vec(15, 18 * 3), module, PolyTools::POLY_C));
            addOutput(createOutputCentered<MicroPortAO>(Vec(15, 18 * 4), module, PolyTools::POLY_OUT_A));

            addInput(createInputCentered<MicroPortB>(Vec(15, 18 * 5.5), module, PolyTools::MONO_A));
            addParam(createParamCentered<MTrimWs>(Vec(15, 18 * 6.5), module, PolyTools::CLONER));
            addParam(createParamCentered<MTrimR>(Vec(15, 18 * 7.5), module, PolyTools::SPREAD));
            addParam(createParamCentered<BtCLONE>(Vec(15, 18 * 8.2), module, PolyTools::MODE));
            addOutput(createOutputCentered<MicroPortAO>(Vec(15, 18 * 9), module, PolyTools::POLY_OUT_B));

            addInput(createInputCentered<MicroPortB>(Vec(15, 18 * 10.5), module, PolyTools::POLY_D));
            addOutput(createOutputCentered<MicroPortAO>(Vec(15, 18 * 11.5), module, PolyTools::POLY_OUT_C));
            addOutput(createOutputCentered<MicroPortAO>(Vec(15, 18 * 12.5), module, PolyTools::POLY_OUT_D));

            addInput(createInputCentered<MicroPortB>(Vec(15, 18 * 14), module, PolyTools::POLY_E));
            addParam(createParamCentered<MTrimW>(Vec(15, 18 * 15), module, PolyTools::LEVEL));
            addOutput(createOutputCentered<MicroPortAO>(Vec(15, 18 * 16), module, PolyTools::MONO_OUT));

            addInput(createInputCentered<MicroPortB>(Vec(15, 18 * 17.5), module, PolyTools::POLY_F));
            addParam(createParamCentered<MTrimWs>(Vec(15, 18 * 18.5), module, PolyTools::CHANS));
            addOutput(createOutputCentered<MicroPortAO>(Vec(15, 18 * 19.5), module, PolyTools::POLY_OUT_E));
        }
    }
};

Model *modelPolyTools = createModel<PolyTools, PolyToolsWidget>("PolyTools");