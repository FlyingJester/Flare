#include "find.hpp"

#include "editor_window.hpp"

namespace Flare {
/*
    EditorWindow &window
    Fl_Input find_input, replace_input;
    Fl_Button find_button, replace_button;
*/

void Find::FindText() const{
    const char * const text = find_input.value();
    if(text[0]==0) return;
    window.find(text);
}

void Find::ReplaceText() const{
    const char * const text = find_input.value(),
        * const replace_text = replace_input.value();
    if(text[0]==0 || replace_text[0]==0) return;
    
}

Find::Find(EditorWindow &w)
  : Fl_Window(400, 100, "Find")
  , window(w)
  , find_input(8, 8, 240, 24)
  , replace_input(8, 40, 240, 24)
  , find_button(256, 8, 80, 24, "Find")
  , replace_button(256, 40, 80, 24, "Replace"){
  
  find_button.callback(FindCallback, this);
  replace_button.callback(ReplaceCallback, this);
    
}

}
