# pico-gcc-lag-tester

This project is a Melee setup lag test measurement device that's oriented at advanced measurements rather than merely outputting the average.

It involves connecting a Male GC port and a phototransistor circuit to a Raspberry Pi Pico. The Pico will pass as a Gamecube controller over that GC port,
and once a measure is started over USB, the Pico will input something that will make the screen change from black to white in Melee. It will then measure the
light intensity read by the phototransistor every 0.2ms for the next 200ms, and will report each individual measure over USB. (an overnight measure yields
several hundreds of KBs of data)

That raw data is then to be processed by a Python script. The raw data allows for more in depth research such as statistical distribution, remanence time or 
latency changing over time.

As soon as it's powered, the Pico will attempt to report light intensities, but it will wait for its USB message to be accepted before moving on. So, starting 
the PC script effectively "starts" a measure.

As for how we make the screen go from black to white, we use the 20XX Training Pack (or anything with a develop mode access, for that matter). When in-game on 
that ISO with develop mode turned on (accessed by pause + dpadRight(s) + X+dpadUp to leave), X+dpadDown makes the background switch between pure black, normal, 
pure black, and pure white. We want to measure black > white; this is built into the lag tester and it will adjust the number of X+dpadDown to press to measure it 
if it doesn't think that it's what it's measuring. (The check is that we go from > 3800 neg light to < 3800 neg light)

In order to have useful absolute lag measures, the Gecko Codes configuration should be equivalent to that of your normal Melee ISO. If you just want to see how 
one configuration compares to another though (such as comparing different adapter ports..) you might not care.

The project is composed of 3 softwares:
- The Pico gcc lag tester firmware.
- The PC application that talks to the Pico and dumps the light measurement.
- The python script necessary to process the results.

# Circuit

You will need:

- A Raspberry Pi Pico.
- A phototransistor (note: photo**transistor**, not photoresistor) that's receptive to visible light. Exemple https://www.amazon.com/HiLetgo-Phototransistor-Photosensitive-Sensitive-Sensors/dp/B00M1PMHO4/ref=sr_1_1_sspa
- A 10 to 22K resistor. The higher the resistance, the more accurate low light measures are, but the lowest the light amount you can take before saturating is. So
use 10 if your screen is very bright and 22 otherwise (you can adjust your screen brightneess/contrast when measuring but keep in mind you need to always use the same 
configuration to have meaningful comparisons between your measures)
- A male GC port connected electrically to the Pico.

This is the phototransistor part of the circuit:
//TODO

The photodiode should be soldered using super long wires (>1m, I advise AWG30). Then, make it so the photodiode is stable at ~half the height of your screen, pointing
towards the screen.

//TODO Picture

As for the GC male port, it goes on GPIO22.

Connect the 3v to VSYS, don't connect the 5v, connect the ground to a ground, connect AGND to a ground (for that last one I'm not sure it's necessary)

After uploading the .uf2, you should see the lag tester appear in Zadig with WinUSB selected (VID/PID A57E/0002).

Gecko codes //TODO

Open the 20XX Training Pack, select DF or BF, launch a game, pause, unzoom, activate develop codes, switch the background to white using X+DpadDown.

Launch the application. It shouldn't terminate instantly but should create a "Lag test <time>" file in the folder it's launched from. The screen should start blinking white and black very fast.
Click on Dolphin again to let it have exclusive fullscreen. Put a shirt or something on the photodiode to isolate it from external light sources (or the whole screen while you're at it).
There's no limit on how long you can let it run (I let it run overnight, yields ~100K measures)
  
Once you have the measure, use the python script and replace "filename" by the name of the file (and replace the path if it's the first time you launch it).
Then run the full script. It should create:
  - A graph showing the evolution of lag through time (X axis is # measures, Y axis is lag in ms)
  - A text output with the name of the file, number of measures and the average latency (until 3800, so not properly accounting for remanence time / or send me the file)
  
  
