/* ===================================================================== *
 * GeneralMidi.cpp (MeV/Midi)
 * ===================================================================== */

#include "GeneralMidi.h"

const char *
GM_PROGRAM_NAMES[128] =
{
	"Acoustic Grand",
	"Bright Grand",
	"Electric Grand",
	"Honky Tonk",
	"Electric Piano 1",
	"Electric Piano 2",
	"Harpsichord",
	"Clavichord",
	"Celesta",
	"Glockenspiel",
	"Music Box",
	"Vibraphone",
	"Marimba",
	"Xylophone",
	"Tubular Bells",
	"Dulcimer",
	"Drawbar Organ",
	"Percussive Organ",
	"Rock Organ",
	"Church Organ",
	"Reed Organ",
	"Accordian",
	"Harmonica",
	"Tango Accordian",

	// Guitars
	"Acoustic Guitar Nylon",
 	"Acoustic Guitar Steel",
 	"Electric Guitar Jazz",
 	"Electric Guitar Clean",
 	"Electric Guitar Muted",
 	"Overdriven Guitar",
 	"Distortion Guitar",
 	"Guitar Harmonics",

	// Basses
 	"Acoustic Bass",
 	"Electric Bass Finger",
 	"Electric Bass Pick",
 	"Fretless Bass",
 	"Slap Bass 1",
 	"Slap Bass 2",
 	"Synth Bass 1",
 	"Synth Bass 2",

	// Strings
 	"Violin",
 	"Viola",
 	"Cello",
 	"Contrabass",
 	"Tremolo Strings",
 	"Pissicato Strings",
 	"Orchestral Strings",
 	"Timpani",

	// Ensemble strings and voices
 	"String Ensemble 1",
 	"String Ensemble 2",
 	"Synth Strings 1",
 	"Synth Strings 2",
 	"Voice Aah",
 	"Voice Ooh",
 	"Synth Voice",
 	"Orchestra Hit",

	// Brass
 	"Trumpet",
 	"Trombone",
 	"Tuba",
 	"Muted Trumpet",
 	"French Horn",
 	"Brass Section",
 	"Synth Brass 1",
 	"Synth Brass 2",

	// Reeds
 	"Soprano Sax",
 	"Alto Sax",
 	"Tenor Sax",
 	"Baritone Sax",
 	"Oboe",
 	"English Horn",
 	"Bassoon",
 	"Clarinet",

	// Pipes
 	"Piccolo",
 	"Flute",
 	"Recorder",
 	"Pan Flute",
 	"Blown Bottle",
 	"Shakuhachi",
 	"Whistle",
 	"Ocarina",

	// Synth Leads
 	"Square Lead",
 	"Sawtooth Lead",
 	"Calliope Lead",
 	"Chiff Lead",
 	"Charang Lead",
 	"Voice Lead",
 	"Fifths Lead",
 	"Bass Lead",

	// Synth Pads
 	"New Age Pad",
 	"Warm Pad",
 	"Polysynth Pad",
 	"Choir Pad",
 	"Bowed Pad",
 	"Metallic Pad",
 	"Halo Pad",
 	"Sweep Pad",

	// Effects
 	"Rain FX",
 	"Soundtrack FX",
 	"Crystal FX",
 	"Atmosphere FX",
 	"Brightness FX",
 	"Goblins FX",
 	"Echoes FX",
 	"Sci-Fi FX",

	// Ethnic
 	"Sitar",
 	"Banjo",
 	"Shamisen",
 	"Koto",
 	"Kalimba",
 	"Bagpipe",
 	"Fiddle",
 	"Shanai",

	// Percussion
 	"Tinkle Bell",
 	"Agogo",
 	"Steel Drums",
 	"Woodblock",
 	"Taiko Drums",
 	"Melodic Tom",
 	"Synth Drum",
 	"Reverse Cymbal",

	// Sound Effects
 	"Fret Noise",
 	"Breath Noise",
 	"Seashore",
 	"Bird Tweet",
 	"Telephone",
 	"Helicopter",
 	"Applause",
 	"Gunshot"
};

const char *
GeneralMidi::GetProgramNameFor(
	uint8 program)
{
	return GM_PROGRAM_NAMES[program];
}

// END - GeneralMidi.cpp
