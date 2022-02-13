#include "plugin.hpp"

#include <sstream>
#include <iomanip>

struct Counter : Module
{
    enum ParamIds
    {
        RST_BUTTON1,
        COUNT_NUM_PARAM_1,
        RST_BUTTON2,
        COUNT_NUM_PARAM_2,
        NUM_PARAMS
    };
    enum InputIds
    {
        CLK_IN_1,
        RESET_IN_1,
        CLK_IN_2,
        RESET_IN_2,
        NUM_INPUTS
    };
    enum OutputIds
    {
        OUTPUT_1,
        OUTPUT_2,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        RESET_LIGHT1,
        RESET_LIGHT2,
        NUM_LIGHTS
    };

    dsp::SchmittTrigger clock_trigger_1;
    dsp::SchmittTrigger reset_trigger_1;
    dsp::SchmittTrigger reset_ext_trigger_1;
    int count_limit1 = 1;
    int count1 = 0;

    dsp::SchmittTrigger clock_trigger_2;
    dsp::SchmittTrigger reset_trigger_2;
    dsp::SchmittTrigger reset_ext_trigger_2;
    int count_limit2 = 1;
    int count2 = 0;

    const float lightLambda = 0.075f;
    float resetLight1 = 0.0f;
    float resetLight2 = 0.0f;

    dsp::PulseGenerator clockPulse1;
    bool pulse1 = false;

    dsp::PulseGenerator clockPulse2;
    bool pulse2 = false;

    Counter()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(Counter::RST_BUTTON1, 0.0f, 1.0f, 0.0f, "CH 1 Reset");
        configParam(Counter::COUNT_NUM_PARAM_1, 1.0f, 256.0f, 1.0f, "CH 1 Count");
        configParam(Counter::RST_BUTTON2, 0.0f, 1.0f, 0.0f, "CH 2 Reset");
        configParam(Counter::COUNT_NUM_PARAM_2, 1.0f, 256.0f, 1.0f, "CH 2 Count");
        configInput(Counter::CLK_IN_1, "CLOCK 1");
        configInput(Counter::CLK_IN_2, "CLOCK 2");
        configInput(Counter::RESET_IN_1, "RESET 1");
        configInput(Counter::RESET_IN_2, "RESET 2");
        configOutput(Counter::OUTPUT_1, "COUNTER 1");
        configOutput(Counter::OUTPUT_2, "COUNTER 2");
        configLight(Counter::RESET_LIGHT1, "RESET 1");
        configLight(Counter::RESET_LIGHT2, "RESET 2");
    }

    void process(const ProcessArgs &args) override
    {

        count_limit1 = round(params[COUNT_NUM_PARAM_1].getValue());
        count_limit2 = round(params[COUNT_NUM_PARAM_2].getValue());

        bool reset1 = false;
        bool reset2 = false;
        pulse1 = false;

        if (reset_trigger_1.process(params[RST_BUTTON1].getValue()))
        {
            reset1 = true;
            count1 = 0;
            outputs[OUTPUT_1].setVoltage(0);
            resetLight1 = 1.0f;
        }
        if (reset_ext_trigger_1.process(inputs[RESET_IN_1].getVoltage()))
        {
            reset1 = true;
            count1 = 0;
            outputs[OUTPUT_1].setVoltage(0);
            resetLight1 = 1.0f;
        }

        resetLight1 -= resetLight1 / lightLambda / args.sampleRate;
        lights[RESET_LIGHT1].value = resetLight1;

        if (reset1 == false)
        {
            if (clock_trigger_1.process(inputs[CLK_IN_1].getVoltage()) && count1 <= count_limit1)
                count1++;
        }
        if (count1 == count_limit1)
        {
            clockPulse1.trigger(1e-3);
        }
        if (count1 > count_limit1)
        {
            count1 = 0;
        }
        pulse1 = clockPulse1.process(1.0 / args.sampleRate);
        outputs[OUTPUT_1].setVoltage(pulse1 ? 10.0f : 0.0f);

        //// second counter
        if (reset_trigger_2.process(params[RST_BUTTON2].getValue()))
        {
            reset2 = true;
            count2 = 0;
            outputs[OUTPUT_2].setVoltage(0);
            resetLight2 = 1.0f;
        }
        if (reset_ext_trigger_2.process(inputs[RESET_IN_2].getVoltage()))
        {
            reset2 = true;
            count2 = 0;
            outputs[OUTPUT_2].setVoltage(0);
            resetLight2 = 1.0f;
        }
        resetLight2 -= resetLight2 / lightLambda / args.sampleRate;
        lights[RESET_LIGHT2].value = resetLight2;

        if (reset2 == false)
        {
            if (clock_trigger_2.process(inputs[CLK_IN_2].getVoltage()) && count2 <= count_limit2)
                count2++;
        }
        if (count2 == count_limit2)
        {
            clockPulse2.trigger(1e-3);
        }
        if (count2 > count_limit2)
        {
            count2 = 0;
        }
        pulse2 = clockPulse2.process(1.0 / args.sampleRate);
        outputs[OUTPUT_2].setVoltage(pulse2 ? 10.0f : 0.0f);
    }
};

///////////////////////////////////
struct NumberDisplayWidget : TransparentWidget
{

    int *value = NULL;
    std::shared_ptr<Font> font;

    NumberDisplayWidget()
    {
        font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Segment7Standard.ttf"));
    };

    void draw(const DrawArgs &args) override
    {
        if (!value)
        {
            return;
        }
        // text
        nvgFontSize(args.vg, 12);
        nvgFontFaceId(args.vg, font->handle);
        nvgTextLetterSpacing(args.vg, 1);

        std::stringstream to_display;
        to_display << std::right << std::setw(3) << *value;

        Vec textPos = Vec(2.f, 10.0f);

        NVGcolor textColor = nvgRGB(0xff, 0xff, 0xff);
        nvgFillColor(args.vg, textColor);
        nvgText(args.vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
    }
};
////////////////////////////////////

struct CounterWidget : ModuleWidget
{

    CounterWidget(Counter *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpCNT.svg")));
        // counter 1
        //COUNT DISPLAY
        NumberDisplayWidget *display1 = new NumberDisplayWidget();
        display1->box.pos = Vec(2, 18);
        display1->box.size = Vec(30, 10);
        if (module)
        {
            display1->value = &module->count1;
        }
        addChild(display1);
        //Counter DISPLAY
        NumberDisplayWidget *display2 = new NumberDisplayWidget();
        display2->box.pos = Vec(2, 34.5);
        display2->box.size = Vec(30, 10);
        if (module)
        {
            display2->value = &module->count_limit1;
        }
        addChild(display2);

        addParam(createParamCentered<SmallBtBER2>(Vec(15, 23 * 2.8), module, Counter::RST_BUTTON1));
        addChild(createLightCentered<LedLight<RedLight>>(Vec(15, 23 * 2.8), module, Counter::RESET_LIGHT1));
        addParam(createParamCentered<TrimpotWs>(Vec(15, 23 * 3.9), module, Counter::COUNT_NUM_PARAM_1));
        addInput(createInputCentered<SmallPortRes>(Vec(15, 23 * 5), module, Counter::RESET_IN_1));
        addInput(createInputCentered<SmallPortSP>(Vec(15, 23 * 6), module, Counter::CLK_IN_1));
        addOutput(createOutputCentered<SmallPortO>(Vec(15, 23 * 7), module, Counter::OUTPUT_1));

        //int group_offset = 190;

        // counter 2
        //COUNT DISPLAY
        NumberDisplayWidget *display3 = new NumberDisplayWidget();
        display3->box.pos = Vec(2, 190.5);
        display3->box.size = Vec(30, 10);
        if (module)
        {
            display3->value = &module->count2;
        }
        addChild(display3);

        //Counter DISPLAY
        NumberDisplayWidget *display4 = new NumberDisplayWidget();
        display4->box.pos = Vec(2, 207);
        display4->box.size = Vec(30, 10);
        if (module)
        {
            display4->value = &module->count_limit2;
        }
        addChild(display4);

        addParam(createParamCentered<SmallBtBER2>(Vec(15, 23 * 10.3), module, Counter::RST_BUTTON2));
        addChild(createLightCentered<LedLight<RedLight>>(Vec(15, 23 * 10.3), module, Counter::RESET_LIGHT2));
        addParam(createParamCentered<TrimpotWs>(Vec(15, 23 * 11.4), module, Counter::COUNT_NUM_PARAM_2));
        addInput(createInputCentered<SmallPortRes>(Vec(15, 23 * 12.5), module, Counter::RESET_IN_2));
        addInput(createInputCentered<SmallPortSP>(Vec(15, 23 * 13.5), module, Counter::CLK_IN_2));
        addOutput(createOutputCentered<SmallPortO>(Vec(15, 23 * 14.5), module, Counter::OUTPUT_2));
    }
};

Model *modelCounter = createModel<Counter, CounterWidget>("Counter");