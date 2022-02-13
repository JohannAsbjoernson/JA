#include "plugin.hpp"
#include "dsp/digital.hpp"
#include "dsp/filter.hpp"

#include <string>
#include <sstream>
#include <iomanip>

#define NUM_LINES 16
struct Debug;
std::string noModuleStringValue = "+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n";

struct Debug : Module
{
    enum ParamIds
    {
        MANUAL_TRIGGER,
        MANUAL_CLEAR_TRIGGER,
        SWITCH_VIEW,
        WHICH_CLOCK,
        CHANNEL_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        VAL_INPUT,
        TRG_INPUT,
        CLR_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        POLY_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        BLINK_LIGHT,
        NUM_LIGHTS
    };

    std::string defaultStrValue = "+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n+0.00\n";
    std::string strValue = "0.00\n0.00\n0.00\n0.00\n0.00\n0.00\n0.00\n0.00\n0.00\n0.00\n0.00\n0.00\n0.00\n0.00\n0.00\n0.00\n";

    float logLines[NUM_LINES] = {0.f};
    float val;

    int clockChannel = 0;
    int inputChannel = 0;

    int clockMode = 1;
    int inputMode = 2;

    int lineCounter = 0;
    int outputRangeEnum = 0;
    float outputRanges[8][2];

    int stepCounter;
    dsp::SchmittTrigger clockTriggers[NUM_LINES];
    dsp::SchmittTrigger clearTrigger;
    dsp::SchmittTrigger manualClockTrigger;
    dsp::SchmittTrigger manualClearTrigger;

    enum clockAndInputModes
    {
        SINGLE_MODE,
        INTERNAL_MODE,
        POLY_MODE
    };

    Debug()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(MANUAL_TRIGGER, 0.f, 1.f, 0.f, "Manual Trigger Button");
        configSwitch(SWITCH_VIEW, 0.0f, 2.0f, 2.0f, "Input Mode", {"Monophonic Input (external)", "Internal/Auto", "Polyphonic Input (external)"});
        configSwitch(WHICH_CLOCK, 0.0f, 2.0f, 1.0f, "Clock Mode", {"Single/Mono (external)", "Internal/Auto", "Poly (external)"});
        configParam(CHANNEL_PARAM, 1.f, 16.f, 16.f, "Channels #");
        configParam(MANUAL_CLEAR_TRIGGER, 0.f, 1.f, 0.f, "Manual Reset Button");
        outputRanges[0][0] = 0.f;
        outputRanges[0][1] = 10.f;
        outputRanges[1][0] = -5.f;
        outputRanges[1][1] = 5.f;
        outputRanges[2][0] = 0.f;
        outputRanges[2][1] = 5.f;
        outputRanges[3][0] = 0.f;
        outputRanges[3][1] = 1.f;
        outputRanges[4][0] = -1.f;
        outputRanges[4][1] = 1.f;
        outputRanges[5][0] = -10.f;
        outputRanges[5][1] = 10.f;
        outputRanges[6][0] = -2.f;
        outputRanges[6][1] = 2.f;
        outputRanges[7][0] = 0.f;
        outputRanges[7][1] = 2.f;
        stepCounter = 0;
        configInput(VAL_INPUT, "Poly Voltage Signal (in)");
        configInput(TRG_INPUT, "Clock/Trigger (in)");
        configInput(CLR_INPUT, "Reset (in)");
        configOutput(POLY_OUTPUT, "Poly Voltage Signal (out)");
    }
    void process(const ProcessArgs &args) override;

    void onRandomize() override
    {
        randomizeStorage();
    }

    void randomizeStorage()
    {
        float min = outputRanges[outputRangeEnum][0];
        float max = outputRanges[outputRangeEnum][1];
        float spread = max - min;
        for (int i = 0; i < 16; i++)
        {
            logLines[i] = min + spread * random::uniform();
        }
    }

    json_t *dataToJson() override
    {
        json_t *rootJ = json_object();

        json_object_set_new(rootJ, "outputRange", json_integer(outputRangeEnum));

        json_t *sequencesJ = json_array();

        for (int i = 0; i < 16; i++)
        {
            json_t *sequenceJ = json_real(logLines[i]);
            json_array_append_new(sequencesJ, sequenceJ);
        }
        json_object_set_new(rootJ, "lines", sequencesJ);
        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override
    {
        json_t *outputRangeEnumJ = json_object_get(rootJ, "outputRange");
        if (outputRangeEnumJ)
        {
            outputRangeEnum = json_integer_value(outputRangeEnumJ);
        }

        json_t *sequencesJ = json_object_get(rootJ, "lines");

        if (sequencesJ)
        {
            for (int i = 0; i < 16; i++)
            {
                json_t *sequenceJ = json_array_get(sequencesJ, i);
                if (sequenceJ)
                    val = json_real_value(sequenceJ);
                logLines[i] = val;
            }
        }
    }
};
void Debug::process(const ProcessArgs &args)
{
    std::string thisVal;
    clockMode = floor(params[WHICH_CLOCK].getValue());

    inputMode = floor(params[SWITCH_VIEW].getValue());

    float min = outputRanges[outputRangeEnum][0];
    float max = outputRanges[outputRangeEnum][1];
    float spread = max - min;
    if (clockMode == SINGLE_MODE)
    {
        if (clockTriggers[clockChannel].process(inputs[TRG_INPUT].getVoltage(clockChannel) / 2.f) || manualClockTrigger.process(params[MANUAL_TRIGGER].getValue()))
        {
            if (inputMode == POLY_MODE)
            {
                for (int i = 0; i < 16; i++)
                {
                    logLines[i] = inputs[VAL_INPUT].getVoltage(i);
                }
            }
            else if (inputMode == SINGLE_MODE)
            {
                for (unsigned int a = NUM_LINES - 1; a > 0; a = a - 1)
                {
                    logLines[a] = logLines[a - 1];
                }

                logLines[0] = inputs[VAL_INPUT].getVoltage(inputChannel);
            }
            else if (inputMode == INTERNAL_MODE)
            {
                for (int i = 0; i < 16; i++)
                {
                    logLines[i] = min + spread * random::uniform();
                }
            }
        }
    }
    else if (clockMode == INTERNAL_MODE)
    {
        if (inputMode == POLY_MODE)
        {
            for (int i = 0; i < 16; i++)
            {
                logLines[i] = inputs[VAL_INPUT].getVoltage(i);
            }
        }
        else if (inputMode == SINGLE_MODE)
        {
            logLines[inputChannel] = inputs[VAL_INPUT].getVoltage(inputChannel);
        }
        else if (inputMode == INTERNAL_MODE)
        {
            for (int i = 0; i < 16; i++)
            {
                logLines[i] = min + spread * random::uniform();
            }
        }
    }
    else if (clockMode == POLY_MODE)
    {
        if (inputMode == POLY_MODE)
        {
            for (int i = 0; i < 16; i++)
            {
                if (clockTriggers[i].process(inputs[TRG_INPUT].getVoltage(i) / 2.f) || manualClockTrigger.process(params[MANUAL_TRIGGER].getValue()))
                {
                    logLines[i] = inputs[VAL_INPUT].getVoltage(i);
                }
            }
        }
        else if (inputMode == SINGLE_MODE)
        {
            for (int i = 0; i < 16; i++)
            {
                if (clockTriggers[i].process(inputs[TRG_INPUT].getVoltage(i) / 2.f) || manualClockTrigger.process(params[MANUAL_TRIGGER].getValue()))
                {
                    logLines[i] = inputs[VAL_INPUT].getVoltage(inputChannel);
                }
            }
        }
        else if (inputMode == INTERNAL_MODE)
        {
            for (int i = 0; i < 16; i++)
            {
                if (clockTriggers[i].process(inputs[TRG_INPUT].getVoltage(i) / 2.f) || manualClockTrigger.process(params[MANUAL_TRIGGER].getValue()))
                {
                    logLines[i] = min + spread * random::uniform();
                }
            }
        }
    }

    if (clearTrigger.process(inputs[CLR_INPUT].getVoltage() / 2.f) || manualClearTrigger.process(params[MANUAL_CLEAR_TRIGGER].getValue()))
    {
        for (unsigned int a = 0; a < NUM_LINES; a++)
        {
            logLines[a] = 0;
        }
        strValue = defaultStrValue;
    }
    if (inputs[VAL_INPUT].isConnected() && inputs[VAL_INPUT].isPolyphonic() && (inputs[VAL_INPUT].getChannels() >= params[CHANNEL_PARAM].getValue()))
        outputs[POLY_OUTPUT].setChannels(inputs[VAL_INPUT].getChannels());
    else if (inputs[TRG_INPUT].isConnected() && inputs[TRG_INPUT].isPolyphonic() && (inputs[TRG_INPUT].getChannels() >= params[CHANNEL_PARAM].getValue()))
        outputs[POLY_OUTPUT].setChannels(inputs[TRG_INPUT].getChannels());
    else
        outputs[POLY_OUTPUT].setChannels(params[CHANNEL_PARAM].getValue());

    stepCounter++;

    if (stepCounter > 1025)
    {
        stepCounter = 0;

        thisVal = "";
        std::string thisLine = "";
        for (unsigned int a = 0; a < params[CHANNEL_PARAM].getValue(); a = a + 1)
        {
            thisLine = logLines[a] >= 0 ? "+" : "";
            thisLine += std::to_string(logLines[a]);
            thisLine = thisLine.substr(0, 5);
            thisVal += (a > 0 ? "\n" : "") + thisLine;

            outputs[POLY_OUTPUT].setVoltage(logLines[a], a);
        }
        strValue = thisVal;
    }
}
////////////////////////////////////
struct StringDisplayWidget3 : Widget
{

    std::string value;
    std::shared_ptr<Font> font;
    Debug *module;

    StringDisplayWidget3()
    {
        font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Oswald-Regular.ttf"));
    };

    void draw(const DrawArgs &ctx) override
    {
        // Background
        NVGcolor backgroundColor = nvgRGB(0x00, 0x00, 0x00);
        NVGcolor StrokeColor = nvgRGB(0xC0, 0xC7, 0xDE);
        nvgBeginPath(ctx.vg);
        nvgRoundedRect(ctx.vg, -1.0, -1.0, box.size.x, box.size.y + 2, 0.0);
        nvgFillColor(ctx.vg, StrokeColor);
        nvgFill(ctx.vg);
        nvgBeginPath(ctx.vg);
        nvgRoundedRect(ctx.vg, 0.0, 0.0, box.size.x, box.size.y, 0.0);
        nvgFillColor(ctx.vg, backgroundColor);
        nvgFill(ctx.vg);

        nvgFontSize(ctx.vg, 11);
        nvgFontFaceId(ctx.vg, font->handle);
        nvgTextLetterSpacing(ctx.vg, 1.5);

        std::string textToDraw = module ? module->strValue : noModuleStringValue;
        Vec textPos = Vec(3.5f, 10.0f);
        NVGcolor textColor = nvgRGB(0xff, 0xff, 0xff);
        nvgFillColor(ctx.vg, textColor);
        nvgTextBox(ctx.vg, textPos.x, textPos.y, 80, textToDraw.c_str(), NULL);
    }
};

struct CLKSW : SVGSwitch
{
    CLKSW()
    {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtDebug00.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtDebug01.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtDebug02.svg")));
        shadow->opacity = 0.f;
    }
};
struct INPSW : SVGSwitch
{
    INPSW()
    {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtDebugA0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtDebugA1.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtDebugA2.svg")));
        shadow->opacity = 0.f;
    }
};
struct DebugWidget : ModuleWidget
{
    DebugWidget(Debug *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hpDEBUG.svg")));

        addParam(createParamCentered<CLKSW>(Vec(15, 23 * 9.5), module, Debug::WHICH_CLOCK));
        addParam(createParamCentered<INPSW>(Vec(15, 23 * 10), module, Debug::SWITCH_VIEW));
        addInput(createInputCentered<SPRES>(Vec(15, 23 * 11), module, Debug::CLR_INPUT));
        addInput(createInputCentered<SPCLK>(Vec(15, 23 * 12), module, Debug::TRG_INPUT));
        addInput(createInputCentered<SPPOLY>(Vec(15, 23 * 13), module, Debug::VAL_INPUT));
        addParam(createParamCentered<MTrimBs>(Vec(15, 23 * 13.8), module, Debug::CHANNEL_PARAM));
        addOutput(createOutputCentered<SPOUT>(Vec(15, 23 * 14.5), module, Debug::POLY_OUTPUT));
        addParam(createParamCentered<BtTRIG>(Vec(15, 23 * 15.5), module, Debug::MANUAL_TRIGGER));
        addParam(createParamCentered<BtRESET>(Vec(15, 23 * 16), module, Debug::MANUAL_CLEAR_TRIGGER));

        StringDisplayWidget3 *stringDisplay = createWidget<StringDisplayWidget3>(Vec(0, 20));
        stringDisplay->box.size = Vec(30, 180);
        stringDisplay->module = module;
        addChild(stringDisplay);

        debug = module;
    }

    void fromJson(json_t *rootJ)
    {
        float val;
        ModuleWidget::fromJson(rootJ);

        json_t *outputRangeEnumJ = json_object_get(rootJ, "outputRange");
        if (outputRangeEnumJ)
        {
            debug->outputRangeEnum = json_integer_value(outputRangeEnumJ);
        }

        json_t *sequencesJ = json_object_get(rootJ, "lines");

        if (sequencesJ)
        {
            for (int i = 0; i < 16; i++)
            {
                json_t *sequenceJ = json_array_get(sequencesJ, i);
                if (sequenceJ)
                    val = json_real_value(sequenceJ);
                debug->logLines[i] = val;
            }
        }
    }
    void appendContextMenu(Menu *menu) override;
    Debug *debug;
};

struct DebugOutputRangeItem : MenuItem
{
    Debug *debug;
    int outputRangeEnum;
    void onAction(const event::Action &e) override
    {
        debug->outputRangeEnum = outputRangeEnum;
        printf("outputRangeEnum %i\n", outputRangeEnum);
    }
    void step() override
    {
        rightText = CHECKMARK(debug->outputRangeEnum == outputRangeEnum);
        MenuItem::step();
    }
};

void DebugWidget::appendContextMenu(Menu *menu)
{
    Debug *debug = dynamic_cast<Debug *>(this->module);

    MenuLabel *spacerLabel = new MenuLabel();
    menu->addChild(spacerLabel);

    menu->addChild(construct<MenuLabel>());
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Random Generator Range (Internal In)"));
    menu->addChild(construct<DebugOutputRangeItem>(&MenuItem::text, "  0v ... +10v", &DebugOutputRangeItem::debug, debug, &DebugOutputRangeItem::outputRangeEnum, 0));
    menu->addChild(construct<DebugOutputRangeItem>(&MenuItem::text, " -5v ...  +5v", &DebugOutputRangeItem::debug, debug, &DebugOutputRangeItem::outputRangeEnum, 1));
    menu->addChild(construct<DebugOutputRangeItem>(&MenuItem::text, "  0v ...  +5v", &DebugOutputRangeItem::debug, debug, &DebugOutputRangeItem::outputRangeEnum, 2));
    menu->addChild(construct<DebugOutputRangeItem>(&MenuItem::text, "  0v ...  +1v", &DebugOutputRangeItem::debug, debug, &DebugOutputRangeItem::outputRangeEnum, 3));
    menu->addChild(construct<DebugOutputRangeItem>(&MenuItem::text, " -1v ...  +1v", &DebugOutputRangeItem::debug, debug, &DebugOutputRangeItem::outputRangeEnum, 4));
    menu->addChild(construct<DebugOutputRangeItem>(&MenuItem::text, "-10v ... +10v", &DebugOutputRangeItem::debug, debug, &DebugOutputRangeItem::outputRangeEnum, 5));
    menu->addChild(construct<DebugOutputRangeItem>(&MenuItem::text, " -2v ...  +2v", &DebugOutputRangeItem::debug, debug, &DebugOutputRangeItem::outputRangeEnum, 6));
    menu->addChild(construct<DebugOutputRangeItem>(&MenuItem::text, "  0v ...  +2v", &DebugOutputRangeItem::debug, debug, &DebugOutputRangeItem::outputRangeEnum, 7));
}

Model *modelDebug = createModel<Debug, DebugWidget>("Debug");