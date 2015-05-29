#pragma once
#include "editor.hpp"

#include <FL/Fl_Text_Editor.H>

namespace Flare {

class TextEditor : public Editor {

    Fl_Text_Editor editor;

    static Fl_Menu_Item *menu();

public:
    const Fl_Menu_Item *prepareMenu(void(*OpenCallback_)(Fl_Widget *, void *a) = nullptr, void *arg_ = nullptr) const override;

    TextEditor(int x, int y, int w, int h);
    virtual ~TextEditor();

    void info() const override;
    bool save() override;
    bool load() override;

    static void infoCallback(Fl_Widget *w, void *a);
    static void saveCallback(Fl_Widget *w, void *a);
    static void saveAsCallback(Fl_Widget *w, void *a);
    static void loadCallback(Fl_Widget *w, void *a);

    void calculateAdler32() override;

    static Editor *CreateTextEditor(int x, int y, int w, int h);

};

}
