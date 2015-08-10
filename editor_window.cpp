#include "editor_window.hpp"

#include <FL/Fl.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Native_File_Chooser.H>

#include <cstdio>
#include <cassert>

namespace Flare {

int EditorWindow::TabScroll::handle(int e){
    
    if(e==FL_MOUSEWHEEL){
    
        if(Fl::event_dx()>0)
            window->scrollFullyRight();
        else if(Fl::event_dx()<0)
            window->scrollFullyLeft();
        else if(Fl::event_dy()>0)
            window->scrollRight(Fl::event_dy()<<3);
        else if(Fl::event_dy()<0)
            window->scrollLeft(-(Fl::event_dy()<<3));

        return 1;
    }
    
    return Fl_Scroll::handle(e);
}

class TabButton : public Fl_Button {
    static void ReloadCallback(Fl_Widget *w, void *a){
        Fl_Button *button = static_cast<Fl_Button *>(a);
        EditorWindow *window = static_cast<EditorWindow *>(button->user_data());
        unsigned i = window->tab_bar.find(button);
        switch(fl_choice("Are you sure you want to reload the document?\nYou will lose any unsave changes.", fl_yes, fl_no, nullptr)){
            case 1: return;
            case 0: window->getEditor(i)->load();
        }
    }
    static void InfoCallback(Fl_Widget *w, void *a){
        Fl_Button *button = static_cast<Fl_Button *>(a);
        EditorWindow *window = static_cast<EditorWindow *>(button->user_data());
        unsigned i = window->tab_bar.find(button);
        window->getEditor(i)->info();
    }
    static void CloseCallback(Fl_Widget *w, void *a){
        Fl_Button *button = static_cast<Fl_Button *>(a);
        EditorWindow *window = static_cast<EditorWindow *>(button->user_data());
        unsigned i = window->tab_bar.find(button);
        switch(fl_choice("Save Changes?", fl_cancel, fl_yes, fl_no)){
            case 0: return;
            case 1: window->getEditor(i)->save();
        }
        window->close(i);
        Fl::delete_widget(button);
    }
public:
    int handle(int e) override {
        if((e==FL_PUSH) && (Fl::event_button()==FL_RIGHT_MOUSE)){
            Fl_Menu_Item rclick_menu[] = {
                { "Properties", 0, InfoCallback, this, FL_MENU_DIVIDER},
                { "Reload", 0, ReloadCallback, this},
                { "Close",  0, CloseCallback, this},
                { 0 }
            };
            const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, 0);
            if(m) m->do_callback(0, m->user_data());
            return 1;
        }
        else if((e==FL_RELEASE) && (Fl::event_button()==FL_RIGHT_MOUSE)){
            return 1;
        }
        else return Fl_Button::handle(e);
    }

    TabButton(int X, int Y, int W, int H, const char *L = nullptr) : Fl_Button(X, Y, W, H, L){}
};

const double ScrollRate(){
    return 1.0/100.0;
}

void EditorWindow::timer_callback(void *a){
    EditorWindow *window = static_cast<EditorWindow *>(a);

    if(!(window->left_button.value() || window->right_button.value())){
        window->scroll_again = false;
        return;
    }
    
    window->TryEditorWindowScroll(window->movement_direction);
    
    Fl::add_timeout(ScrollRate(), EditorWindow::timer_callback, a);
}

void EditorWindow::scrollFullyLeft(){
    TryEditorWindowScroll(0);
}

void EditorWindow::scrollFullyRight(){
    TryEditorWindowScroll(EditorMaxScroll());
}

void EditorWindow::scrollLeft(int ticks){

    if(ticks>=scroll.xposition())
        scrollFullyLeft();
    else for(int i = 0; i<ticks; i++)
            TryEditorWindowScroll(-1);
}

void EditorWindow::scrollRight(int ticks){
    if(ticks + scroll.xposition()>=EditorMaxScroll())
        scrollFullyRight();
    else for(int i = 0; i<ticks; i++)
            TryEditorWindowScroll(1);
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

void EditorWindow::ShowButtonCallback(Fl_Widget *w, void *a){
    EditorWindow *window = static_cast<EditorWindow *>(a);
    unsigned i = window->tab_bar.find(w);

    assert(window->children()==window->tab_bar.children());
    assert(window->children()==window->holder.children());

    assert(i<window->tab_bar.children());

    window->push(i);
    
}

void EditorWindow::openFile(const std::string &path){

    assert(editors.size()==tab_bar.children());
    assert(editors.size()==holder.children());

    fl_font(window.labelfont(), window.labelsize());


    std::string::const_iterator from = path.cend()--, to = path.cend();
    while(from!=path.cbegin()) if(*from=='/') {from++; break; } else from--;

    const std::string new_path = std::string(from, to);

    from = path.cend()--;
    while(from!=path.cbegin()) if(*from=='.') {from++; break; } else from--;

    const std::string extension = std::string(from, to);

    const Fl_Widget * const end_button = last_button();
    Fl_Button * const button = new TabButton(0, 0, fl_width(new_path.c_str(), new_path.size())+12, 24);

    button->copy_label(new_path.c_str());
    
    {
        Editor *e = Editor::GetEditorForExtension(extension)(holder.x(), holder.y(), holder.w(), holder.h());
        editors.emplace_back(e);
    }

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
  , finder(*this)
  , menu_bar(0, 0, WIDTH, MENU_HEIGHT)
  , left_button(0, MENU_HEIGHT, BUTTON_HEIGHT, BUTTON_HEIGHT, "<")
  , right_button(WIDTH-BUTTON_WIDTH, MENU_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, ">")
  , scroll(BUTTON_HEIGHT, MENU_HEIGHT, WIDTH-(BUTTON_WIDTH<<1), BUTTON_HEIGHT)
  , tab_bar(BUTTON_HEIGHT, MENU_HEIGHT, 0, BUTTON_HEIGHT)
  , holder(0, BUTTON_HEIGHT+MENU_HEIGHT, WIDTH, HEIGHT-(BUTTON_HEIGHT+MENU_HEIGHT))
  , resizer(BUTTON_WIDTH<<1, (BUTTON_HEIGHT<<1)+MENU_HEIGHT, WIDTH-(BUTTON_WIDTH<<3), HEIGHT-(BUTTON_HEIGHT<<2)){
    
    scroll.window = this;
    
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

    Flare::Editor::RestoreDefaultEditor();

    Flare::EditorWindow window;
// editor(0, 0, 600, 400);
    
    window.show();
    
    
    return Fl::run();
}
