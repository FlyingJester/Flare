#pragma once

#include <zlib.h>

#include <FL/Fl_Group.H>
#include <FL/Fl_Text_Editor.H>

#include <string>

namespace Flare {

class Editor {
protected:

    Fl_Group holder;

    uLong adler;

    std::string path_;

public:

    typedef Editor *(*EditorFactory)(int, int, int, int);

    virtual const Fl_Menu_Item *prepareMenu(void(*OpenCallback_)(Fl_Widget *, void *a) = nullptr, void *arg_ = nullptr) const = 0;

    Editor(int x, int y, int w, int h);
    virtual ~Editor();

    virtual Fl_Group &getGroup(){ return holder; }

    virtual void info() const = 0;
    virtual bool save() = 0;
    virtual bool load() = 0;
    virtual void path(const std::string &s) {path_ = s;}
    virtual const std::string &path() const {return path_;}

    virtual void calculateAdler32() = 0;

    static bool RegisterFiletype(const std::string &extension, EditorFactory factory);
    static bool RegisterDefaultEditor(EditorFactory factory);
    static EditorFactory GetDefaultEditor();
    static bool RestoreDefaultEditor();
    static EditorFactory GetEditorForExtension(const std::string &extension);

};

}
