NAME= MeV
TYPE= APP


SRCS= \
	MeVApp.cpp \
	MeVDoc.cpp \
	MeVPlugin.cpp \
	Framework/AppWindow.cpp \
	Framework/DocApp.cpp \
	Framework/Document.cpp \
	Framework/DocWindow.cpp \
	Framework/DynamicMenu.cpp \
	Framework/ItemList.cpp \
	Framework/Observable.cpp \
	Framework/Observer.cpp \
	Framework/RecentDocumentsMenu.cpp \
	Framework/RefCount.cpp \
	Framework/ScreenUtils.cpp \
	Framework/SharedLock.cpp \
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
	Engine/Track.cpp \
	Midi/GeneralMidi.cpp \
	Midi/InternalSynth.cpp \
	Midi/MidiDeviceInfo.cpp \
	Midi/MidiManager.cpp \
	Midi/PortNameMap.cpp \
	Midi/ReconnectingMidiProducer.cpp \
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
	UI/ContinuousValueEditor.cpp \
	UI/CursorCache.cpp \
	UI/DestinationListView.cpp \
	UI/DestinationModifier.cpp \
	UI/EventEditor.cpp \
	UI/GridWindow.cpp \
	UI/IconMenuItem.cpp \
	UI/InspectorWindow.cpp \
	UI/LinearEditor.cpp \
	UI/LinearWindow.cpp \
	UI/MenuTool.cpp \
	UI/MultiColumnListView.cpp \
	UI/OperatorWindow.cpp \
	UI/PitchBendEditor.cpp \
	UI/Polygon.cpp \
	UI/PreferencesWindow.cpp \
	UI/QuickKeyMenuItem.cpp \
	UI/RulerView.cpp \
	UI/Scroller.cpp \
	UI/Spinner.cpp \
	UI/Splitter.cpp \
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
	UI/TransportWindow.cpp \
	UI/VelocityEditor.cpp

#	specify the resource files to use
#	full path or a relative path to the resource file can be used.
RSRCS= \
	../resources/Bitmaps.rsrc \
	../resources/MeV.rsrc \
	../resources/Cursor.rsrc

# @<-src@ 
#%}

#	end support for Pe and Eddie

#	specify additional libraries to link against
#	there are two acceptable forms of library specifications
#	-	if your library follows the naming pattern of:
#		libXXX.so or libXXX.a you can simply specify XXX
#		library: libbe.so entry: be
#		
#	- 	if your library does not follow the standard library
#		naming scheme you need to specify the path to the library
#		and it's name
#		library: my_lib.a entry: my_lib.a or path/my_lib.a
LIBS= be media midi midi2 root tracker translation ../lib/libprefs/bigprefs.x86.a

#	specify additional paths to directories following the standard
#	libXXX.so or libXXX.a naming scheme.  You can specify full paths
#	or paths relative to the makefile.  The paths included may not
#	be recursive, so include all of the paths where libraries can
#	be found.  Directories where source files are found are
#	automatically included.
LIBPATHS= 

#	additional paths to look for system headers
#	thes use the form: #include <header>
#	source file directories are NOT auto-included here
SYSTEM_INCLUDE_PATHS = 

#	additional paths to look for local headers
#	thes use the form: #include "header"
#	source file directories are automatically included
LOCAL_INCLUDE_PATHS = ../lib/libprefs/src

#	specify the level of optimization that you desire
#	NONE, SOME, FULL
OPTIMIZE= 

#	specify any preprocessor symbols to be defined.  The symbols will not
#	have their values set automatically; you must supply the value (if any)
#	to use.  For example, setting DEFINES to "DEBUG=1" will cause the
#	compiler option "-DDEBUG=1" to be used.  Setting DEFINES to "DEBUG"
#	would pass "-DDEBUG" on the compiler's command line.
DEFINES= DEBUG=1 NDEBUG=0

#	specify special warning levels
#	if unspecified default warnings will be used
#	NONE = supress all warnings
#	ALL = enable all warnings
WARNINGS = 

#	specify whether image symbols will be created
#	so that stack crawls in the debugger are meaningful
#	if TRUE symbols will be created
SYMBOLS = 

#	specify debug settings
#	if TRUE will allow application to be run from a source-level
#	debugger.  Note that this will disable all optimzation.
DEBUGGER = TRUE

#	specify additional compiler flags for all files
COMPILER_FLAGS =

#	specify additional linker flags
LINKER_FLAGS =

#	specify the version of this particular item
#	(for example, -app 3 4 0 d 0 -short 340 -long "340 "`echo -n -e '\302\251'`"1999 GNU GPL") 
#	This may also be specified in a resource.
APP_VERSION = 

## include local makefile-engine
include makefile-engine

