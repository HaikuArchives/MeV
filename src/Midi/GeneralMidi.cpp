/* ===================================================================== *
 * GeneralMidi.cpp (MeV/Midi)
 * ===================================================================== */

#include "GeneralMidi.h"

const char *
GM_PROGRAM_NAMES[128] =
{
	// Piano
	"Acoustic Grand",
	"Bright Acoustic",
	"Electric Grand",
	"Honky-Tonk",
	"Electric Piano 1",
	"Electric Piano 2",
	"Harpsichord",
	"Clavichord",

	// Chromatic Percussion
	"Celesta",
	"Glockenspiel",
	"Music Box",
	"Vibraphone",
	"Marimba",
	"Xylophone",
	"Tubular Bells",
	"Dulcimer",

	// Organ
	"Drawbar Organ",
	"Percussive Organ",
	"Rock Organ",
	"Church Organ",
	"Reed Organ",
	"Accordian",
	"Harmonica",
	"Tango Accordian",

	// Guitar
	"Acoustic Guitar Nylon",
 	"Acoustic Guitar Steel",
 	"Electric Guitar Jazz",
 	"Electric Guitar Clean",
 	"Electric Guitar Muted",
 	"Overdriven Guitar",
 	"Distortion Guitar",
 	"Guitar Harmonics",

	// Bass
 	"Acoustic Bass",
 	"Electric Bass Finger",
 	"Electric Bass Pick",
 	"Fretless Bass",
 	"Slap Bass 1",
 	"Slap Bass 2",
 	"Synth Bass 1",
 	"Synth Bass 2",

	// Solo Strings
 	"Violin",
 	"Viola",
 	"Cello",
 	"Contrabass",
 	"Tremolo Strings",
 	"Pissicato Strings",
 	"Orchestral Strings",
 	"Timpani",

	// Ensemble
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

	// Reed
 	"Soprano Sax",
 	"Alto Sax",
 	"Tenor Sax",
 	"Baritone Sax",
 	"Oboe",
 	"English Horn",
 	"Bassoon",
 	"Clarinet",

	// Pipe
 	"Piccolo",
 	"Flute",
 	"Recorder",
 	"Pan Flute",
 	"Blown Bottle",
 	"Shakuhachi",
 	"Whistle",
 	"Ocarina",
 
	// Synth Lead
 	"Square Lead",
 	"Sawtooth Lead",
 	"Calliope Lead",
 	"Chiff Lead",
 	"Charang Lead",
 	"Voice Lead",
 	"Fifths Lead",
 	"Bass Lead",

	// Synth Pad
 	"New Age Pad",
 	"Warm Pad",
 	"Polysynth Pad",
 	"Choir Pad",
 	"Bowed Pad",
 	"Metallic Pad",
 	"Halo Pad",
 	"Sweep Pad",

	// Synth Effects
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

	// Percussive
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

#define GM_DRUM_SOUND_FIRST 35
#define GM_DRUM_SOUND_LAST 81

const char *
GM_DRUM_SOUND_NAMES[] =
{
	"Acoustic Bass Drum",
	"Bass Drum 1",
	"Side Stick",
	"Acoustic Snare",
	"Hand Clap",
	"Electric Snare",
	"Low Floor Tom",
	"Closed Hi-Hat",
	"High Floor Tom",
	"Pedal Hi-Hat",
	"Low Tom",
	"Open Hi-Hat",
	"Low-Mid Tom",
	"Hi-Mid Tom",
	"Crash Cymbal 1",
	"High Tom",
	"Ride Cymbal 1",
	"Chinese Cymbal",
	"Ride Bell",
	"Tambourine",
	"Splash Cymbal",
	"Cowbell",
	"Crash Cymbal 2",
	"Vibraslap",
	"Ride Cymbal 2",
	"Hi Bongo",
	"Low Bongo",
	"Mute Hi Conga",
	"Open Hi Conga",
	"Low Conga",
	"High Timbale",
	"Low Timbale",
	"High Agogo",
	"Low Agogo",
	"Cabasa",
	"Maracas",
	"Short Whistle",
	"Long Whistle",
	"Short Guiro",
	"Long Guiro",
	"Claves",
	"Hi Wood Block",
	"Low Wood Block",
	"Mute Cuica",
	"Open Cuica",
	"Mute Triangle",
	"Open Triangle"
};

const char *
GeneralMidi::GetProgramNameFor(
	uint8 program)
{
	return GM_PROGRAM_NAMES[program];
}

const char *
GeneralMidi::GetDrumSoundNameFor(
	uint8 note)
{
	if ((note < GM_DRUM_SOUND_FIRST) || (note > GM_DRUM_SOUND_LAST))
		return "";

	return GM_DRUM_SOUND_NAMES[note - GM_DRUM_SOUND_FIRST];
}

// END - GeneralMidi.cpp
