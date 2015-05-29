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

namespace Flare {

// Shorthand.
static inline Fl_Text_Buffer *CreateTextBuffer(){ return new Fl_Text_Buffer(0x100, 0x100); }

enum FileSizePrefix {fsp_B, fsp_KB, fsp_MB, fsp_GB, fsp_TB};

static inline FileSizePrefix sizePrefix(unsigned long long i){
    if(i<1000u) return fsp_B;
    else if(i<1000000u) return fsp_KB;
    else if(i<1000000000ull) return fsp_MB;
    else if(i<1000000000000ull) return fsp_GB;
    else return fsp_TB; // Unlikely :)
}

static inline char sizePrefixChar(unsigned long long i){
    static const char lookup_table_[] = {' ', 'K', 'M', 'G', 'T'};
    return lookup_table_[sizePrefix(i)];
}

static inline const char *sizeNumberString(char buffer[8], unsigned long long i){
    unsigned char at = 0;
    static const unsigned long long lookup_table_[] = {1u, 100u, 100000u, 100000000ull, 100000000000ull};
    unsigned long long p = i/lookup_table_[sizePrefix(i)];
    if(p>=1000)
        buffer[at++] = ((p/1000)%10)+'0';
    if(p>=100)
        buffer[at++] = ((p/100)%10)+'0';
    if(p>=10)
        buffer[at++] = ((p/10)%10)+'0';
    buffer[at++] = '.';
    buffer[at++] = (p%10)+'0';
    buffer[at++] = 0;

    return buffer;
}

Editor::Editor(int x, int y, int w, int h) 
  : holder(x, y, w, h)
  //, menu_bar(x, y, w, 24)
  , editor(x, y, w, h)
  , adler(adler32(0L, nullptr, 0)){

    editor.buffer(CreateTextBuffer());
    editor.textfont(FL_SCREEN);

    holder.resizable(editor);
    holder.end();
}

Editor::Editor(const Editor &that)
  : holder(that.holder.x(), that.holder.y(), that.holder.w(), that.holder.h())
  //, menu_bar(that.holder.x(), that.holder.y(), that.holder.w(), 24)
  , editor(that.holder.x(), that.holder.y(), that.holder.w(), that.holder.h())
  , adler(adler32(0L, nullptr, 0))
  , path_(that.path_){

    editor.buffer(CreateTextBuffer());
    editor.textfont(FL_SCREEN);

    holder.resizable(editor);

    //menu_bar.menu(prepareMenu());

    const char *text = that.editor.buffer()->text();
    editor.buffer()->text(text);        
    adler = adler32(adler, (unsigned char *)text, strlen(text));
    free((void *)text);
    holder.end();
}

Editor::~Editor(){
    //free((void *)menu_bar.menu());
    //menu_bar.menu(nullptr);
}

// Basically dump what we know.
void Editor::info() const {
    char buffer[8];
    unsigned long long s = editor.buffer()->length();
    fl_alert("Editor information:\npath: %s\nFilesize: %s %cB\nAdler32 Checksum: %i\n", 
        path().c_str(), sizeNumberString(buffer, s), sizePrefixChar(s), adler);
}

bool Editor::load(){

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

    return true;
}

bool Editor::save(){

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

void Editor::infoCallback(Fl_Widget *w, void *a){
    Editor *ed = static_cast<Editor *>(a);
    ed->info();
}

void Editor::saveCallback(Fl_Widget *w, void *a){
    Editor *ed = static_cast<Editor *>(a);
    ed->save();
}

void Editor::saveAsCallback(Fl_Widget *w, void *a){
    Editor *ed = static_cast<Editor *>(a);
    const char *file_name = fl_input("Choose a file", ed->path().c_str());
    if(file_name){
        ed->path(file_name);
        ed->save();
    }
}

void Editor::loadCallback(Fl_Widget *w, void *a){
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


#define MENU_SIZE 9
#define MENU_DUMMY (void *)0xDEAD

static const Fl_Menu_Item menu_[MENU_SIZE] = {
    {"File", 0, 0, 0, FL_SUBMENU},
        {"Open", FL_COMMAND+'o', Editor::loadCallback, MENU_DUMMY},
        {"Save", FL_COMMAND+'s', Editor::saveCallback, MENU_DUMMY},
        {"Save As", FL_COMMAND+FL_SHIFT+'s', Editor::saveAsCallback, MENU_DUMMY},
    {0},
        {"Edit", 0, 0, 0, FL_SUBMENU},
        {"Properties", FL_COMMAND+'?', Editor::infoCallback, MENU_DUMMY},
    {0},
{0}
};

Fl_Menu_Item *Editor::menu(){
    Fl_Menu_Item *that = (Fl_Menu_Item *)malloc(sizeof(Fl_Menu_Item)*MENU_SIZE);
    memcpy(that, menu_, sizeof(menu_));
    return that;
}

const Fl_Menu_Item *Editor::prepareMenu(void(*OpenCallback_)(Fl_Widget *, void *a), void *arg_) const{
    Fl_Menu_Item *m = menu();
    for(int i = 0; i<MENU_SIZE; i++){
        if(m[i].user_data()==MENU_DUMMY)
            m[i].user_data((void *)this);
    }

    m[1].callback(OpenCallback_);
    m[1].user_data(arg_);
    return m;
}

} // namespace Flare
