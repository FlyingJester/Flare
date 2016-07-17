import os
import sys

flare_files = ["editor.cpp", "text_editor.cpp", "editor_window.cpp", # Main UI files
    "size_utilities.cpp", # Utilities
    "flare_text_editor_widget.cpp", "find.cpp"] # Widgets

flare_libs = ["fltk", "fltk_images", "z"]

if os.name=="posix" and not sys.platform == "darwin":
    flare_libs += ["X11", "Xft", "fontconfig", "Xfixes", "Xext", "Xinerama"]
if os.name=="posix":
    flare_libs += ["dl"]

flare = Program("flare", flare_files, LIBS = flare_libs, CCFLAGS = " -g -std=c++11 ", FRAMEWORKS = ["Cocoa"], LIBPATH=["lib"], CPPPATH=["include"])
