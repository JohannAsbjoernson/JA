#include "plugin.hpp"

struct N1 : Module
{
    enum ParamIds
    {
        NORM,
        ENUMS(MODE, 4),
        NUM_PARAMS
    };
    enum InputIds
    {
        ENUMS(IN, 4),
        ENUMS(TR, 4),
        NUM_INPUTS
    };
    enum OutputIds
    {
        ENUMS(OUT, 4),
        POLY,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    N1()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configSwitch(NORM, 0.f, 1.f, 1.f, "Normaled Trigger input", {"off", "on (default)"});

        for (int i = 0; i < 4; i++)
        {
            configSwitch(MODE + i, 0.f, 2.f, 0.f, "Sequencing Mode", {"Forwards", "Backwards", "Random"});
            configInput(IN + i, "Takes a Polyphonic Signal (# " + std::to_string(i) + ")");
            configInput(TR + i, "Trigger/Pulse/Clock, monophonic (# " + std::to_string(i) + ")");
            configOutput(OUT + i, "Sequence (step = input channel), monophonic (# " + std::to_string(i) + ")");
        }
        configOutput(POLY, "Outputs all Sequences, Polyphonic");
    }

    dsp::ClockDivider connectionUpdater;
    dsp::SchmittTrigger stepTrigger[4];

    float norm, involt, involt2;
    int mod[4], chnr[4], step[4] = {0}, activeChannel[4] = {-1}, channels[4] = {0}, snr = 0, scount = 0;
    bool tin[4] = {false}, sin[4] = {false}, snrr[4] = {false}, trr = false;

    void updateConnections()
    {
        for (int k = 0; k < 4; k++)
        {
            tin[k] = inputs[TR + k].isConnected();
            sin[k] = inputs[IN + k].isConnected();
            if (!sin[k])
            {
                outputs[IN + k].setVoltage(0);
                outputs[POLY].setVoltage(0, k);
            }
        }
    }

    void process(const ProcessArgs &args) override
    {

        if (connectionUpdater.process())
            updateConnections();

        for (int i = 0; i < 4; i++) //loop covers the four instances of n1 sequencers on this module
        {
            mod[i] = params[MODE + i].getValue();
            chnr[i] = inputs[IN + i].getChannels();

            norm = params[NORM].getValue(); //check normalisation button on/off

            // normaling of trig ins 2, 3 & 4 to 1st trig (if no trig in is connected & normalisation turned on)
            Input *input = &inputs[TR + i];
            if (i == 1 && !input->isConnected() && norm > 0.5)
                input = &inputs[TR + 0];
            if (i == 2 && !input->isConnected() && norm > 0.5)
                input = &inputs[TR + 0];
            if (i == 3 && !input->isConnected() && norm > 0.5)
                input = &inputs[TR + 0];

            // placing the pointer 'input' into a local bool variable so .process can use it
            trr = input->getVoltage() >= 2.f;

            if (stepTrigger[i].process(trr)) // on trigger input
            {
                if (mod[i] == 0) //mode one: forwards sequence. steps are added. reset on max value.
                {
                    step[i]++;
                    if (step[i] >= chnr[i])
                        step[i] = 0;
                }
                if (mod[i] == 1) //mode two: backwards. steps are substracted. reset on min value.
                {
                    step[i]--;
                    if (step[i] < 0)
                        step[i] = chnr[i] - 1;
                }
                if (mod[i] == 2) //mode three: random. random is generated between 0 to 1 & multiplied by total channels then rounded down to next integer value.
                    step[i] = std::floor(random::uniform() * chnr[i]);
            }
            activeChannel[i] = step[i]; //after each trigger the step is the active channel

            if (chnr[i] <= 0)
            {
                activeChannel[i] = -1;
            }

            if (activeChannel[i] > -1) //if there is an active channel, add it to the output.
            {
                outputs[POLY].setChannels(4);
                outputs[OUT + i].setVoltage(inputs[IN + i].getVoltage(activeChannel[i]));
                outputs[POLY].setVoltage(inputs[IN + i].getVoltage(activeChannel[i]), i);
            }
        }
    }
};

struct N1Widget : ModuleWidget
{
    N1Widget(N1 *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpN1.svg")));

        addInput(createInputCentered<SPCLK>(Vec(15, 3 + 20 * 1), module, N1::TR + 0));
        addInput(createInputCentered<SPPOLY>(Vec(15, 3 + 20 * 2), module, N1::IN + 0));
        addParam(createParamCentered<PMOD>(Vec(15, 3 + 20 * 2.8), module, N1::MODE + 0));
        addOutput(createOutputCentered<SPOUT>(Vec(15, 3 + 20 * 3.6), module, N1::OUT + 0));

        addInput(createInputCentered<SPCLK>(Vec(15, 3 + 20 * 5.1), module, N1::TR + 1));
        addInput(createInputCentered<SPPOLY>(Vec(15, 3 + 20 * 6.1), module, N1::IN + 1));
        addParam(createParamCentered<PMOD>(Vec(15, 3 + 20 * 6.9), module, N1::MODE + 1));
        addOutput(createOutputCentered<SPOUT>(Vec(15, 3 + 20 * 7.7), module, N1::OUT + 1));

        addInput(createInputCentered<SPCLK>(Vec(15, 3 + 20 * 9.2), module, N1::TR + 2));
        addInput(createInputCentered<SPPOLY>(Vec(15, 3 + 20 * 10.2), module, N1::IN + 2));
        addParam(createParamCentered<PMOD>(Vec(15, 3 + 20 * 11), module, N1::MODE + 2));
        addOutput(createOutputCentered<SPOUT>(Vec(15, 3 + 20 * 11.8), module, N1::OUT + 2));

        addInput(createInputCentered<SPCLK>(Vec(15, 3 + 20 * 13.3), module, N1::TR + 3));
        addInput(createInputCentered<SPPOLY>(Vec(15, 3 + 20 * 14.3), module, N1::IN + 3));
        addParam(createParamCentered<PMOD>(Vec(15, 3 + 20 * 15.1), module, N1::MODE + 3));
        addOutput(createOutputCentered<SPOUT>(Vec(15, 3 + 20 * 15.9), module, N1::OUT + 3));

        addOutput(createOutputCentered<SPOUT>(Vec(15, 3 + 20 * 17.4), module, N1::POLY));
        addParam(createParamCentered<BtNORM>(Vec(15, 3 + 20 * 18.2), module, N1::NORM));
    }
};

Model *modelN1 = createModel<N1, N1Widget>("N1");