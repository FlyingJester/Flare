#include "flare_text_editor_widget.hpp"

#include <FL/fl_ask.h>
#include <FL/Fl.H>

namespace Flare {

    void Text_Editor_Widget::BufferCallback(int pos, int add, int del, int styled, const char* deleted_text){
            
        // I do not know if we can really trust this 
        // to be true, but we rely on it for now.
#ifndef NDEBUG
        if((del>0) && (add>0)) fl_alert("Whoops!\nWe both added and deleted? What?");
        if((pos<0)) fl_alert("Whoops!\nPosition is negative?");
        if((add<0)) fl_alert("Whoops!\nNumber of added chars is negative?");
        if((del<0)) fl_alert("Whoops!\nNumber of deleted chars is negative?");
#endif
        if(add==0 && del==0){
           // style_buffer->unselect();
            return;
        }
        
        if(canary>0u){ return; }
        canary++;
                    
        if((!future.empty()) || history.empty()){
            future.clear();
          
            struct diff that = {
                ((add>0)  ?
                    (buffer()->text_range(pos, pos+add)) :
                    (strdup(deleted_text))),
                pos,
                add,
                del
            };
            
            history.push_back(that);
        }
        else{
            struct diff & top = history.back();
            if(add>0 && top.add>0 && top.pos+top.add==pos){
                
                char *t = buffer()->text_range(pos, pos+add);
                top.text = (char *)realloc(top.text,top.add+add+1);
                memcpy(top.text+top.add, t, add+1);
                top.add+=add;
                free(t);
            }
            else if(del>0 && top.del>0 && top.pos==pos-del){
                top.text = (char *)realloc(top.text, top.del+del+1);
                memcpy(top.text+top.del, deleted_text, del+1);
                top.del+=del;
            }
            else{
                struct diff that = {
                    ((add>0)  ?
                        (buffer()->text_range(pos, pos+add)) :
                        (strdup(deleted_text))),
                    pos,
                    add,
                    del
                };       
                history.push_back(that);
            }
        }
        canary--;
    }

    void Text_Editor_Widget::undo(){
        if(canary>0u) return;
        if(history.empty()) return;
        canary++;
        
        struct diff op = history.pop();
        if(op.del>0){
            buffer()->insert(op.pos, op.text);
        }
        else{
            buffer()->remove(op.pos, op.pos+op.add);
        }

        future.push_back(op);
        
        canary--;
    }

    void Text_Editor_Widget::redo(){
        if(canary>0u) return;   
        if(future.empty()) return;     
        canary++;

        struct diff op = future.pop();
        if(op.add>0){
            buffer()->insert(op.pos, op.text);
        }
        else{
            buffer()->remove(op.pos, op.pos+op.del);
        }
        
        history.push_back(op);
        
        canary--;
    }

    int Text_Editor_Widget::handle(int e){
        if(e==FL_KEYDOWN){
            if(Fl::event_length()!=1) return Fl_Text_Editor::handle(e);
            const char first_char = Fl::event_text()[0];
            if(first_char=='\t'){
                const Fl_Text_Selection * const selection = mBuffer->primary_selection();
                if(!selection->selected()){
                    insert(tab.c_str());
                }
                else{
                    int line_start_pos = line_start(selection->start());
                    
                    while(line_start_pos<selection->end()){
                        if(Fl::event_shift()!=0){
                            // TODO: Add ability to remove multiple tabs.
                        }
                        else
                            mBuffer->insert(line_start_pos, tab.c_str());
                        line_start_pos = line_end(line_start_pos, true);
                        
                        while((mBuffer->char_at(line_start_pos)=='\n') || (mBuffer->char_at(line_start_pos+1)=='\n')){
                            line_start_pos++;
                        }
                        
                    }
                }
                return 1;
            }
            else if(first_char=='\n' || first_char=='\r'){
                std::string whitespace;
                char ucs_buffer[5];

                if((insert_position()>=1) && (line_start(insert_position())!=insert_position())){
                                
                    int start = line_start(insert_position()-1); 
                    
                    // Advance until we hit a newline, we hit the current insert position, or we are no longer on a whitespace character.
                    // NOTE! `fl_nonspacing' is backwards--the same as `isspace'. So why is it NON-spacing? I have no idea.
                    while((mBuffer->byte_at(start)!='\n') && (mBuffer->byte_at(start)!='\r') &&
                        (start < insert_position()) && (fl_nonspacing(mBuffer->char_at(start)))){

                        if(mBuffer->char_at(start)<0x80){
                            whitespace += mBuffer->byte_at(start);
                            start++;
                        }
                        else{ // TODO: Test this block.
                            int len = fl_utf8encode(mBuffer->char_at(start), ucs_buffer);
                            ucs_buffer[len] = '\0';
                            whitespace += ucs_buffer;
                            start+=len;
                        }
                    }
                }
                insert("\n");
                
                if(!whitespace.empty()) insert(whitespace.c_str());
                
                if(mBuffer->selected()){
                    mBuffer->remove_selection();
                }
                
                return 1;       
            }
        }
        return Fl_Text_Editor::handle(e);
    }

}
