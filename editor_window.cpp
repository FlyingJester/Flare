#include "editor_window.hpp"

#include <FL/Fl.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Native_File_Chooser.H>

#include <cstdio>
#include <cassert>

namespace Flare {

const double ScrollRate(){
    return 1.0/100.0;
}

/*
std::list<Editor> editors;
    
Fl_Window window;
Fl_Scroll tab_bar;
Fl_Repeat_Button left_button, right_button;
    
bool scroll_again;

unsigned movement_direction; // 1 is left, 2 is right
    
static void timer_callback(void *a);
*/

void EditorWindow::timer_callback(void *a){
    EditorWindow *window = static_cast<EditorWindow *>(a);

    if(!(window->left_button.value() || window->right_button.value())){
        window->scroll_again = false;
        return;
    }
    
    window->scroll.scroll_to(
        window->scroll.xposition()+window->movement_direction, 
        window->scroll.yposition());
    
    Fl::add_timeout(ScrollRate(), EditorWindow::timer_callback, a);
}

template<int D>
void repeat_button_callback(Fl_Widget *w, void *a){
    EditorWindow *window = static_cast<EditorWindow *>(a);
    window->movement_direction = D;

    if(!window->scroll_again){
        Fl::add_timeout(0.01, EditorWindow::timer_callback, a);
        window->scroll_again = true;
    }
}


void EditorWindow::openFile(const std::string &path){

    assert(editors.size()==tab_bar.children());
    assert(editors.size()==holder.children());

    fl_font(window.labelfont(), window.labelsize());


    std::string::const_iterator from = path.cend()--, to = path.cend();
    while(from!=path.cbegin()) if(*from=='/') {from++; break; } else from--;

    const std::string new_path = std::string(from, to);

    const Fl_Widget * const end_button = last_button();
    Fl_Button * const button = new Fl_Button(0, 0, fl_width(new_path.c_str(), new_path.size())+12, 24);

    button->copy_label(new_path.c_str());
    
    editors.push_back(std::unique_ptr<Editor>(new Editor(
        holder.x(),
        holder.y(),
        holder.w(),
        holder.h()
    )));

    holder.add(editors.back()->getGroup()),
    tab_bar.add(button);

    tab_bar.size(tab_bar.h()+button->w()+Fl::box_dw(button->box()), tab_bar.h());

    editors.back()->path(path);
    editors.back()->load();

    button->callback(ShowButtonCallback, this);

    assert(editors.size()==tab_bar.children());
    assert(editors.size()==holder.children());

    button->do_callback();

}

/*
void NonNativeOpenCallback(Fl_Widget *w, void *a){
    EditorWindow *window = static_cast<EditorWindow *>(a);

    static Fl_File_Chooser chooser("./", nullptr, Fl_File_Chooser::MULTI, "Open Files");

    chooser.show();

    while(chooser.shown()) Fl::wait();

    for(int i = 0; i<chooser.count(); i++)
        window->openFile(chooser.value(i));

    window->scroll.redraw();
    window->tab_bar.redraw();

    assert(window->editors.size()==window->tab_bar.children());
    assert(window->editors.size()==window->holder.children());

    window->push(window->children()-1, true);

}
*/

void EditorWindow::OpenCallback(Fl_Widget *w, void *a){
    EditorWindow *window = static_cast<EditorWindow *>(a);

    Fl_Native_File_Chooser chooser;

    chooser.title("Open Files");
    chooser.type(Fl_Native_File_Chooser::BROWSE_MULTI_FILE);
    chooser.options(Fl_Native_File_Chooser::NEW_FOLDER | 
        Fl_Native_File_Chooser::PREVIEW);
    
    int err = chooser.show();
    if(err==1) return;
    else if (err==-1){
        fl_alert("Error choosing a file\n%s", chooser.errmsg());
        return;
    }

    for(int i = 0; i<chooser.count(); i++)
        window->openFile(chooser.filename(i));

}

void EditorWindow::ShowButtonCallback(Fl_Widget *w, void *a){
    EditorWindow *window = static_cast<EditorWindow *>(a);
    unsigned i = window->tab_bar.find(w);

    assert(window->children()==window->tab_bar.children());
    assert(window->children()==window->holder.children());

    assert(i<window->tab_bar.children());

    window->push(i);
    
}

static const Fl_Menu_Item s_menu[4] = {
  {"File", 0, 0, 0, FL_SUBMENU},
    {"Open", FL_COMMAND+'o', EditorWindow::OpenCallback, 0},
  {0},
{0}
};

Fl_Menu_Item *EditorWindow::emptyMenu(){
    Fl_Menu_Item *l_menu = (Fl_Menu_Item *)malloc(sizeof(s_menu));
    memcpy(l_menu, s_menu, sizeof(s_menu));
    
    l_menu[1].user_data((void *)this);
    return l_menu;
}

#define WIDTH  600
#define HEIGHT 400
#define BUTTON_HEIGHT 32
#define BUTTON_WIDTH 32
#define MENU_HEIGHT 24

EditorWindow::EditorWindow()
  : window(WIDTH, HEIGHT, "Flare Text Editor")
  , menu_bar(0, 0, WIDTH, MENU_HEIGHT)
  , left_button(0, MENU_HEIGHT, BUTTON_HEIGHT, BUTTON_HEIGHT, "<")
  , right_button(WIDTH-BUTTON_WIDTH, MENU_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, ">")
  , scroll(BUTTON_HEIGHT, MENU_HEIGHT, WIDTH-(BUTTON_WIDTH<<1), BUTTON_HEIGHT)
  , tab_bar(BUTTON_HEIGHT, MENU_HEIGHT, 0, BUTTON_HEIGHT)
  , holder(0, BUTTON_HEIGHT+MENU_HEIGHT, WIDTH, HEIGHT-(BUTTON_HEIGHT+MENU_HEIGHT))
  , resizer(BUTTON_WIDTH<<1, (BUTTON_HEIGHT<<1)+MENU_HEIGHT, WIDTH-(BUTTON_WIDTH<<3), HEIGHT-(BUTTON_HEIGHT<<2)){

    window.add(holder);
    window.add(resizer);
    window.resizable(resizer);
//    resizer.box(FL_EMBOSSED_BOX);
    holder.end();

    scroll.type(0);
    scroll.end();

    tab_bar.resizable(nullptr);
    tab_bar.type(Fl_Pack::HORIZONTAL);

    left_button.callback(repeat_button_callback<-3>, this);
    left_button.when(FL_WHEN_CHANGED);
    right_button.callback(repeat_button_callback<3>, this);
    right_button.when(FL_WHEN_CHANGED);

    menu_bar.menu(emptyMenu());

//    menu_bar.

    window.end();
}

} // namespace Flare

int main(int argc, char *argv[]){
    Flare::EditorWindow window;
// editor(0, 0, 600, 400);
    
    window.show();
    
    
    return Fl::run();
}
