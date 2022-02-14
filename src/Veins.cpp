#include "plugin.hpp"

struct Veins : Module
{
	enum ParamIds
	{
		THRESHOLD1_PARAM,
		THRESHOLD2_PARAM,
		MODE1_PARAM,
		MODE2_PARAM,
		NUM_PARAMS
	};
	enum InputIds
	{
		IN1_INPUT,
		IN2_INPUT,
		P1_INPUT,
		P2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		OUT1A_OUTPUT,
		OUT2A_OUTPUT,
		OUT1B_OUTPUT,
		OUT2B_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds
	{
		ENUMS(STATE_LIGHTS, 2 * 2),
		NUM_LIGHTS
	};

	dsp::BooleanTrigger gateTriggers[2][16];
	dsp::BooleanTrigger modeTriggers[2];
	bool modes[2] = {};
	bool outcomes[2][16] = {};

	Veins()
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int c = 0; c < 2; c++)
		{
			configParam(THRESHOLD1_PARAM + c, 0.0, 1.0, 0.5, string::f("Channel %d probability", c + 1), "%", 0, 100);
			configParam(MODE1_PARAM + c, 0.0, 1.0, 0.0, string::f("Channel %d mode", c + 1));
			configInput(IN1_INPUT + c, string::f("Channel %d", c + 1));
			configInput(P1_INPUT + c, string::f("Channel %d probability", c + 1));
			configOutput(OUT1A_OUTPUT + c, string::f("Channel %d A", c + 1));
			configOutput(OUT1B_OUTPUT + c, string::f("Channel %d B", c + 1));
		}
	}

	void process(const ProcessArgs &args) override
	{
		for (int i = 0; i < 2; i++)
		{
			// Get input
			Input *input = &inputs[IN1_INPUT + i];
			// 2nd input is normalized to 1st.
			if (i == 1 && !input->isConnected())
				input = &inputs[IN1_INPUT + 0];
			int channels = std::max(input->getChannels(), 1);

			// mode button
			if (modeTriggers[i].process(params[MODE1_PARAM + i].getValue() > 0.f))
				modes[i] ^= true;

			bool lightA = false;
			bool lightB = false;

			// Process triggers
			for (int c = 0; c < channels; c++)
			{
				bool gate = input->getVoltage(c) >= 2.f;
				if (gateTriggers[i][c].process(gate))
				{
					// trigger
					// We don't have to clamp here because the threshold comparison works without it.
					float threshold = params[THRESHOLD1_PARAM + i].getValue() + inputs[P1_INPUT + i].getPolyVoltage(c) / 10.f;
					bool toss = (random::uniform() < threshold);
					if (!modes[i])
					{
						// direct modes
						outcomes[i][c] = toss;
					}
					else
					{
						// toggle modes
						if (toss)
							outcomes[i][c] ^= true;
					}
				}

				// Output gate logic
				bool gateA = !outcomes[i][c] && (modes[i] ? true : gate);
				bool gateB = outcomes[i][c] && (modes[i] ? true : gate);

				if (gateA)
					lightA = true;
				if (gateB)
					lightB = true;

				// Set output gates
				outputs[OUT1A_OUTPUT + i].setVoltage(gateA ? 10.f : 0.f, c);
				outputs[OUT1B_OUTPUT + i].setVoltage(gateB ? 10.f : 0.f, c);
			}

			outputs[OUT1A_OUTPUT + i].setChannels(channels);
			outputs[OUT1B_OUTPUT + i].setChannels(channels);

			lights[STATE_LIGHTS + i * 2 + 1].setSmoothBrightness(lightA, args.sampleTime);
			lights[STATE_LIGHTS + i * 2 + 0].setSmoothBrightness(lightB, args.sampleTime);
		}
	}

	void onReset() override
	{
		for (int i = 0; i < 2; i++)
		{
			modes[i] = false;
			for (int c = 0; c < 16; c++)
			{
				outcomes[i][c] = false;
			}
		}
	}

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();
		json_t *modesJ = json_array();
		for (int i = 0; i < 2; i++)
		{
			json_array_insert_new(modesJ, i, json_boolean(modes[i]));
		}
		json_object_set_new(rootJ, "modes", modesJ);
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		json_t *modesJ = json_object_get(rootJ, "modes");
		if (modesJ)
		{
			for (int i = 0; i < 2; i++)
			{
				json_t *modeJ = json_array_get(modesJ, i);
				if (modeJ)
					modes[i] = json_boolean_value(modeJ);
			}
		}
	}
};

struct VeinsWidget : ModuleWidget
{
	VeinsWidget(Veins *module)
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpBER.svg")));

		addParam(createParamCentered<SmallBtBER2>(mm2px(Vec(5.08, 7.5 * 1)), module, Veins::MODE1_PARAM));
		addParam(createParamCentered<TrimpotW>(mm2px(Vec(5.08, 7.5 * 2)), module, Veins::THRESHOLD1_PARAM));
		addInput(createInputCentered<SmallPortBER3>(mm2px(Vec(5.08, 7.5 * 3)), module, Veins::P1_INPUT));

		addInput(createInputCentered<SmallPortBER0>(mm2px(Vec(5.08, 7.5 * 4.5)), module, Veins::IN1_INPUT));
		addOutput(createOutputCentered<SmallPortBER1>(mm2px(Vec(5.08, 7.5 * 5.75)), module, Veins::OUT1A_OUTPUT));
		addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(5.08, 7.5 * 6.50)), module, Veins::STATE_LIGHTS + 0 * 2));
		addOutput(createOutputCentered<SmallPortBER2>(mm2px(Vec(5.08, 7.5 * 7.25)), module, Veins::OUT1B_OUTPUT));

		addParam(createParamCentered<SmallBtBER2>(mm2px(Vec(5.08, 7.5 * 9.5)), module, Veins::MODE2_PARAM));
		addParam(createParamCentered<TrimpotW>(mm2px(Vec(5.08, 7.5 * 10.5)), module, Veins::THRESHOLD2_PARAM));
		addInput(createInputCentered<SmallPortBER3>(mm2px(Vec(5.08, 7.5 * 11.5)), module, Veins::P2_INPUT));

		addInput(createInputCentered<SmallPortBER0>(mm2px(Vec(5.08, 7.5 * 13)), module, Veins::IN2_INPUT));
		addOutput(createOutputCentered<SmallPortBER1>(mm2px(Vec(5.08, 7.5 * 14.25)), module, Veins::OUT2A_OUTPUT));
		addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(5.08, 7.5 * 15)), module, Veins::STATE_LIGHTS + 1 * 2));
		addOutput(createOutputCentered<SmallPortBER2>(mm2px(Vec(5.08, 7.5 * 15.75)), module, Veins::OUT2B_OUTPUT));
	}
};

Model *modelVeins = createModel<Veins, VeinsWidget>("Veins");