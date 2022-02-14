## JA (Module Collection for VCV)
**22 modules** in **2HP** - a collection of densely compressed utilities, sequencers and experimental builds for **VCV2**.  

*The modules are currently in beta and only tested on a Windows 64 System.  
Download builds for your Operating System here:* [https://github.com/JohannAsbjoernson/JA/releases](https://github.com/JohannAsbjoernson/JA/releases)
*(unpack the .zip into your Rack v2 plugins folder [Rack v2 -> Help -> Open User Folder -> plugins] and restart Rack)*

![enter image description here](https://user-images.githubusercontent.com/97259710/153781732-621218eb-17e7-4c0c-9372-a12431984f65.jpg)

 - **Count** - two triggerable Counters
 - **Veins** - 2HP clone of Bernoulli Gate (Audible Instruments)
 - **Comparator** - Compare a (poly) input against a voltage. Outputs Gates and Voltage Divisions.
 - **Debug light** - 2HP clone of Debug (computerscare)
 - **Poly Tools** - Poly merger, Cloner (basic, global offset, voltage spread), Poly to Poly L & R (i.e. MindMeld), Poly SUM to Mono (w.
   gain), Redact total Channels of a Poly Cable
 - **Switch** - 2:1, 2:1, 1:2, 1:2 gated switch.. currently no latching mode (coming soon)
 - **Morph** - Crossfade between Channels of a Poly Cable (top & bottom), Crossfade between two (poly) Signals (mid).
 - **N1** - Poly to Mono Sequencers / Switches. Normalled SeqSwitches which can operate independently. 3 Modes (Forwrd. Backwrd. Random). Takes a Poly Signal and Sequences their Channels. Bottom Out is a Poly of all 4 Sequencers.
 - **SEQ16** - Trigger, Gate Sequencer & Voltage Switch. TRG out for each active step, adjacent steps are turned to Gates at GT out. CV in takes a Poly Signal and outputs its Channels on the respective active Steps (step 8 = channel 8).. 2 Modes: S&H (Samples CH in at CLK in) & Switch (routes original Signal out). 3 Sequencing Modes (Fw, Rw, Rd). Length knob, Reset in, End of Cycle out.
 - **DivSEQ** -  Clock Division Trigger Sequencer. 4 Clock Divisions (1/1 to 1/32 for now, w. on/off for each), global Probability, OR & XOR output. CV input is a bit buggy still: takes trigs/clk/gt on channels 1-4 to turn divider 1-4 on/off.
 - **CRG** - Count Modulas Clocked Random Gates in 2HP. With channel selector, probability knobs for 8ch, Multi/Solo mode, Gate/Trig/Clock outputs.
 - **Flamm** - Count Modulas Burst Generator in 2HP. Internal & External CLKing, # of pulses raised to 24, pulse probability, Retrigger button (on/off), Pulses/Start of Cycle/End of Cycle Trig outs & Duration of Cycle Gate out.
 - **CV Bank** - manual below
 - **SHSeq** - manual below
 - **Mono to Poly Shift Register** - takes a Mono input with a Trigger and saves the Signals to Channels of its Poly output. 16 Signals max. A built in N1 Sequencer can resequence it to Mono again. There is a lot of use for this - especially in generative music production. Seriously. Get creative.
 - **DRONE** - 1-8 Voice(s) with addable Drift (Strength & Speed). Basenote selector (Oct & Semi) or vOct in (CV in). Each voice has its own offset from -8 to +8 Semitones.
 - **NOISE** - Dust module with Low-Pass-Filter, Gain and Random Clock out (great to drive a S&H to modulate filter, gain and rate). External Clock in is working, but unsatisfying. This is basically an experimental noise or voltage generator at this point.
 - **Merge / Split** - Merge 16:1 & 8:1 with right-click menu to choose total channels (default [0]: auto), Split 1:16, 1:8 with LEDs reflecting the amount of active channels from the input.
xxxxxxxxxxxxxxxxx
**CVBANK  / Voltage Bank**
 - 2HP, 2 sections of voltage storages with targeted overwrite and cv
   modulation, polyphonic (16cv per bank)
 - INPUT takes a polyphonic cable (max 16ch)
 - GRAB button saves a snapshot of the INPUT signals (Sample&Hold)
 - POLARITY button, default: -10|+10 || in unipolar mode will be 0|+10
 - CV input takes polyphonic modulation or trigger/gates and correspond
   with trimpot
 - TRIMPOT sets amount of modulation influence*
 - TRIGGER input takes polyphonic cable and can be used for targeted
   overwrite of stored voltages
 - OUTPUT outputs the current state of stored voltages this is a
   derivative of unless modules "TOWERS"

**CVBANK** is an advanced Sample+Hold module.  up until voltage is stored it works exactly as such. 
once you have grabbed an input signal there are some additional features expanding on the concept of S&H modules. if you have copied your input to storage (with GRAB) you can remove the cable.  
the output now holds an exact copy of the input signals state at the moment of grabberingo.    
with voltages stored, the INPUT takes on an additional role.  
put a different set of voltages to the input and open the TRIMPOT (CW/CCW)
the stored voltages and the input gradually merge at lower positions (CW 1-3; CCW 11-9)
and transform radically at higher positions (CW 3-5; CCW 9-7)
depending on the difference of storage and input if your input has moving voltages, the modulated value in the storage will keep changing along modulation between storage and input are non volatile - closing the trimpot or removing the input signal returns the storage back to its original state.  

**CV INPUT/CV MODULATION**

 - if voltage is stored, differing input connected, you can add a
   monophonic (applies to all channels) modulation source or a
   polyphonic signal (targets its specific channel) into the CV input.
 - if the TRIMPOT is closed, no modulation is applied.
 - the TRIMPOT functions as a scaler of the incoming modulation
 - whereas 12 o'clock is 0 and turns modulation off, FULL CW|CCW equals
   +10   try to add a simple LFO to the CV INPUT and gradually open the TRIMPOT   the LFO morphs values between stored voltages and input voltages, clamped/ranged by the TRIMPOT position and depending on the gap between storage and input.

  
**TRIGGER INPUT**
a monophonic signal to the TRIGGER INPUT is synonymous to pushing the GRAB BUTTON
and will permanently (volatile) overwrite the stored voltages plugging in a polyphonic signal will switch to targeted overwrites.  In polyphony a trigger/signal only overwrites the stored voltage in its respective channel.  
the TRIGGER INPUT reacts to all sorts of signals (TRG, PULSE, CLK, LFO[SINE|TRI|SAW|SQR]) - as long as they have an edge.
TRIGGERS can overwrite stored voltages while MODULATION is active.  
  
CVBANKs storage is, unless manually overwritten, permanent. unlike commonplace S&H modules it writes the voltages into parameters instead of caching them. thus CVBANKs state is autosaved on close, along with patch on manual save, can be saved as preset,  persists in copies of the module and can be saved and triggered by 8FACE/TRANSIT/STRIP modules from stoermelder.
in conjunction with various modules from this collection CVBANK can be used as a CV SEQUENCE via N1 Seq.
  
*TIP:
use the bottom part of CVBANK to store a backup of your storage.  
(top output to bottom input, push bottom grab button, remove cable)
or simply use 8FACE (map CVBANK, long-press second preset knob on 8FACE to deactivate all others, save CVBANK in preset 1 & 2. send any trigger to 8FACE to restore storage)
when working with generative sequencing, CVBANK or rather SHSEQ (CVBANK + N1) is perfect to generate an evolving sequence with repetitive elements using random inputs, alternating CV modulation signals and overwrite single values with probability.* 


**SHSEQ**

 - 2HP, CVBANK paired with N1, polyphonic
 - CVBANK [TOP] see CVBANK manual entry;
 - N1 [BOTTOM] all in- and outputs of N1 are monophonic at this stage.
 - the CVBANK storage is internally routed into N1 and is lined up(FORWARDS, BACKWARDS, CVADDRESSABLE, RANDOM) as a monophonic voltage array.  N1 translates a polyphonic signal into a sequence.
 - CV INPUT takes a range of around 10v (+10||-10) to scan through 16 channels - the math is a bit off [TO-DO].   the amount of signals stored in the voltage bank is divided by 10 and sets the step size between channels. the division is set to "rotate" the channels seamlessly across -10|+10.
*however: at 16 for example the division puts channel 1 at -0.14v and again at 9.4v while channel 2 (step 2) is at 0.0v.*  
*the order is a bit jumbled.*
 - [+]INCREASE INPUT takes triggers, gates, clocks to advance the sequence forwards.
 - [-]DECREASE INPUT takes triggers, gates, clocks to devance the sequence backwards.
 - [rnd]INPUT takes triggers, gates, clocks to jumbley jump to a random step in the sequence.
 - [rst]INPUT take triggers, gates, clocks to reset the sequence to step1 (channel1/stored voltage1).
 - {step]OUTPUT gives the CV SEQUENCE as a monophonic signal, one
   voltage at a time.
 - [trig]OUTPUT sends a pulse whenever the sequence is moved via [+] [-]
   or [rnd] INPUT.   it does not react to CVINPUT step selections.

*Tip:
SHSEQ is a great ambient sequencer.  
Paired with random generators (like S&H, Turing, semi-random sequencers like Neutrinode, JWSeq etc.)  switches, merges, TRANSIT, 8FACE, counters/clock dividers and probability gates  
anything can pair up with SHSEQ to add and morph notes.
use CV2SEQ (Mono to Poly Shift Register) to generate poly signals from monophonic sequences and feed them to SHSEQ.*
