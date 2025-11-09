#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <ncurses.h>
#include <algorithm>
#include <fstream>
#include <iomanip>

namespace fs = std::filesystem;

struct FileItem {
    std::string name;
    bool isDir;
    uintmax_t size;
    std::string perms;
};

std::string perms_to_string(fs::perms p) {
    std::string s = "---------";
    s[0]=(p&fs::perms::owner_read )!=fs::perms::none?'r':'-';
    s[1]=(p&fs::perms::owner_write)!=fs::perms::none?'w':'-';
    s[2]=(p&fs::perms::owner_exec )!=fs::perms::none?'x':'-';
    s[3]=(p&fs::perms::group_read )!=fs::perms::none?'r':'-';
    s[4]=(p&fs::perms::group_write)!=fs::perms::none?'w':'-';
    s[5]=(p&fs::perms::group_exec )!=fs::perms::none?'x':'-';
    s[6]=(p&fs::perms::others_read)!=fs::perms::none?'r':'-';
    s[7]=(p&fs::perms::others_write)!=fs::perms::none?'w':'-';
    s[8]=(p&fs::perms::others_exec )!=fs::perms::none?'x':'-';
    return s;
}

fs::perms numeric_to_perms(int n){
    fs::perms p=fs::perms::none;
    int o=(n/100)%10,g=(n/10)%10,ot=n%10;
    auto add=[&](int t,fs::perms r,fs::perms w,fs::perms x){
        if(t&4)p|=r; if(t&2)p|=w; if(t&1)p|=x;
    };
    add(o,fs::perms::owner_read,fs::perms::owner_write,fs::perms::owner_exec);
    add(g,fs::perms::group_read,fs::perms::group_write,fs::perms::group_exec);
    add(ot,fs::perms::others_read,fs::perms::others_write,fs::perms::others_exec);
    return p;
}

class FileExplorer{
    fs::path currentPath;
    std::vector<FileItem> items;
    int cursor=0;
    fs::path clipboardPath;
    bool isCopy=false,isCut=false;

public:
    FileExplorer(){ currentPath=fs::current_path(); loadDir(); }

    void loadDir(){
        items.clear();
        for(auto &e:fs::directory_iterator(currentPath)){
            FileItem i;
            i.name=e.path().filename().string();
            i.isDir=e.is_directory();
            std::error_code ec;
            i.size=e.is_regular_file(ec)?e.file_size(ec):0;
            i.perms=perms_to_string(e.status().permissions());
            items.push_back(i);
        }
        std::sort(items.begin(),items.end(),[](auto&a,auto&b){
            if(a.isDir!=b.isDir) return a.isDir>b.isDir;
            return a.name<b.name;
        });
        cursor=0;
    }

    void draw(const std::string &msg=""){
        clear();
        int rows,cols; getmaxyx(stdscr,rows,cols);
        attron(A_BOLD|COLOR_PAIR(2));
        mvprintw(0,0,"+------------------------------------------------------------------------------+");
        mvprintw(1,0,"| FILE EXPLORER | Path: %-60s |",currentPath.string().substr(0,60).c_str());
        mvprintw(2,0,"+------------------------------------------------------------------------------+");
        attroff(A_BOLD|COLOR_PAIR(2));

        int start=4;
        for(int i=0;i<(int)items.size() && i<rows-10;i++){
            if(i==cursor) attron(A_REVERSE|COLOR_PAIR(3));
            if(items[i].isDir) attron(COLOR_PAIR(5)); else attron(COLOR_PAIR(1));
            mvprintw(start+i,2,"%-45s %10s %12lu bytes",
                     (items[i].isDir?(items[i].name+"/").c_str():items[i].name.c_str()),
                     items[i].perms.c_str(),items[i].size);
            if(items[i].isDir) attroff(COLOR_PAIR(5)); else attroff(COLOR_PAIR(1));
            if(i==cursor) attroff(A_REVERSE|COLOR_PAIR(3));
        }

        attron(COLOR_PAIR(6));
        mvprintw(rows-9,0,"Status: %-74s",msg.c_str());
        attroff(COLOR_PAIR(6));

        attron(A_BOLD|COLOR_PAIR(4));
        mvprintw(rows-7,0,"+------------------------------------------------------------------------------+");
        mvprintw(rows-6,0,"| Move: UP/DOWN | Open: ENTER | Back: LEFT | Search: /                        |");
        mvprintw(rows-5,0,"| New File: n | Mkdir: m | Rename: r | Delete: d | Permissions: p             |");
        mvprintw(rows-4,0,"| Copy: c | Cut: x | Paste: v | Quit: q                                       |");
        mvprintw(rows-3,0,"+------------------------------------------------------------------------------+");
        attroff(A_BOLD|COLOR_PAIR(4));
        refresh();
    }

    void copyCut(bool cpy){ if(items.empty())return; clipboardPath=currentPath/items[cursor].name; isCopy=cpy; isCut=!cpy; }

    void paste(){
        if(clipboardPath.empty())return;
        fs::path dest=currentPath/clipboardPath.filename();
        std::error_code ec;
        if(fs::exists(dest,ec)){ draw("File exists."); return;}
        if(isCopy) fs::copy(clipboardPath,dest,fs::copy_options::recursive|fs::copy_options::overwrite_existing,ec);
        else if(isCut) fs::rename(clipboardPath,dest,ec);
        if(ec) draw("Error."); else { draw(isCopy?"Copied.":"Moved."); if(isCut) clipboardPath.clear(); }
        loadDir();
    }

    void search(){
        echo(); char q[256];
        mvprintw(LINES-9,0,"Enter search term: "); getstr(q); noecho();
        clear(); int line=2;
        mvprintw(0,0,"Search results for '%s':",q);
        for(auto &p:fs::recursive_directory_iterator(currentPath)){
            if(p.path().filename().string().find(q)!=std::string::npos){
                mvprintw(line++,2,"%s",p.path().string().c_str());
                if(line>=LINES-2){ mvprintw(line,2,"--More--"); getch(); clear(); line=2; }
            }
        }
        mvprintw(line+1,2,"Press any key to return...");
        getch(); loadDir();
    }

    void changePerms(){
        if(items.empty()) return;
        clear();
        mvprintw(0,0,"Common permission codes:");
        mvprintw(1,2,"644 = rw-r--r--   (Owner read/write, others read)");
        mvprintw(2,2,"600 = rw-------   (Owner only)");
        mvprintw(3,2,"755 = rwxr-xr-x   (Owner full, others read/exec)");
        mvprintw(4,2,"700 = rwx------   (Owner full only)");
        mvprintw(5,2,"777 = rwxrwxrwx   (Everyone full)");
        mvprintw(7,0,"Enter numeric permission (e.g., 755): ");
        echo(); char buf[16]; getstr(buf); noecho();
        int val=atoi(buf);
        fs::permissions(currentPath/items[cursor].name,numeric_to_perms(val),fs::perm_options::replace);
        draw("Permissions changed.");
        loadDir();
    }

    void run(){
        initscr(); noecho(); keypad(stdscr,TRUE); curs_set(0); start_color();
        init_pair(1,COLOR_WHITE,COLOR_BLACK);
        init_pair(2,COLOR_YELLOW,COLOR_BLACK);
        init_pair(3,COLOR_BLACK,COLOR_CYAN);
        init_pair(4,COLOR_GREEN,COLOR_BLACK);
        init_pair(5,COLOR_BLUE,COLOR_BLACK);
        init_pair(6,COLOR_MAGENTA,COLOR_BLACK);

        draw("Ready.");
        int ch;
        while((ch=getch())!='q'){
            switch(ch){
                case KEY_UP: if(cursor>0)cursor--; break;
                case KEY_DOWN: if(cursor<(int)items.size()-1)cursor++; break;
                case 10: if(!items.empty()&&items[cursor].isDir){ currentPath/=items[cursor].name; loadDir(); } break;
                case KEY_LEFT: if(currentPath.has_parent_path()){ currentPath=currentPath.parent_path(); loadDir(); } break;
                case 'n': {echo(); char f[256]; mvprintw(LINES-9,0,"New file name: "); getstr(f); noecho(); std::ofstream(currentPath/f); loadDir(); draw("File created."); break;}
                case 'm': {echo(); char d[256]; mvprintw(LINES-9,0,"New directory name: "); getstr(d); noecho(); fs::create_directory(currentPath/d); loadDir(); draw("Dir created."); break;}
                case 'r': {echo(); char n[256]; mvprintw(LINES-9,0,"Rename to: "); getstr(n); noecho(); fs::rename(currentPath/items[cursor].name,currentPath/n); loadDir(); draw("Renamed."); break;}
                case 'd': if(!items.empty()){ fs::remove_all(currentPath/items[cursor].name); loadDir(); draw("Deleted."); } break;
                case 'c': copyCut(true); draw("Copied."); break;
                case 'x': copyCut(false); draw("Cut."); break;
                case 'v': paste(); break;
                case '/': search(); break;
                case 'p': changePerms(); break;
                default: break;
            }
            draw();
        }
        endwin();
    }
};

int main(){ FileExplorer e; e.run(); return 0; }

