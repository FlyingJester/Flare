#include "text_editor.hpp"
#include "size_utilities.hpp"

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
#include <cstring>
#include <cstdio>
#include <cassert>

namespace Flare {

// Shorthand.
static inline Fl_Text_Buffer *CreateTextBuffer(){ return new Fl_Text_Buffer(0x100, 0x100); }

TextEditor::TextEditor(int x, int y, int w, int h) 
  : Editor(x, y, w, h)
  , editor(x, y, w, h){

    editor.buffer(CreateTextBuffer());
    editor.textfont(FL_SCREEN);

    editor.buffer()->add_modify_callback(Text_Editor_Widget::text_buffer_change_cb, &editor);

    holder.resizable(editor);
    holder.end();
}

TextEditor::~TextEditor(){

}

// Basically dump what we know.
void TextEditor::info() const {
    char buffer[8];
    unsigned long long s = editor.buffer()->length();
    fl_alert("Editor information:\npath: %s\nFilesize: %s %cB\nAdler32 Checksum: %lu\n", 
        path().c_str(), sizeNumberString(buffer, s), sizePrefixChar(s), adler);
}

bool TextEditor::load(){

    FILE *that = fopen(path_.c_str(), "r");
    if(!that){
        fl_alert("Cannot open file %s", path_.c_str());
        return false;
    }
    // Buffer the file in.
    char buffer[0x100];
    unsigned to = 0;

    adler = adler32(0L, nullptr, 0);

    // Clear the buffer
    editor.buffer()->text(nullptr);
    // Load the file.
    do{
        to = fread(buffer, 1, 0xFF, that);

        adler = adler32(adler, (unsigned char *)buffer, to);

        buffer[to] = 0;
        editor.buffer()->append(buffer);
    }while(to==0xFF);

    fclose(that);

    editor.clearHistory();

    return true;
}

bool TextEditor::save(){

    char buffer[0x100];

    unsigned to = 0;

    // Check if the file contents are what we saw when we last loaded/saved the file.
    uLong adler_file = adler32(0L, nullptr, 0);

    FILE *that = fopen(path_.c_str(), "r");
    // Load the file.
    do{
        to = fread(buffer, 1, 0xFF, that);

        adler_file = adler32(adler_file, (unsigned char *)buffer, to);
    }while(to==0xFF);

    fclose(that);

    if(adler_file!=adler){
        if(!fl_choice("File %s was changed outside of the editor. Would you like to save anyway?", 
            fl_cancel, fl_yes, nullptr, path_.c_str()))
            return false;
    }

    // Create a backup of the original file, just in case :)
    const std::string new_file_name = path_+".baku";
    rename(path_.c_str(), new_file_name.c_str());

    that = fopen(path_.c_str(), "w");
    if(that){

        const char *text = editor.buffer()->text();
        const unsigned length = strlen(text);
        // Try to write the file.
        if(fwrite(text, 1, length, that)!=length){
            fl_alert("Could not save file %s.", path_.c_str());
            return false;
        }

        fclose(that);
        // If we succeeded, we don't need the backup any more.
        remove(new_file_name.c_str());

        // Calculate the new checksum and free the buffer.
        adler = adler32(adler32(0L, nullptr, 0), (unsigned char *)text, strlen(text));

        free((void *)text);
    }
    else{ // Alert if we couldn't open the file for saving.
        fl_alert("Could open file %s for saving.", path_.c_str());
        return false;
    }

    return true;
}

void TextEditor::find(const char *text){
    const unsigned len = strlen(text);
    int cur_pos = editor.insert_position()+1, to = cur_pos+1;
    while(editor.buffer()->findchar_forward(cur_pos, text[0], &to) && (to+len<editor.buffer()->length())){
        if(memcmp(editor.buffer()->address(to), text, len)==0){
            editor.buffer()->highlight(to, to+len);
            editor.insert_position(to);
            editor.show_insert_position();
            editor.redraw();
            return;
        }
        cur_pos = to+1;
    }
    fl_alert("Could not find text:\n%s", text);
}

void TextEditor::calculateAdler32(){
    const char *text = editor.buffer()->text();
   
    // Calculate the new checksum and free the buffer.
    adler = adler32(adler32(0L, nullptr, 0), (unsigned char *)text, strlen(text));

    free((void *)text);
}

void TextEditor::infoCallback(Fl_Widget *w, void *a){
    Editor *ed = static_cast<Editor *>(a);
    ed->info();
}

void TextEditor::saveCallback(Fl_Widget *w, void *a){
    Editor *ed = static_cast<Editor *>(a);
    ed->save();
}

void TextEditor::saveAsCallback(Fl_Widget *w, void *a){
    Editor *ed = static_cast<Editor *>(a);
    const char *file_name = fl_input("Choose a file", ed->path().c_str());
    if(file_name){
        ed->path(file_name);
        ed->save();
    }
}

void TextEditor::loadCallback(Fl_Widget *w, void *a){
    Editor *ed = static_cast<Editor *>(a);
    
    if(!ed->path().empty()){
        switch(fl_choice("Save changes to file %s?", fl_no, fl_yes, fl_cancel, ed->path().c_str())){
            case 0:
            break;
            case 1:
            ed->save();
            break;
            case 2:
            return;
        }
    }

    const char *file_name = fl_input("Choose a file", nullptr);
    if(file_name){
        ed->path(file_name);
        ed->load();
    }
}


#define MENU_SIZE 10
#define MENU_DUMMY (void *)0xDEAD

static const Fl_Menu_Item menu_[MENU_SIZE] = {
    {"File", 0, 0, 0, FL_SUBMENU},
        {"Open", FL_COMMAND+'o', TextEditor::loadCallback, MENU_DUMMY},
        {"Save", FL_COMMAND+'s', TextEditor::saveCallback, MENU_DUMMY},
        {"Save As", FL_COMMAND+FL_SHIFT+'s', TextEditor::saveAsCallback, MENU_DUMMY},
    {0},
        {"Edit", 0, 0, 0, FL_SUBMENU},
        {"Properties", FL_COMMAND+'h', TextEditor::infoCallback, MENU_DUMMY},
        {"Find", FL_COMMAND+'f', 0, MENU_DUMMY},
    {0},
{0}
};

Fl_Menu_Item *TextEditor::menu(){
    Fl_Menu_Item *that = (Fl_Menu_Item *)malloc(sizeof(Fl_Menu_Item)*MENU_SIZE);
    memcpy(that, menu_, sizeof(menu_));
    return that;
}

const Fl_Menu_Item *TextEditor::prepareMenu(void(*OpenCallback_)(Fl_Widget *, void *a), void(*FindCallback_)(Fl_Widget *, void *a), void *arg_) const{
    Fl_Menu_Item *m = menu();
    for(int i = 0; i<MENU_SIZE; i++){
        if(m[i].user_data()==MENU_DUMMY)
            m[i].user_data((void *)this);
    }

    m[1].callback(OpenCallback_);
    m[1].user_data(arg_);
    m[7].callback(FindCallback_);
    m[7].user_data(arg_);
    return m;
}

Editor *TextEditor::CreateTextEditor(int x, int y, int w, int h){
    return new TextEditor(x, y, w, h);
}

} // namespace Flare
