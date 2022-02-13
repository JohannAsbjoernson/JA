#include <math.h>

#pragma once

using namespace rack;

extern Plugin *pluginInstance;

///////////////////
//////////Ports
struct MicroPortA : app::SvgPort
{
	MicroPortA()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/MicroPortA.svg")));
		shadow->opacity = 0.f;
	}
};

struct MicroPortAO : app::SvgPort
{
	MicroPortAO()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/MicroPortAO.svg")));
		shadow->opacity = 0.f;
	}
};

struct MicroPortBO : app::SvgPort
{
	MicroPortBO()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/MicroPortBO.svg")));
		shadow->opacity = 0.f;
	}
};

struct MicroPortB : app::SvgPort
{
	MicroPortB()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/MicroPortB.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortO : app::SvgPort
{
	SmallPortO()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortO.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortC : app::SvgPort
{
	SmallPortC()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortC.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortB : app::SvgPort
{
	SmallPortB()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortB.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortG : app::SvgPort
{
	SmallPortG()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortG.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortCV : app::SvgPort
{
	SmallPortCV()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortCV.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortPT : app::SvgPort
{
	SmallPortPT()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortPT.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortPC : app::SvgPort
{
	SmallPortPC()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortPC.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortPG : app::SvgPort
{
	SmallPortPG()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortPG.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortClk : app::SvgPort
{
	SmallPortClk()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortClk.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortRes : app::SvgPort
{
	SmallPortRes()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortRes.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortV : app::SvgPort
{
	SmallPortV()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortV.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortSP : app::SvgPort
{
	SmallPortSP()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortSP.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortSM : app::SvgPort
{
	SmallPortSM()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortSM.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortRND : app::SvgPort
{
	SmallPortRND()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortRND.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortRST : app::SvgPort
{
	SmallPortRST()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortRST.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortSTEP : app::SvgPort
{
	SmallPortSTEP()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortSTEP.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortTRIG : app::SvgPort
{
	SmallPortTRIG()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortTRIG.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortBER0 : app::SvgPort
{
	SmallPortBER0()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortBER0.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortBER1 : app::SvgPort
{
	SmallPortBER1()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortBER1.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortBER2 : app::SvgPort
{
	SmallPortBER2()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortBER2.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortBER3 : app::SvgPort
{
	SmallPortBER3()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortBER3.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortUP : app::SvgPort
{
	SmallPortUP()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortUP.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortDN : app::SvgPort
{
	SmallPortDN()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortDN.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortUP2 : app::SvgPort
{
	SmallPortUP2()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortUP2.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallPortDN2 : app::SvgPort
{
	SmallPortDN2()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SmallPortDN2.svg")));
		shadow->opacity = 0.f;
	}
};

struct SPEOC : app::SvgPort
{
	SPEOC()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPEOC.svg")));
		shadow->opacity = 0.f;
	}
};

struct SPSOC : app::SvgPort
{
	SPSOC()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPSOC.svg")));
		shadow->opacity = 0.f;
	}
};

struct SPDUR : app::SvgPort
{
	SPDUR()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPDUR.svg")));
		shadow->opacity = 0.f;
	}
};

struct SPOUT : app::SvgPort
{
	SPOUT()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPOUT.svg")));
		shadow->opacity = 0.f;
	}
};

struct SPCVN : app::SvgPort
{
	SPCVN()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPCVN.svg")));
		shadow->opacity = 0.f;
	}
};

struct SPCVP : app::SvgPort
{
	SPCVP()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPCVP.svg")));
		shadow->opacity = 0.f;
	}
};

struct SPCVR : app::SvgPort
{
	SPCVR()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPCVR.svg")));
		shadow->opacity = 0.f;
	}
};

struct SPCLK : app::SvgPort
{
	SPCLK()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPCLK.svg")));
		shadow->opacity = 0.f;
	}
};

struct SPCLKout : app::SvgPort
{
	SPCLKout()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPCLKout.svg")));
		shadow->opacity = 0.f;
	}
};

struct SPTRG : app::SvgPort
{
	SPTRG()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPTRG.svg")));
		shadow->opacity = 0.f;
	}
};

struct SPCVin : app::SvgPort
{
	SPCVin()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPCVin.svg")));
		shadow->opacity = 0.f;
	}
};

struct SPTRGout : app::SvgPort
{
	SPTRGout()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPTRGout.svg")));
		shadow->opacity = 0.f;
	}
};

struct SPGTout : app::SvgPort
{
	SPGTout()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPGTout.svg")));
		shadow->opacity = 0.f;
	}
};

struct SPRES : app::SvgPort
{
	SPRES()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPRES.svg")));
		shadow->opacity = 0.f;
	}
};

struct SPPOLY : app::SvgPort
{
	SPPOLY()
	{
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ports/SPPOLY.svg")));
		shadow->opacity = 0.f;
	}
};

///////////////////
//////////Buttons
struct SmallBtMS : SVGSwitch
{
	SmallBtMS()
	{
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/SmallBtMS0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/SmallBtMS1.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallBtBER : SVGSwitch
{
	SmallBtBER()
	{
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/SmallBtBER0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/SmallBtBER1.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallBtBER2 : SVGSwitch
{
	SmallBtBER2()
	{
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/SmallBtBER0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/SmallBtBER1.svg")));
		shadow->opacity = 0.f;
	}
};

struct BtRESET : SVGSwitch
{
	BtRESET()
	{
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtRESET0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtRESET1.svg")));
		shadow->opacity = 0.f;
	}
};

struct BtTRIG : SVGSwitch
{
	BtTRIG()
	{
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtTRIG0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtTRIG1.svg")));
		shadow->opacity = 0.f;
	}
};

struct BtLATCH : SVGSwitch
{
	BtLATCH()
	{
		momentary = false;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtLATCH0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtLATCH1.svg")));
		shadow->opacity = 0.f;
	}
};

struct BtAUDIO : SVGSwitch
{
	BtAUDIO()
	{
		momentary = false;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtAUDIO0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtAUDIO1.svg")));
		shadow->opacity = 0.f;
	}
};

struct BtNORM : SVGSwitch
{
	BtNORM()
	{
		momentary = false;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtNORM0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtNORM1.svg")));
		shadow->opacity = 0.f;
	}
};

struct BtLOGIC : SVGSwitch
{
	BtLOGIC()
	{
		momentary = false;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtLOG0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtLOG1.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtLOG2.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtLOG3.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtLOG4.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtLOG5.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallBtS : SVGSwitch
{
	SmallBtS()
	{
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/SmallBtS0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/SmallBtS1.svg")));
		shadow->opacity = 0.f;
	}
};

struct SmallBtP : SVGSwitch
{
	SmallBtP()
	{
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/SmallBtP0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/SmallBtP1.svg")));
		shadow->opacity = 0.f;
	}
};
struct PMOD : SVGSwitch
{
	PMOD()
	{
		momentary = false;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtPLAYMODE0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtPLAYMODE1.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/buttons/BtPLAYMODE2.svg")));
		shadow->opacity = 0.f;
	}
};

///////////////////
//////////Knobs

struct Trimknob : RoundKnob
{
public:
	Trimknob()
	{
	}

	void setSVG(std::shared_ptr<Svg> svg)
	{
		app::SvgKnob::setSvg(svg);
	}

	void draw(const DrawArgs &args) override
	{
		/** component */
		app::SvgKnob::draw(args);
	}
};

struct TrimpotW : Trimknob
{
	TrimpotW()
	{
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimpotW.svg")));
		shadow->opacity = 0.f;
	}
};

struct TrimpotG : Trimknob
{
	TrimpotG()
	{
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimpotG.svg")));
		shadow->opacity = 0.f;
	}
};

struct TrimpotR : Trimknob
{
	TrimpotR()
	{
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimpotR.svg")));
		shadow->opacity = 0.f;
	}
};

struct TrimpotB : Trimknob
{
	TrimpotB()
	{
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimpotB.svg")));
		shadow->opacity = 0.f;
	}
};

struct TrimpotGray : Trimknob
{
	TrimpotGray()
	{
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimpotGray.svg")));
		shadow->opacity = 0.f;
	}
};
struct TrimpotGrays : Trimknob
{
	TrimpotGrays()
	{
		snap = true;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimpotGray.svg")));
		shadow->opacity = 0.f;
	}
};
struct TrimpotWs : Trimknob
{
	TrimpotWs()
	{
		snap = true;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimpotW.svg")));
		shadow->opacity = 0.f;
	}
};

struct TrimpotRs : Trimknob
{
	TrimpotRs()
	{
		snap = true;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimpotR.svg")));
		shadow->opacity = 0.f;
	}
};
struct TrimpotM : Trimknob
{
	TrimpotM()
	{
		snap = false;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimpotM.svg")));
		shadow->opacity = 0.f;
	}
};
struct TrimpotMs : Trimknob
{
	TrimpotMs()
	{
		snap = true;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimpotM.svg")));
		shadow->opacity = 0.f;
	}
};
struct TrimpotROs : Trimknob
{
	TrimpotROs()
	{
		snap = true;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimpotRO.svg")));
		shadow->opacity = 0.f;
	}
};
struct MTrimW : Trimknob
{
	MTrimW()
	{
		snap = false;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimMicroW.svg")));
		shadow->opacity = 0.f;
	}
};

struct MTrimWs : Trimknob
{
	MTrimWs()
	{
		snap = true;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimMicroW.svg")));
		shadow->opacity = 0.f;
	}
};
struct MTrimPink : Trimknob
{
	MTrimPink()
	{
		snap = false;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimMicroPink.svg")));
		shadow->opacity = 0.f;
	}
};
struct MTrimBlue : Trimknob
{
	MTrimBlue()
	{
		snap = false;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimMicroBlue.svg")));
		shadow->opacity = 0.f;
	}
};
struct MTrimGray : Trimknob
{
	MTrimGray()
	{
		snap = false;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimMicroGray.svg")));
		shadow->opacity = 0.f;
	}
};

struct MTrimGrays : Trimknob
{
	MTrimGrays()
	{
		snap = true;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimMicroGray.svg")));
		shadow->opacity = 0.f;
	}
};

struct MTrimR : Trimknob
{
	MTrimR()
	{
		snap = false;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimMicroR.svg")));
		shadow->opacity = 0.f;
	}
};

struct MTrimRs : Trimknob
{
	MTrimRs()
	{
		snap = true;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimMicroR.svg")));
		shadow->opacity = 0.f;
	}
};

struct MTrimB : Trimknob
{
	MTrimB()
	{
		snap = false;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimMicroB.svg")));
		shadow->opacity = 0.f;
	}
};

struct MTrimBs : Trimknob
{
	MTrimBs()
	{
		snap = true;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimMicroB.svg")));
		shadow->opacity = 0.f;
	}
};

struct MTrimBW : Trimknob
{
	MTrimBW()
	{
		snap = false;
		minAngle = -0.75 * M_PI;
		maxAngle = 0.75 * M_PI;
		setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/TrimMicroBW.svg")));
		shadow->opacity = 0.f;
	}
};

/////////////////////
///////////////////// LIGHTS

template <typename BASE>
struct GiantLight : BASE
{
	GiantLight()
	{
		this->box.size = mm2px(Vec(18.0, 18.0));
	}
};

template <typename BASE>
struct TestLight : BASE
{
	TestLight()
	{
		this->box.size = Vec(6.8, 2.8);
		this->bgColor = nvgRGB(0x14, 0x14, 0x14);
	}
};

template <typename BASE>
struct MeterLight : BASE
{
	MeterLight()
	{
		this->box.size = Vec(8, 8);
		this->bgColor = nvgRGB(0x14, 0x14, 0x14);
	}
};

template <typename BASE>
struct DisplayLedLight : BASE
{
	DisplayLedLight()
	{
		this->box.size = mm2px(Vec(2.0, 2.0));
		this->bgColor = nvgRGB(0x14, 0x14, 0x14);
	}
};
struct OrangeLight : GrayModuleLightWidget
{
	OrangeLight()
	{
		addBaseColor(SCHEME_ORANGE);
	}
};

template <typename BASE>
struct LedLight : BASE
{
	LedLight()
	{
		//this->box.size = Vec(20.0, 20.0);
		this->box.size = mm2px(Vec(6.0, 6.0));
	}
};

/** Reads two adjacent lightIds, so `lightId` and `lightId + 1` must be defined */
struct YellowRedLight : GrayModuleLightWidget
{
	YellowRedLight()
	{
		addBaseColor(SCHEME_YELLOW);
		addBaseColor(SCHEME_RED);
	}
};

struct WhitheLight : GrayModuleLightWidget
{
	WhitheLight()
	{
		addBaseColor(SCHEME_WHITE);
	}
};

template <typename Base = GrayModuleLightWidget>
struct TWhiteBlueLight : Base
{
	TWhiteBlueLight()
	{
		this->addBaseColor(SCHEME_BLUE);
		this->addBaseColor(SCHEME_WHITE);
	}
};
typedef TWhiteBlueLight<> WhiteBlueLight;

//template <typename Base = GrayModuleLightWidget>
//struct TBlackBlueLight : Base
//{
//	TBlackBlueLight()
//	{
//		static const NVGcolor COLOR_BG = nvgRGB(0xf4, 0xf4, 0xf4);
//		this->addBaseColor(SCHEME_BLUE);
//		this->addBaseColor(SCHEME_BLACK);
//		this->bgColor = COLOR_BG;
//	}
//};
//typedef TBlackBlueLight<> BlackBlueLight;

///////////////////
//////////////// VARIOUS
static const NVGcolor COLOR_BLACK = nvgRGB(0x00, 0x00, 0x00);
static const NVGcolor COLOR_GREY_DARK = nvgRGB(0x20, 0x20, 0x20);
static const NVGcolor COLOR_PURPLE_DARK = nvgRGB(0x21, 0x1e, 0x29);
static const NVGcolor COLOR_BG = nvgRGB(0xf4, 0xf4, 0xf4);
static const NVGcolor BLACK_TRANSPARENT = nvgRGBA(0x00, 0x00, 0x00, 0x00);
static const NVGcolor BLACK = nvgRGB(0x00, 0x00, 0x00);
static const NVGcolor WHITE = nvgRGB(0xff, 0xff, 0xff);
static const NVGcolor RED = nvgRGB(0xed, 0x2c, 0x24);
static const NVGcolor ORANGE = nvgRGB(0xf2, 0xb1, 0x20);
static const NVGcolor YELLOW = nvgRGB(0xf9, 0xdf, 0x1c);
static const NVGcolor GREEN = nvgRGB(0x90, 0xc7, 0x3e);
static const NVGcolor CYAN = nvgRGB(0x22, 0xe6, 0xef);
static const NVGcolor BLUE = nvgRGB(0x29, 0xb2, 0xef);
static const NVGcolor PURPLE = nvgRGB(0xd5, 0x2b, 0xed);
static const NVGcolor LIGHT_GRAY = nvgRGB(0xe6, 0xe6, 0xe6);
static const NVGcolor DARK_GRAY = nvgRGB(0x17, 0x17, 0x17);