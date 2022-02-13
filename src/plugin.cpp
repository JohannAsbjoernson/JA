#include "plugin.hpp"

Plugin *pluginInstance;

void init(Plugin *p)
{
    pluginInstance = p;

    p->addModel(modelMerge8);
    p->addModel(modelMerge16);
    p->addModel(modelSplit8);
    p->addModel(modelSplit16);
    p->addModel(modelBlank2hp);
    p->addModel(modelCRG);
    p->addModel(modelDebug);
    p->addModel(modelCounter);
    p->addModel(modelVeins);
    p->addModel(modelCompare);
    p->addModel(modelPolyTools);
    p->addModel(modelPolyRND);
    p->addModel(modelVoltageBank);
    p->addModel(modelVoltageBank2);
    p->addModel(modelFlamm);
    p->addModel(modelCV2P);
    p->addModel(modelSeq16);
    p->addModel(modelDivider);
    // p->addModel(modelPolyMath);
    p->addModel(modelDrone);
    p->addModel(modelPolySwitch);
    p->addModel(modelN1);
    p->addModel(modelPolyMorph);
}