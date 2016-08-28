#pragma once

#include "editor.hpp"
#include "find.hpp"

#include <FL/Fl_Window.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>

#include <vector>
#include <memory>

namespace Flare {

class EditorWindow {
public:
    static void OpenCallback(Fl_Widget *w, void *a);
    static void FindCallback(Fl_Widget *w, void *a){
        EditorWindow *const window = static_cast<EditorWindow *>(a);
        window->finder.hide();
        window->finder.show();
    }
private:
    
    class TabScroll : public Fl_Scroll {
    public:
        TabScroll(int ax, int ay, int aw, int ah, const char *acaption = nullptr)
          : Fl_Scroll(ax, ay, aw, ah, acaption){}
        int handle(int e) override;
        
        EditorWindow *window;
        
    };
    
    Find finder;
    
    std::vector<std::unique_ptr<Editor> > editors;
    
    Fl_Window window;
    Fl_Menu_Bar menu_bar;
    Fl_Button left_button, right_button;
    TabScroll scroll;
    Fl_Pack tab_bar;
    Fl_Group holder;
    Fl_Box resizer;
    
    bool scroll_again;
    unsigned movement_direction; // -1 is left, 1 is right
    static void timer_callback(void *a);
    
    void scrollFullyLeft();
    void scrollFullyRight();
    void scrollLeft(int ticks = 1);
    void scrollRight(int ticks = 1);
    
    unsigned which_;
    
    template<int D> friend
    void repeat_button_callback(Fl_Widget *w, void *a);

    Fl_Menu_Item *emptyMenu();
    
    void show(unsigned i){
        if(i>=children()) return;
        void *o = (void *)menu_bar.menu();
        menu_bar.menu(
            editors[i]->prepareMenu(OpenCallback, FindCallback, this)
        );
        free(o);
        tab_bar.child(i)->box(FL_GLEAM_DOWN_BOX);
        tab_bar.child(i)->labelcolor(1);
        tab_bar.child(i)->color(FL_BLUE);
        holder.child(i)->show(); 
        menu_bar.redraw();
    }

    void hide(unsigned i){
        if(i>=children()) return;
        tab_bar.child(i)->box(FL_UP_BOX);
        tab_bar.child(i)->labelcolor(FL_FOREGROUND_COLOR);
        tab_bar.child(i)->color(FL_BACKGROUND_COLOR);
        holder.child(i)->hide();
    }

    inline const Fl_Widget *first_button() const {
        if(tab_bar.children()==0) return nullptr;
        return tab_bar.child(0);
    }

    inline const Fl_Widget *last_button() const {
        if(tab_bar.children()==0) return nullptr;
        return tab_bar.child(tab_bar.children()-1);
    }

    inline Editor *getEditor(unsigned i) { return editors[i].get(); };
    bool close(unsigned i){
        editors.erase(editors.begin()+i);
        return true;
    }
    
    inline long EditorMaxScroll(){
    
        long last_x_plus_w = 0;
        
        for(int i = 0; i<scroll.children(); i++){
            const Fl_Widget * const c = scroll.child(i);
            const long l = c->w() + c->x();
            
            if(l > last_x_plus_w)
                last_x_plus_w = l;
        }
        return last_x_plus_w;
    }

    inline void TryEditorWindowScroll(int DX){
        
        if(scroll.children()==0) 
            return;
        const long to = scroll.xposition()+DX,
            n_w = window.w() - 32;
            
        if(
                ((to >= 0) && DX<0)
                ||
                ((n_w < EditorMaxScroll()) && DX>0)
        ){
            scroll.scroll_to(
                to, 
                scroll.yposition());
        }
    }
public:
    
    friend class TabButton;
    friend class TabScroll;
    
    EditorWindow();
    ~EditorWindow(){
        editors.clear();
    }

    Fl_Box *getResizer() { return &resizer; }

    void show() { window.show(); }
    void end() { window.end(); }
    template<class T>
    void add(T &that){ holder.add(that); }

    unsigned which(){
        if(which_>=holder.children())
            which_ = holder.children()-1;
        return which_;
    }

    void push(unsigned i, bool do_it_anyway = false){
        if(empty()) return;

        hide(which_);
        show(i);
        which_ = i;
    }
    
    inline void find(const char * text){ editors[which()]->find(text); }
    
    inline bool empty() const { return editors.empty(); }
    inline unsigned children() const { return editors.size(); }
    inline Fl_Widget *child(unsigned i) { return holder.child(i); }

    inline Fl_Widget *first(){
        if(empty()) return nullptr;
        return child(0);
    }

    inline Fl_Widget *last(){
        if(empty()) return nullptr;
        return child(children()-1);
    }

    void openFile(const std::string &path);
    static void ShowButtonCallback(Fl_Widget *w, void *a);

};

}