#include "plugin.hpp"
#include "inc/GateProcessor.hpp"
#include "inc/Utility.hpp"

#include <string>
#include <sstream>
#include <iomanip>

struct PolyMorph : Module
{
    enum ParamIds
    {
        MORPH,
        MORPH2,
        MORPH3,
        MORPHMOD,
        MORPHMOD2,
        MORPHMOD3,
        NUM_PARAMS
    };
    enum InputIds
    {
        MORPHIN,
        MORPHIN1,
        MORPHIN2,
        MORPHIN3,
        MORPHCV,
        MORPHCV2,
        MORPHCV3,
        NUM_INPUTS
    };
    enum OutputIds
    {
        MORPHOUT,
        MORPHOUT2,
        MORPHOUT3,
        NUM_OUTPUTS
    };

    enum LighIds
    {
        NUM_LIGHTS
    };

    dsp::SchmittTrigger rtrig[16];

    PolyMorph()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(MORPH, 0.f, 1.f, 0.0f, "Morph between Inputs", "%", 0, 100);
        configParam(MORPH2, 0.f, 1.f, 0.5f, "Morph between Inputs", "%", 0, 2, -1);
        configParam(MORPH3, 0.f, 1.f, 0.0f, "Morph between Inputs", "%", 0, 100);
        configInput(MORPHCV, "Morph CV");
        configInput(MORPHCV2, "Morph CV");
        configInput(MORPHCV3, "Morph CV");
        configSwitch(MORPHMOD, 0.f, 1.f, 0.f, "Mode: ", {"Audio (-5/5)", "Voltages (-10/10)"});
        configSwitch(MORPHMOD2, 0.f, 1.f, 0.f, "Mode: ", {"Audio (-5/5)", "Voltages (-10/10)"});
        configSwitch(MORPHMOD3, 0.f, 1.f, 0.f, "Mode: ", {"Audio (-5/5)", "Voltages (-10/10)"});
        configInput(MORPHIN, "Any Polyphonic Signal");
        configInput(MORPHIN1, "A: Any Poly- or Monophonic Signal");
        configInput(MORPHIN2, "B: Any Poly- or Monophonic Signal");
        configInput(MORPHIN3, "Any Polyphonic Signal");
        configOutput(MORPHOUT, "Monophonic Morph");
        configOutput(MORPHOUT2, "Polyphonic Morph of A&B");
        configOutput(MORPHOUT3, "Monophonic Morph");
    }
    double mina, minb, min1, min2, minc, mind;
    float mfade, mfade2, mfade3, mout, mout2, mout3, mcv, mcv2, mcv3, mch, mch1, mch2, mch3, mch4;
    float mfadeA, mfadeB, mdivA, mdivB;

    void process(const ProcessArgs &args) override
    {
        if (inputs[MORPHIN].isConnected())
        {
            mch = inputs[MORPHIN].getChannels();
            mcv = clamp(inputs[MORPHCV].getVoltage(), -10.f, 10.f);
            mdivA = 1 / mch;
            mfadeA = (params[MORPH].getValue() + mcv / 10.0f);
            mfade = clamp(params[MORPH].getValue() + mcv / 10.0f, 0.0f, (mch - 1));
            mfade *= (mch - 1);
            mina = inputs[MORPHIN].getVoltage(floor(mfade));
            minb = inputs[MORPHIN].getVoltage((floor(mfade + 1)));
            mout = crossfade(mina, minb, mfadeA);
            if (params[MORPHMOD].getValue() < 0.5)
            {
                mout *= 0.3;
                mout = clamp(mout, -5.f, 5.f);
            }
            else
                mout = clamp(mout, -10.f, 10.f);
            outputs[MORPHOUT].setVoltage(mout);
        }

        if (inputs[MORPHIN1].isConnected() && inputs[MORPHIN2].isConnected())
        {
            mch1 = inputs[MORPHIN1].getChannels();
            mch2 = inputs[MORPHIN2].getChannels();
            if (mch1 >= mch2)
                mch3 = mch2;
            else
                mch3 = mch1;
            outputs[MORPHOUT2].setChannels(mch3);

            for (int i = 0; i < mch3; i++)
            {
                if (inputs[MORPHCV2].isPolyphonic())
                    mcv2 = inputs[MORPHCV2].getVoltage(i);
                else
                    mcv2 = inputs[MORPHCV2].getVoltage();

                mfade2 = clamp(params[MORPH2].getValue() + mcv2 / 10.0f, 0.0f, 1.0f);
                mfade2 *= 1.0f;

                min1 = inputs[MORPHIN1].getVoltage(i);
                min2 = inputs[MORPHIN2].getVoltage(i);
                mout2 = crossfade(min1, min2, mfade2);
                if (params[MORPHMOD2].getValue() < 0.5)
                {
                    mout2 = 0.6 * mout2;
                    mout2 = clamp(mout2, -5.f, 5.f);
                }
                else
                    mout2 = clamp(mout2, -10.f, 10.f);
                outputs[MORPHOUT2].setVoltage(mout2, i);
            }
        }

        if (inputs[MORPHIN3].isConnected())
        {
            mch4 = inputs[MORPHIN3].getChannels();
            mcv3 = inputs[MORPHCV3].getVoltage();
            mfade3 = clamp(params[MORPH3].getValue() + mcv3 / 10.0f, 0.0f, (mch4 - 1));
            mfade3 *= (mch4 - 1);
            mdivB = 1 / mch;
            mfadeB = (params[MORPH].getValue() + mcv / 10.0f);
            minc = inputs[MORPHIN3].getVoltage((floor(mfade3)));
            mind = inputs[MORPHIN3].getVoltage((floor(mfade3) + 1));
            mout3 = crossfade(minc, mind, mfadeB);
            if (params[MORPHMOD3].getValue() < 0.5)
            {
                mout3 = 0.6 * mout3;
                mout3 = clamp(mout3, -5.f, 5.f);
            }
            else
                mout3 = clamp(mout3, -10.f, 10.f);
            outputs[MORPHOUT3].setVoltage(mout3);
        }
    }
};

// Set the Panel appearance
struct PolyMorphWidget : ModuleWidget
{
    PolyMorphWidget(PolyMorph *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpMORPH.svg")));

        addInput(createInputCentered<SPPOLY>(Vec(15, 23 * 1), module, PolyMorph::MORPHIN));
        addParam(createParamCentered<TrimpotB>(Vec(15, 23 * 2), module, PolyMorph::MORPH));
        addInput(createInputCentered<SmallPortCV>(Vec(15, 23 * 3), module, PolyMorph::MORPHCV));
        addParam(createParamCentered<BtAUDIO>(Vec(15, 23 * 3.8), module, PolyMorph::MORPHMOD));
        addOutput(createOutputCentered<SPOUT>(Vec(15, 23 * 4.6), module, PolyMorph::MORPHOUT));

        addInput(createInputCentered<SmallPortBER1>(Vec(15, 23 * 6.1), module, PolyMorph::MORPHIN1));
        addInput(createInputCentered<SmallPortBER2>(Vec(15, 23 * 7.1), module, PolyMorph::MORPHIN2));
        addParam(createParamCentered<TrimpotB>(Vec(15, 23 * 8.1), module, PolyMorph::MORPH2));
        addInput(createInputCentered<SmallPortCV>(Vec(15, 23 * 9.1), module, PolyMorph::MORPHCV2));
        addParam(createParamCentered<BtAUDIO>(Vec(15, 23 * 9.9), module, PolyMorph::MORPHMOD2));
        addOutput(createOutputCentered<SPOUT>(Vec(15, 23 * 10.7), module, PolyMorph::MORPHOUT2));

        addInput(createInputCentered<SPPOLY>(Vec(15, 23 * 12.2), module, PolyMorph::MORPHIN3));
        addParam(createParamCentered<TrimpotB>(Vec(15, 23 * 13.2), module, PolyMorph::MORPH3));
        addInput(createInputCentered<SmallPortCV>(Vec(15, 23 * 14.2), module, PolyMorph::MORPHCV3));
        addParam(createParamCentered<BtAUDIO>(Vec(15, 23 * 15), module, PolyMorph::MORPHMOD3));
        addOutput(createOutputCentered<SPOUT>(Vec(15, 23 * 15.8), module, PolyMorph::MORPHOUT3));
    }
};

Model *modelPolyMorph = createModel<PolyMorph, PolyMorphWidget>("PolyMorph");