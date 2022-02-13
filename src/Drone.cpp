#include "plugin.hpp"

struct Drone : Module
{
    enum ParamId
    {
        RATE,
        DEPTH,
        VOICES,
        OCTAVE,
        PITCH,
        ENUMS(SPREAD, 8),
        SPREADBT,
        NUM_PARAMS
    };
    enum InputId
    {
        NOTEIN,
        CVIN,
        NUM_INPUTS
    };
    enum OutputId
    {
        POLYOUT,
        NUM_OUTPUTS
    };
    enum LightId
    {
        NUM_LIGHTS
    };

    const char *chrom[12] = {"c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"};

    float phase = 0.0;
    dsp::PulseGenerator gatePulse;
    dsp::PulseGenerator buttonPulse;
    float sample = 0.0;
    float out = 0.0;

    Drone()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(RATE, 0.0, 1.0, 0.3f, "Rate (of movement)");
        configParam(DEPTH, 0.0, 1.0, 0.5, "Depth (of movement)");
        configParam(VOICES, 1.0, 8.0, 4.0, "Number of Voices");
        configParam(OCTAVE, 0.0, 8.0, 3.0, "Basepitch: Octave");
        configParam(PITCH, 0.0, 11.0, 0.0, "Basepitch: Semitone");

        configParam(SPREAD + 0, -8.f, 8.f, 0.f, "Spread (voice #1)", " semitones");
        configParam(SPREAD + 1, -8.f, 8.f, -2.f, "Spread (voice #2)", " semitones");
        configParam(SPREAD + 2, -8.f, 8.f, 2.f, "Spread (voice #3)", " semitones");
        configParam(SPREAD + 3, -8.f, 8.f, -4.f, "Spread (voice #4)", " semitones");
        configParam(SPREAD + 4, -8.f, 8.f, 4.f, "Spread (voice #5)", " semitones");
        configParam(SPREAD + 5, -8.f, 8.f, -6.f, "Spread (voice #6)", " semitones");
        configParam(SPREAD + 6, -8.f, 8.f, 6.f, "Spread (voice #7)", " semitones");
        configParam(SPREAD + 7, -8.f, 8.f, -3.f, "Spread (voice #8)", " semitones");
    }

    void process(const ProcessArgs &args) override
    {

        const float slewMin = 0.1;
        const float slewMax = 10000.f;

        const float shapeScale = 1 / 10.f;

        float voc = (params[VOICES].getValue());
        float reyt = (params[RATE].getValue());
        float range = (params[DEPTH].getValue());

        float sm = 0.08333333333;
        float oct = (params[OCTAVE].getValue() - 3.0);
        float pitch = (params[PITCH].getValue() * sm);
        float notes;

        if (inputs[NOTEIN].isConnected())
            notes = clamp(inputs[NOTEIN].getVoltage(), -3.f, 6.f);
        else
            notes = oct + pitch;

        float noise = 4.0 * random::normal();
        bool nextStep = false;
        float clockTime = powf(2.0, (reyt * 30.0));
        phase += clockTime / args.sampleRate;

        if (phase >= 1.0)
        {
            phase -= 1.0;
            nextStep = true;
        }

        if (nextStep)
        {
            gatePulse.trigger(0.01);
        }

        bool gpulse = gatePulse.process(1.0 / args.sampleRate);

        float sandh = (gpulse ? 10.0 : 0.0);

        if (sandh)
        {
            sample = noise;
        }

        float in = sample;

        if (in > out)
        {
            float rise = (1 - (((params[RATE].getValue()) * 0.21) + 0.3));
            float slew = slewMax * std::pow(slewMin / slewMax, rise);
            out += slew * crossfade(1.f, shapeScale * (in - out), 1.0) * args.sampleTime;
            if (out > in)
                out = in;
        }

        else if (in < out)
        {
            float fall = (1 - (((params[RATE].getValue()) * 0.21) + 0.3));
            float slew = slewMax * std::pow(slewMin / slewMax, fall);
            out -= slew * crossfade(1.f, shapeScale * (out - in), 1.0) * args.sampleTime;
            if (out < in)
                out = in;
        }
        for (int i = 0; i < voc; i++)
        {
            float space = (params[SPREAD + i].getValue() * sm);
            float voct = (notes + space);
            outputs[POLYOUT].setChannels(i + 1);
            outputs[POLYOUT].setVoltage(voct + (out * (range * 0.018)), i);
        }
    }
};

struct DroneWidget : ModuleWidget
{
    DroneWidget(Drone *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpDRONE.svg")));

        //addInput(createInput<SPCVin>(Vec(15, 23 * 1), module, Drone::CVIN1));
        addParam(createParamCentered<TrimpotW>(Vec(15, 23 * 1), module, Drone::RATE));
        addParam(createParamCentered<TrimpotW>(Vec(15, 23 * 2), module, Drone::DEPTH));
        addParam(createParamCentered<TrimpotMs>(Vec(15, 23 * 3.3), module, Drone::OCTAVE));
        addParam(createParamCentered<TrimpotMs>(Vec(15, 23 * 4.3), module, Drone::PITCH));
        addParam(createParamCentered<TrimpotROs>(Vec(15, 23 * 5.5), module, Drone::VOICES));

        for (int i = 0; i < 8; i++)
        {
            if (i % 2 == 0)
                addParam(createParamCentered<MTrimWs>(Vec(15, 19 * (i + 8.5)), module, Drone::SPREAD + i));

            else if (i % 2 == 1)
            {
                //int s = i - 1;
                addParam(createParamCentered<MTrimGrays>(Vec(15, 19 * (i + 8.5)), module, Drone::SPREAD + i));
            }
        }

        addInput(createInputCentered<SPCVin>(Vec(15, 23 * 14), module, Drone::NOTEIN));
        addOutput(createOutputCentered<SPOUT>(Vec(15, 23 * 15.4), module, Drone::POLYOUT));
    }
};

Model *modelDrone = createModel<Drone, DroneWidget>("Drone");