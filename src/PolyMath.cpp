#include "plugin.hpp"
#include "inc/GateProcessor.hpp"
#include "inc/Utility.hpp"

#include <string>
#include <sstream>
#include <iomanip>

struct PolyMath : Module
{
    enum ParamIds
    {
        MOTH,
        MOUDE,
        MOTH2,
        MOUDE2,
        T1,
        T2,
        T3,
        T4,
        T5,
        T6,
        T7,
        T8,
        LOGMODE,
        NUM_PARAMS
    };
    enum InputIds
    {
        AIN,
        BIN,
        LOGIN,
        CIN,
        DIN,
        NUM_INPUTS
    };
    enum OutputIds
    {
        ROUT,
        LOGOUT,
        ROUT2,
        NUM_OUTPUTS
    };

    enum LighIds
    {
        NUM_LIGHTS
    };

    GateProcessor logicTriggers[16];

    PolyMath()
    {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configSwitch(MOTH, 0, 3, 0, "Operation", {"add", "substract", "multiply", "divide"});
        configSwitch(MOUDE, 0, 5, 0, "Range Mod", {"unscaled (careful!)", "-10v to +10v", "-5v to +5v", "divide by 10", "positive", "negative"});
        configSwitch(MOTH2, 0, 3, 0, "Operation", {"add", "substract", "multiply", "divide"});
        configSwitch(MOUDE2, 0, 5, 0, "Range Mod", {"unscaled (careful!)", "-10v to +10v", "-5v to +5v", "divide by 10", "positive", "negative"});
        configParam(T1, -10, 10, 0, "A: Set Constant Voltage when no input is connected");
        configParam(T2, -10, 10, 0, "C: Set Constant Voltage when no input is connected");
        configParam(T3, -1, 1, 0, "B: Fine Constant Voltage");
        configParam(T4, -1, 1, 0, "D: Fine Constant Voltage");
        configParam(T5, -10, 10, 0, "A: Set Constant Voltage when no input is connected");
        configParam(T6, -10, 10, 0, "C: Set Constant Voltage when no input is connected");
        configParam(T7, -1, 1, 0, "B: Fine Constant Voltage");
        configParam(T8, -1, 1, 0, "D: Fine Constant Voltage");
        configSwitch(LOGMODE, 0, 5, 0, "Logic Operation", {"OR", "AND", "NOR", "XOR", "NAND", "XNOR"});
    }

    bool checkPoly, checkPoly2;
    float checkA = 0, checkB = 0, inA, inB, meth, solvd, chN, monitor;
    float checkC = 0, checkD = 0, inC, inD, meth2, solvd2, chN2, monitor2;
    void process(const ProcessArgs &args) override
    {
        // Set the floats/ints prior any Ifs and Loops to avoid compiler errors
        float chA = inputs[AIN].getChannels(), chB = inputs[BIN].getChannels();

        // Check if there is a cable connected
        if (inputs[AIN].isConnected())
            checkA = 1;
        else
            checkA = 0;

        if (inputs[BIN].isConnected())
            checkB = 1;
        else
            checkB = 0;

        // If two cables are connected, set the channels to the lesser value (i.e. A has 16 channels, B has 1 channel, we get results for 1 channel)
        // If 1 or 0 cables are connected, set channels & loop to 1
        if (chA >= chB && checkA == 1 && checkB == 1)
            chN = chB;
        else if (chA < chB && checkA == 1 && checkB == 1)
            chN = chA;
        else if (checkA == 0 || checkB == 0)
            chN = 1;
        outputs[ROUT].setChannels(chN);

        for (int i = 0; i < chN; i++)
        {
            // If either A or B is not connected, use the constant value from trimpot
            if (checkA == 1)
                inA = inputs[AIN].getVoltage(i);
            else
                inA = params[T1].getValue() + params[T3].getValue();
            if (checkB == 1)
                inB = inputs[BIN].getVoltage(i);
            else
                inB = params[T2].getValue() + params[T4].getValue();

            // Check for chosen MATH Operation and get result
            if (params[MOTH].getValue() == 0)
                meth = inA + inB;
            else if (params[MOTH].getValue() == 1)
                meth = inA - inB;
            else if (params[MOTH].getValue() == 2)
                meth = inA * inB;
            else if (params[MOTH].getValue() == 3)
                meth = inA / inB;

            // Check if mode is chosen (default is 0) and if yes, change results accordingly
            if (params[MOUDE].getValue() == 0)
                solvd = meth;
            else if (params[MOUDE].getValue() == 1)
                solvd = clamp(meth, -10.f, 10.f);
            else if (params[MOUDE].getValue() == 2)
                solvd = clamp(meth, -5.f, 5.f);
            else if (params[MOUDE].getValue() == 3)
                solvd = (meth / 10);
            else if (params[MOUDE].getValue() == 4 && meth >= 0)
                solvd = meth;
            else if (params[MOUDE].getValue() == 4 && meth < 0)
                solvd = meth * (-1);
            else if (params[MOUDE].getValue() == 5 && meth > 0)
                solvd = meth * (-1);
            else if (params[MOUDE].getValue() == 5 && meth <= 0)
                solvd = meth;

            // Send the result of the operation to Output
            outputs[ROUT].setVoltage(solvd, i);

            // Removes unwanted decimals to always fit the Display
            if (inputs[AIN].isPolyphonic() && inputs[BIN].isPolyphonic())
                checkPoly = false;
            else
            {
                checkPoly = true;
                monitor = (int)(solvd * 1000 + .5);
                monitor = (float)monitor / 1000;
            }
        }

        // Set the floats/ints prior any Ifs and Loops to avoid compiler errors
        float chC = inputs[CIN].getChannels(), chD = inputs[DIN].getChannels();

        // Check if there is a cable connected
        if (inputs[CIN].isConnected())
            checkC = 1;
        else
            checkC = 0;

        if (inputs[DIN].isConnected())
            checkD = 1;
        else
            checkD = 0;

        // If two cables are connected, set the channels to the lesser value (i.e. A has 16 channels, B has 1 channel, we get results for 1 channel)
        // If 1 or 0 cables are connected, set channels & loop to 1
        if (chC >= chD && checkC == 1 && checkD == 1)
            chN2 = chD;
        else if (chC < chD && checkC == 1 && checkD == 1)
            chN2 = chC;
        else if (checkC == 0 || checkD == 0)
            chN2 = 1;
        outputs[ROUT2].setChannels(chN2);

        for (int i = 0; i < chN2; i++)
        {
            // If either A or B is not connected, use the constant value from trimpot
            if (checkC == 1)
                inC = inputs[CIN].getVoltage(i);
            else
                inC = params[T5].getValue() + params[T7].getValue();
            if (checkD == 1)
                inD = inputs[DIN].getVoltage(i);
            else
                inD = params[T6].getValue() + params[T8].getValue();

            // Check for chosen MATH Operation and get result
            if (params[MOTH2].getValue() == 0)
                meth2 = inC + inD;
            else if (params[MOTH2].getValue() == 1)
                meth2 = inC - inD;
            else if (params[MOTH2].getValue() == 2)
                meth2 = inC * inD;
            else if (params[MOTH2].getValue() == 3)
                meth2 = inC / inD;

            // Check if mode is chosen (default is 0) and if yes, change results accordingly
            if (params[MOUDE2].getValue() == 0)
                solvd2 = meth;
            else if (params[MOUDE2].getValue() == 1)
                solvd2 = clamp(meth2, -10.f, 10.f);
            else if (params[MOUDE2].getValue() == 2)
                solvd2 = clamp(meth2, -5.f, 5.f);
            else if (params[MOUDE2].getValue() == 3)
                solvd2 = (meth2 / 10);
            else if (params[MOUDE2].getValue() == 4 && meth2 >= 0)
                solvd = meth2;
            else if (params[MOUDE2].getValue() == 4 && meth2 < 0)
                solvd2 = meth2 * (-1);
            else if (params[MOUDE2].getValue() == 5 && meth2 > 0)
                solvd2 = meth2 * (-1);
            else if (params[MOUDE2].getValue() == 5 && meth2 <= 0)
                solvd2 = meth2;

            // Send the result of the operation to Output
            outputs[ROUT2].setVoltage(solvd2, i);

            // Removes unwanted decimals to always fit the Display
            if (inputs[CIN].isPolyphonic() && inputs[DIN].isPolyphonic())
                checkPoly2 = false;
            else
            {
                checkPoly2 = true;
                monitor2 = (int)(solvd2 * 1000 + .5);
                monitor2 = (float)monitor2 / 1000;
            }
        }
        // POLY TO MONO LOGIC
        int logch = inputs[LOGIN].getChannels(), lstep = 0;
        bool lout;
        if (inputs[LOGIN].isConnected() && inputs[LOGIN].isPolyphonic())
        {
            int lmode = params[LOGMODE].getValue();
            for (int k = 0; k < logch; k++)
            {
                logicTriggers[k].set(inputs[LOGIN].getVoltage(k));
                if (logicTriggers[k].high())
                    lstep++;
                else
                    logicTriggers[k].set(0.0f);
            }
            bool andbool = (lstep == logch), orbool = (lstep > 0), xorbool = (0 > 0.5f ? lstep == 1 : isOdd(lstep));
            if (lmode == 0)
                lout = orbool;
            else if (lmode == 1)
                lout = andbool;
            else if (lmode == 2)
                lout = xorbool;
            else if (lmode == 3)
                lout = !orbool;
            else if (lmode == 4)
                lout = !andbool;
            else
                lout = !xorbool;
            outputs[LOGOUT].setVoltage(boolToGate(lout));
        }
    }
};

////////////////////////////////////

///////////////////////////////////

// Define the Buttons for MOTH and MOUDE
struct BtMOTH : app::SvgSwitch
{
    BtMOTH()
    {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtMATH0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtMATH1.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtMATH2.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtMATH3.svg")));
        shadow->opacity = 0.f;
    }
};

struct BtMOUDE : app::SvgSwitch
{
    BtMOUDE()
    {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtMODE0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtMODE1.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtMODE2.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtMODE3.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtMODE4.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtMODE5.svg")));
        shadow->opacity = 0.f;
    }
};

struct NumberDisplayWidget4 : TransparentWidget
{

    // int *value;
    float *value;

    PolyMath *module;
    std::shared_ptr<Font> font;

    NumberDisplayWidget4()
    {
        font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Oswald-Regular.ttf"));
    };

    void draw(const DrawArgs &args) override
    {
        // Background
        NVGcolor backgroundColor = nvgRGB(0x00, 0x00, 0x00);
        NVGcolor StrokeColor = nvgRGB(0x00, 0x00, 0x00);
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, -1.0, -1.0, box.size.x + 2, box.size.y + 2, 3.0);
        nvgFillColor(args.vg, StrokeColor);
        nvgFill(args.vg);
        nvgBeginPath(args.vg);
        nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 3.0);
        nvgFillColor(args.vg, backgroundColor);
        nvgFill(args.vg);

        // text
        nvgFontSize(args.vg, 11);
        nvgFontFaceId(args.vg, font->handle);
        nvgTextLetterSpacing(args.vg, 1.0);

        std::stringstream to_display;
        to_display << std::setw(4) << std::setprecision(4) << *value;

        Vec textPos = Vec(0.f, 10.0f);
        NVGcolor textColor = nvgRGB(0xff, 0xff, 0xff);
        nvgFillColor(args.vg, textColor);
        nvgText(args.vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
    }
};
// Set the Panel appearance
struct PolyMathWidget : ModuleWidget
{
    PolyMathWidget(PolyMath *module)
    {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Panel2hp.svg")));

        NumberDisplayWidget4 *display1 = new NumberDisplayWidget4();
        display1->box.pos = Vec(2, 14);
        display1->box.size = Vec(26, 13);
        display1->value = &module->monitor;
        addChild(display1);

        // Set input A & micro Trimpots
        addParam(createParamCentered<MTrimBs>(Vec(8, 23 * 1.6), module, PolyMath::T1));
        addParam(createParamCentered<MTrimBW>(Vec(22, 23 * 1.6), module, PolyMath::T3));
        addInput(createInputCentered<SmallPortBER1>(Vec(15, 23 * 2.3), module, PolyMath::AIN));

        // Set input B & micro Trimpots
        addInput(createInputCentered<SmallPortBER2>(Vec(15, 23 * 3.3), module, PolyMath::BIN));
        addParam(createParamCentered<MTrimBs>(Vec(8, 23 * 4), module, PolyMath::T2));
        addParam(createParamCentered<MTrimBW>(Vec(22, 23 * 4), module, PolyMath::T4));

        // Set buttons for MOTH and MOUDE
        addParam(createParamCentered<BtMOTH>(Vec(15, 23 * 4.6), module, PolyMath::MOTH));
        addParam(createParamCentered<BtMOUDE>(Vec(15, 23 * 5), module, PolyMath::MOUDE));

        // Set output
        addOutput(createOutputCentered<SPOUT>(Vec(15, 23 * 5.8), module, PolyMath::ROUT));

        // NumberDisplayWidget4 *display2 = new NumberDisplayWidget4();
        // display2->box.pos = Vec(2, 155);
        // display2->box.size = Vec(26, 13);
        // display2->value = &module->monitor2;
        // addChild(display2);

        // Set input A & micro Trimpots 2
        addParam(createParamCentered<MTrimBs>(Vec(8, 23 * 8.3), module, PolyMath::T5));
        addParam(createParamCentered<MTrimBW>(Vec(22, 23 * 8.3), module, PolyMath::T7));
        addInput(createInputCentered<SmallPortBER1>(Vec(15, 23 * 9), module, PolyMath::CIN));
        addInput(createInputCentered<SmallPortBER2>(Vec(15, 23 * 10), module, PolyMath::DIN));
        addParam(createParamCentered<MTrimBs>(Vec(8, 23 * 10.7), module, PolyMath::T6));
        addParam(createParamCentered<MTrimBW>(Vec(22, 23 * 10.7), module, PolyMath::T8));
        addParam(createParamCentered<BtMOTH>(Vec(15, 23 * 11.3), module, PolyMath::MOTH2));
        addParam(createParamCentered<BtMOUDE>(Vec(15, 23 * 11.7), module, PolyMath::MOUDE2));
        addOutput(createOutputCentered<SPOUT>(Vec(15, 23 * 12.5), module, PolyMath::ROUT2));

        // Set Poly Logic to Mono output
        addInput(createInputCentered<SPPOLY>(Vec(15, 23 * 14), module, PolyMath::LOGIN));
        addParam(createParamCentered<BtLOGIC>(Vec(15, 23 * 14.7), module, PolyMath::LOGMODE));
        addOutput(createOutputCentered<SPOUT>(Vec(15, 23 * 15.4), module, PolyMath::LOGOUT));
    }
};

Model *modelPolyMath = createModel<PolyMath, PolyMathWidget>("PolyMath");