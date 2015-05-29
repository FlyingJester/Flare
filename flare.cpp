#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Buffer.H> 
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Menu_Item.H>
#include <FL/Fl.H>

#include <FL/Enumerations.H>

#include <FL/fl_ask.H>

#include <string>
#include <cstdio>
#include <cassert>

/* An example of a text editor program that crashes with FLTK 1.3.3
    It only seems to crash in OS X (I've only tried 10.10), not Linux.
    I also did not have this issue with 1.3.2 (although I have not
    tried this test with 1.3.2).
        To make it crash within minutes, all I have to do is open a 
    file (I've used a copy of this file, flare.cpp, to test this), 
    start making small changes, and at some point it will crash.
        This seems to happen most often when a focus even happens,
    when I click a menu entry, or when I copy or paste. I actually
    wondered for a long time if it had to do mainly with c&p, but
    it will (very occasionally) crash when loading a file, as well.
        The crashes appear as heap corruption, or as a warning that
    I've damaged an OS X canary that is put on freed objects (object
    modified after free). I do not know which one of the two (if
    either) is true.

    This text editor is featureful enough that I can actually use it
    for non-contrived purposes, but it is still small enough that it
    seems a good example. The usefulness is important to me, anyway,
    since that makes it much easier to actually do enough to cause a
    crash.

    I accept that this could still be my mistake. But I've made many
    attempts to write a text editor with FLTK, each from scratch, and
    all of them have these same issues rather extremely.

    Perhaps it is that I am mishandling the text in the text buffer.
    In that case, I would certainly love to hear what I am doing wrong.
    I have attempted to follow the documentation on how to use them
    as close as I can!
*/

// Shorthand.
inline Fl_Text_Buffer *CreateTextBuffer(){ return new Fl_Text_Buffer(0x100, 0x100); }

// A couple attributes for our files.
struct EditorData {
    std::string path;
};

// Basically dump what we know.
void Info(Fl_Text_Editor *ed){
    assert(ed);
    fl_alert("Editor information:\npath: %s\n",
        static_cast<struct EditorData *>(ed->user_data())->path.c_str());
}

bool Load(Fl_Text_Editor *ed, const char *path){
    assert(ed);
    assert(path);
    FILE *that = fopen(path, "r");
    if(!that){
        fl_alert("Cannot open file %s", path);
        return false;
    }
    // Buffer the file in.
    char buffer[0x100];
    unsigned to = 0;

    // Clear the buffer
    ed->buffer()->text(nullptr);
    // Load the file.
    do{
        to = fread(buffer, 1, 0xFF, that);
        buffer[to] = 0;
        ed->buffer()->append(buffer);
    }while(to==0xFF);

    fclose(that);

    static_cast<struct EditorData *>(ed->user_data())->path  = path;

    return true;
}

void Save(Fl_Text_Editor *ed, const char *path){
    assert(ed);
    assert(path);

    char buffer[0x100];

    unsigned to = 0;

    // Create a backup of the original file, just in case :)
    const std::string new_file_name = std::string(path)+".baku";
    rename(path, new_file_name.c_str());

    FILE *that = fopen(path, "w");
    if(that){

        const char *text = ed->buffer()->text();
        const unsigned length = strlen(text);
        // Try to write the file.
        if(fwrite(text, 1, length, that)!=length){
            fl_alert("Could not save file %s.", path);
            return;
        }

        fclose(that);
        // If we succeeded, we don't need the backup any more.
        remove(new_file_name.c_str());

    }
    else // Alert if we couldn't open the file for saving.
        fl_alert("Could save file %s.", path);
}

void InfoCallback(Fl_Widget *w, void *a){
    Fl_Text_Editor *ed = static_cast<Fl_Text_Editor *>(a);
    Info(ed);
}

void SaveCallback(Fl_Widget *w, void *a){
    Fl_Text_Editor *ed = static_cast<Fl_Text_Editor *>(a);
    const std::string path = static_cast<struct EditorData *>(ed->user_data())->path;
    Save(ed, path.c_str());
}

void SaveAsCallback(Fl_Widget *w, void *a){
    Fl_Text_Editor *ed = static_cast<Fl_Text_Editor *>(a);
    std::string &path = static_cast<struct EditorData *>(ed->user_data())->path;

    const char *file_name = fl_input("Choose a file", nullptr);
    if(file_name){
        path = file_name;
        Save(ed, path.c_str());
    }
}

void LoadCallback(Fl_Widget *w, void *a){
    Fl_Text_Editor *ed = static_cast<Fl_Text_Editor *>(a);
    std::string &path = static_cast<struct EditorData *>(ed->user_data())->path;
    
    if(!path.empty()){
        switch(fl_choice("Save changes to file %s?", fl_no, fl_yes, fl_cancel, path.c_str())){
            case 0:
            break;
            case 1:
            Save(ed, path.c_str());
            break;
            case 2:
            return;
        }
    }
    const char *file_name = fl_input("Choose a file", nullptr);
    if(file_name){
        path = file_name;
        Load(ed, path.c_str());
    }
}

int main(int argc, char *argv[]){
    Fl_Window this_window(800, 400, "Flare Editor");
    Fl_Menu_Bar main_bar(0, 0, 800, 24);
    Fl_Text_Editor editor(0, 24, 800, 400-24);

    editor.user_data(new struct EditorData);
    editor.textfont(FL_SCREEN);
    editor.buffer(CreateTextBuffer());
#if FLTK_ABI_VERSION == 10303
    editor.linenumber_width(64);
#endif
    editor.end();

    this_window.resizable(editor);
    
    Fl_Menu_Item menu[] = {
        {"File", 0, 0, 0, FL_SUBMENU},
            {"Open", FL_COMMAND+'o', LoadCallback, &editor},
            {"Save", FL_COMMAND+'s', SaveCallback, &editor},
            {"Save As", FL_COMMAND+FL_SHIFT+'s', SaveAsCallback, &editor},
        {0},
            {"Edit", 0, 0, 0, FL_SUBMENU},
            {"Properties", FL_COMMAND+'?', InfoCallback, &editor},
        {0},
    {0}
    };
    
    main_bar.menu(menu);

    this_window.show();
    this_window.end();

    if(argc>1){
        Load(&editor, argv[1]);
    }
    else{
        const char *file_name = fl_input("Choose a file", nullptr);
        if(file_name && file_name[0]!=0){
            Load(&editor, file_name);
        }
    }

    this_window.redraw();

    return Fl::run();
}

