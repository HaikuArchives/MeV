## Haiku Generic Makefile v2.6 ## 

## Fill in this file to specify the project being created, and the referenced
## Makefile-Engine will do all of the hard work for you. This handles any
## architecture of Haiku.

# The name of the binary.
NAME = MeV
TARGET_DIR = ../app

# The type of binary, must be one of:
#	APP:	Application
#	SHARED:	Shared library or add-on
#	STATIC:	Static library archive
#	DRIVER: Kernel driver
TYPE = APP

# 	If you plan to use localization, specify the application's MIME signature.
APP_MIME_SIG = 

#	The following lines tell Pe and Eddie where the SRCS, RDEFS, and RSRCS are
#	so that Pe and Eddie can fill them in for you.
#%{
# @src->@ 

#	Specify the source files to use. Full paths or paths relative to the 
#	Makefile can be included. All files, regardless of directory, will have
#	their object files created in the common object directory. Note that this
#	means this Makefile will not work correctly if two source files with the
#	same name (source.c or source.cpp) are included from different directories.
#	Also note that spaces in folder names do not work well with this Makefile.
SRCS = MeVApp.cpp \
	MeVDoc.cpp \
	MeVModule.cpp \
	MeVPlugin.cpp \
	Framework/AppWindow.cpp \
	Framework/DialogWindow.cpp \
	Framework/DocApp.cpp \
	Framework/Document.cpp \
	Framework/DocWindow.cpp \
	Framework/ItemList.cpp \
	Framework/Lockable.cpp \
	Framework/Observable.cpp \
	Framework/Observer.cpp \
	Framework/RecentDocumentsMenu.cpp \
	Framework/RefCount.cpp \
	Framework/ScreenUtils.cpp \
	Framework/Serializable.cpp \
	Framework/Undo.cpp \
	Framework/WindowState.cpp \
	Engine/Destination.cpp \
	Engine/Event.cpp \
	Engine/EventList.cpp \
	Engine/EventOp.cpp \
	Engine/EventStack.cpp \
	Engine/EventTask.cpp \
	Engine/EventTrack.cpp \
	Engine/PlaybackTask.cpp \
	Engine/PlaybackTaskGroup.cpp \
	Engine/Player.cpp \
	Engine/PlayerControl.cpp \
	Engine/SignatureMap.cpp \
	Engine/TempoMap.cpp \
	Engine/Time.cpp \
	Engine/TimeSpan.cpp \
	Engine/Track.cpp \
	Midi/DestinationConfigView.cpp \
	Midi/DestinationMonitorView.cpp \
	Midi/GeneralMidi.cpp \
	Midi/InternalSynth.cpp \
	Midi/MidiDestination.cpp \
	Midi/MidiDeviceInfo.cpp \
	Midi/MidiModule.cpp \
	Midi/MidiPortsMenu.cpp \
	Midi/PortNameMap.cpp \
	Support/BeError.cpp \
	Support/BeFileReader.cpp \
	Support/Dictionary.cpp \
	Support/DList.cpp \
	Support/IFFReader.cpp \
	Support/IFFWriter.cpp \
	Support/MathUtils.cpp \
	Support/Reader.cpp \
	Support/ResourceUtils.cpp \
	Support/Writer.cpp \
	UI/AssemblyWindow.cpp \
	UI/BitmapTool.cpp \
	UI/BorderButton.cpp \
	UI/BorderView.cpp \
	UI/ColorDialogWindow.cpp \
	UI/ColorWell.cpp \
	UI/ConsoleContainerView.cpp \
	UI/ConsoleView.cpp \
	UI/ContinuousValueEditor.cpp \
	UI/CursorCache.cpp \
	UI/DestinationListView.cpp \
	UI/DestinationView.cpp \
	UI/EventEditor.cpp \
	UI/EventRenderer.cpp \
	UI/GridWindow.cpp \
	UI/IconMenuItem.cpp \
	UI/InspectorWindow.cpp \
	UI/LinearEditor.cpp \
	UI/LinearWindow.cpp \
	UI/LoopButton.cpp \
	UI/MenuTool.cpp \
	UI/MixWindow.cpp \
	UI/MultiColumnListView.cpp \
	UI/OperatorWindow.cpp \
	UI/PitchBendEditor.cpp \
	UI/PlayPauseButton.cpp \
	UI/Polygon.cpp \
	UI/PositionInfoBar.cpp \
	UI/PreferencesWindow.cpp \
	UI/QuickKeyMenuItem.cpp \
	UI/RulerView.cpp \
	UI/Scroller.cpp \
	UI/Spinner.cpp \
	UI/Splitter.cpp \
	UI/StatusBar.cpp \
	UI/StdBevels.cpp \
	UI/StdButton.cpp \
	UI/StringEditView.cpp \
	UI/StripFrameView.cpp \
	UI/StripLabelView.cpp \
	UI/StripSplitter.cpp \
	UI/StripView.cpp \
	UI/TempoEditControl.cpp \
	UI/TextDisplay.cpp \
	UI/TextSlider.cpp \
	UI/TimeEditControl.cpp \
	UI/TimeIntervalControl.cpp \
	UI/Tool.cpp \
	UI/ToolBar.cpp \
	UI/TrackCtlStrip.cpp \
	UI/TrackListItem.cpp \
	UI/TrackListView.cpp \
	UI/TrackListWindow.cpp \
	UI/TrackWindow.cpp \
	UI/TransportButton.cpp \
	UI/TransportWindow.cpp \
	UI/VelocityEditor.cpp \
	libprefs/libprefs.cpp

#	Specify the resource definition files to use. Full or relative paths can be
#	used.
RDEFS = ../resources/Bitmaps.rdef \
	../resources/Buttons.rdef \
	../resources/Cursors.rdef \
	../resources/Icons.rdef \
	../resources/MeV.rdef \
	../resources/Midi.rdef 

#	Specify the resource files to use. Full or relative paths can be used.
#	Both RDEFS and RSRCS can be utilized in the same Makefile.
RSRCS =

# End Pe/Eddie support.
# @<-src@ 
#%}

#	Specify libraries to link against.
#	There are two acceptable forms of library specifications:
#	-	if your library follows the naming pattern of libXXX.so or libXXX.a,
#		you can simply specify XXX for the library. (e.g. the entry for
#		"libtracker.so" would be "tracker")
#
#	-	for GCC-independent linking of standard C++ libraries, you can use
#		$(STDCPPLIBS) instead of the raw "stdc++[.r4] [supc++]" library names.
#
#	- 	if your library does not follow the standard library naming scheme,
#		you need to specify the path to the library and it's name.
#		(e.g. for mylib.a, specify "mylib.a" or "path/mylib.a")
LIBS = be media midi midi2 tracker translation

#	Specify additional paths to directories following the standard libXXX.so
#	or libXXX.a naming scheme. You can specify full paths or paths relative
#	to the Makefile. The paths included are not parsed recursively, so
#	include all of the paths where libraries must be found. Directories where
#	source files were specified are	automatically included.
LIBPATHS = 

#	Additional paths to look for system headers. These use the form
#	"#include <header>". Directories that contain the files in SRCS are
#	NOT auto-included here.
SYSTEM_INCLUDE_PATHS = 

#	Additional paths paths to look for local headers. These use the form
#	#include "header". Directories that contain the files in SRCS are
#	automatically included.
LOCAL_INCLUDE_PATHS = 

#	Specify the level of optimization that you want. Specify either NONE (O0),
#	SOME (O1), FULL (O2), or leave blank (for the default optimization level).
OPTIMIZE := SOME

# 	Specify the codes for languages you are going to support in this
# 	application. The default "en" one must be provided too. "make catkeys"
# 	will recreate only the "locales/en.catkeys" file. Use it as a template
# 	for creating catkeys for other languages. All localization files must be
# 	placed in the "locales" subdirectory.
LOCALES = 

#	Specify all the preprocessor symbols to be defined. The symbols will not
#	have their values set automatically; you must supply the value (if any) to
#	use. For example, setting DEFINES to "DEBUG=1" will cause the compiler
#	option "-DDEBUG=1" to be used. Setting DEFINES to "DEBUG" would pass
#	"-DDEBUG" on the compiler's command line.
DEFINES = 

#	Specify the warning level. Either NONE (suppress all warnings),
#	ALL (enable all warnings), or leave blank (enable default warnings).
WARNINGS = ALL

#	With image symbols, stack crawls in the debugger are meaningful.
#	If set to "TRUE", symbols will be created.
SYMBOLS := 

#	Includes debug information, which allows the binary to be debugged easily.
#	If set to "TRUE", debug info will be created.
DEBUGGER := 

#	Specify any additional compiler flags to be used.
COMPILER_FLAGS = 

#	Specify any additional linker flags to be used.
LINKER_FLAGS = 

#	Specify the version of this binary. Example:
#		-app 3 4 0 d 0 -short 340 -long "340 "`echo -n -e '\302\251'`"1999 GNU GPL"
#	This may also be specified in a resource.
APP_VERSION := 

#	(Only used when "TYPE" is "DRIVER"). Specify the desired driver install
#	location in the /dev hierarchy. Example:
#		DRIVER_PATH = video/usb
#	will instruct the "driverinstall" rule to place a symlink to your driver's
#	binary in ~/add-ons/kernel/drivers/dev/video/usb, so that your driver will
#	appear at /dev/video/usb when loaded. The default is "misc".
DRIVER_PATH = 

## Include the Makefile-Engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
