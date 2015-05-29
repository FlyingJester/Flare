#pragma once

#include <zlib.h>

#include <FL/Fl_Group.H>
//#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Text_Editor.H>

#include <string>

namespace Flare {

class Editor {

    Fl_Group holder;
//    Fl_Menu_Bar menu_bar;
    Fl_Text_Editor editor;

    uLong adler;

    std::string path_;

    static Fl_Menu_Item *menu();

public:
    const Fl_Menu_Item *prepareMenu(void(*OpenCallback_)(Fl_Widget *, void *a) = nullptr, void *arg_ = nullptr) const;

    Editor(int x, int y, int w, int h);
    Editor(const Editor &that);
    ~Editor();

    Fl_Group &getGroup(){ return holder; }

    void focus();
    void unfocus();

    void info() const;
    bool save();
    bool load();
    void path(const std::string &s) {path_ = s;}
    const std::string &path() const {return path_;}

    static void infoCallback(Fl_Widget *w, void *a);
    static void saveCallback(Fl_Widget *w, void *a);
    static void saveAsCallback(Fl_Widget *w, void *a);
    static void loadCallback(Fl_Widget *w, void *a);

    void setMenu();
    void calculateAdler32();

};

}
