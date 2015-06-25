#pragma once

#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Input.H>

namespace Flare {

class EditorWindow;

class Find : public Fl_Window {
    EditorWindow &window;
    Fl_Input find_input, replace_input;
    Fl_Return_Button find_button;
    Fl_Button replace_button;
    
    static void FindCallback(Fl_Widget *w, void *a){
        static_cast<Find *>(a)->FindText();
    }
    
    static void ReplaceCallback(Fl_Widget *w, void *a){
        static_cast<Find *>(a)->ReplaceText();
    }
    
    void FindText() const;
    void ReplaceText() const;
    
public:
    Find(EditorWindow &w);
    virtual ~Find(){}
};


}
