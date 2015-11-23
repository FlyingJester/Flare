#pragma once

#include <FL/Fl_Text_Editor.H>
#include <cstring>
#include "history_tracker.hpp"

namespace Flare {

class Text_Editor_Widget : public Fl_Text_Editor {

    struct diff {
        char *text;
        int pos, add, del;
    };

    static void delete_diff(struct diff d){
        free((void *)d.text);
    }

    static size_t size_diff(size_t a, struct diff d){
        return a+strlen(d.text)+sizeof(struct diff);
    }

    unsigned canary;

    std::string tab;
    Pluto::HistoryTracker<struct diff, delete_diff, size_diff, 0x3FFFF> history, future;

    void BufferCallback(int, int, int, int, const char*);

    static int undo_key_binding(int k, Fl_Text_Editor *editor){ static_cast<Text_Editor_Widget *>(editor)->undo(); return 1; }
    static int redo_key_binding(int k, Fl_Text_Editor *editor){ static_cast<Text_Editor_Widget *>(editor)->redo(); return 1; }

    void removeText(long at, const char *text, unsigned long len = 0);

public:

    Text_Editor_Widget(int X, int Y, int W, int H, const char *L = nullptr)
      : Fl_Text_Editor(X, Y, W, H, L)
      , tab(4, ' '){
#if FLTK_ABI_VERSION >= 10303
        if(W>128)
            Fl_Text_Display::linenumber_width(40);
#endif
        canary = 0u;

        remove_key_binding('z', FL_COMMAND);
        remove_key_binding('y', FL_COMMAND);
        add_key_binding('z', FL_COMMAND, undo_key_binding);
        add_key_binding('y', FL_COMMAND, redo_key_binding);

    }
 
    void clearHistory(){
        history.clear();
    }
 
   static void text_buffer_change_cb(int a, int b, int c, int d, const char* e, void*that){
        static_cast<Text_Editor_Widget *>(that)->BufferCallback(a, b, c, d, e);
    }

    int handle(int e) override;

    void tabString(const std::string &str){ tab = str; }
    void tabString(const char *str){ tab = str; }
    void tabString(const char c){ char str[2] = {c, 0}; tabString(str); }

    const std::string &tabString() const { return tab; }

    void undo();
    void redo();
    
    void duplicate();

};

}
