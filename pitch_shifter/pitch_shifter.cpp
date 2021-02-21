#include "daisy_petal.h"
#include "daisysp.h"
#include "terrarium.h"

using namespace daisy;
using namespace daisysp;
using namespace terrarium;
#include <math.h>

// Declare a local daisy_petal for hardware access
DaisyPetal hw;
Parameter volume, dry, wet, trans, wet2, trans2;
PitchShifter ps, ps2;
bool bypass;

Led led1, led2;

// This runs at a fixed rate, to prepare audio samples
void callback(float *in, float *out, size_t size)
{
    hw.ProcessAllControls();
    led1.Update();
    led2.Update();


    float transpose = floor((trans.Process() * 100.0f) / 7.0f);
    float transpose2 = floor((trans2.Process() * 100.0f) / 7.0f);
    if (hw.switches[Terrarium::SWITCH_1].Pressed()) {
        ps.SetTransposition(12.0f + transpose);
    } else {
        ps.SetTransposition(transpose);
    }

    if (hw.switches[Terrarium::SWITCH_2].Pressed()) {
        ps2.SetTransposition(12.0f + transpose2);
    } else {
        ps2.SetTransposition(transpose);
    }


    // (De-)Activate bypass and toggle LED when left footswitch is pressed
    if(hw.switches[Terrarium::FOOTSWITCH_1].RisingEdge())
    {
        bypass = !bypass;
        led1.Set(bypass ? 0.0f : 1.0f);
    }

    for(size_t i = 0; i < size; i += 2)
    {
        float dryl, dryr, shiftedl, shiftedr, shiftedl2, shiftedr2;
        dryl  = in[i];
        dryr  = in[i + 1];

        shiftedl = ps.Process(dryl);
        shiftedr = ps.Process(dryr);

        shiftedl2 = ps2.Process(dryl);
        shiftedr2 = ps2.Process(dryr);

        if(bypass)
        {
            out[i]     = in[i];     // left
            out[i + 1] = in[i + 1]; // right
        }
        else {
            out[i] = (dryl * dry.Process() + shiftedl * wet.Process() + + shiftedl2 * wet2.Process()) * volume.Process() * 2.5f;
            out[i + 1] = (dryr * dry.Process() + shiftedr * wet.Process() + shiftedr2 * wet2.Process()) * volume.Process() * 2.5f;
        }
    }
}

int main(void)
{
    float samplerate;

    hw.Init();
    samplerate = hw.AudioSampleRate();
    hw.SetAudioBlockSize(12);

    volume.Init(hw.knob[Terrarium::KNOB_6], 0.01f, 0.999f, Parameter::LOGARITHMIC);
    dry.Init(hw.knob[Terrarium::KNOB_3], 0.01f, 0.999f, Parameter::LOGARITHMIC);
    wet.Init(hw.knob[Terrarium::KNOB_1], 0.01f, 0.999f, Parameter::LOGARITHMIC);
    wet2.Init(hw.knob[Terrarium::KNOB_2], 0.01f, 0.999f, Parameter::LOGARITHMIC);
    trans.Init(hw.knob[Terrarium::KNOB_4], 0.0001f, 0.91f, Parameter::LINEAR);
    trans2.Init(hw.knob[Terrarium::KNOB_5], 0.0001f, 0.91f, Parameter::LINEAR);


    // Set samplerate for your processing like so:
    ps.Init(samplerate);
    ps2.Init(samplerate);

    // Init the LEDs and set activate bypass
    led1.Init(hw.seed.GetPin(Terrarium::LED_1),false);
    led1.Update();
    bypass = true;

    hw.StartAdc();
    hw.StartAudio(callback);
    while(1) {}
}