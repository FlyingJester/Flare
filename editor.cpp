#include "editor.hpp"

#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Buffer.H> 
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl.H>

#include <FL/Enumerations.H>

#include <FL/fl_ask.H>

#include <zlib.h>

#include <string>
#include <cstdio>
#include <cassert>

// Stuff for editor filetype registration
#include "text_editor.hpp"
#include <deque>

namespace Flare {

Editor::~Editor(){
    
}

Editor::Editor(int x, int y, int w, int h)
  : holder(x, y, w, h)
  , adler(adler32(0L, nullptr, 0)){

}

// Editor filetype registry.
struct filetype{
    char extension[0x10];
    Editor::EditorFactory factory;
} default_editor;

static inline bool ConstructFiletype(const char * const c, const unsigned l, const Editor::EditorFactory f, struct filetype &t){
    t.factory = f;
    // Since 0xF is contiguous set bits, a bitwise and will perform a min operation.
    const unsigned n = l&0xF;
    memcpy(t.extension, c, n);
    t.extension[n] = 0;
    return n==l;
}

static inline bool ConstructFiletype(const std::string &extension, const Editor::EditorFactory factory, struct filetype &new_filetype){
    return ConstructFiletype(extension.c_str(), extension.size(), factory, new_filetype);
}

static std::deque<struct filetype> filetypes;

bool Editor::RegisterFiletype(const std::string &extension, Editor::EditorFactory factory){

    const char *ext = extension.c_str();
    unsigned len = extension.size();

    if(ext[0]=='.'){
        ext++;
        len--;
    }

    for(std::deque<struct filetype>::iterator i = filetypes.begin(); i!=filetypes.end(); i++)
        if(i->extension==ext){
            i->factory = factory;
            return true;
        }
    struct filetype new_filetype;
    if(ConstructFiletype(ext, factory, new_filetype)){
        filetypes.push_back(new_filetype);
        return true;
    }
    return false;
}

bool Editor::RegisterDefaultEditor(EditorFactory factory){
    return default_editor.factory = factory;
}

Editor::EditorFactory Editor::GetDefaultEditor(){
    return default_editor.factory;
}

bool Editor::RestoreDefaultEditor(){
    return default_editor.factory = TextEditor::CreateTextEditor;
}

Editor::EditorFactory Editor::GetEditorForExtension(const std::string &extension){
    const char *ext = extension.c_str();
    unsigned len = extension.size();

    if(ext[0]=='.'){
        ext++;
        len--;
    }

    for(std::deque<struct filetype>::iterator i = filetypes.begin(); i!=filetypes.end(); i++)
        if(i->extension==ext)
            return i->factory;
    return default_editor.factory;
}

} // namespace Flare
