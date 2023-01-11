//===========================================FILE EXPLORER IN LINUX===================================================
//   BY : DIKSHA DARYANI
//   2021201045
//===========================================FILE EXPLORER IN LINUX===================================================

#include <bits/stdc++.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#define esc 27
#define cls printf("%c[2J", esc)
using namespace std;

//=================================================SOME GLOBAL DECLARATIONS==============================================
int co = 1;
int mode_bit; //0 for normal mode 1 for cmd mode
vector<struct dirent> curdirents;
stack<string> nextdir;
stack<string> prevdir; //new
string cur_dir_path;
struct termios orig_termios;
void runloop(int s, int e, string y);
int listdir(string x);
void noncanomode(string cur_dir_path);
void die(const char *s);
void disableRawMode();
void commandmode();
void entriess(string x, string y);
string findpath(string token);
struct winsize termSize;
int curs = 0;
string w;
int tempcur = 1;
string dot = ".";
string dotdot = "..";
string path;

//=================================================FUNCTION DEFINITIONS START==============================================
void colorgreen(string message)
{
    cout << "\033[0;32m";
    cout << message;
    cout << "\033[0m";
}
void colorred(string message)
{
    cout << "\033[0;31m";
    cout << message;
    cout << "\033[0m";
}
void normalstatus()
{
    colorred("nnnnnnnnnnnnnnnnnnnnnn NORMAL MODE  nnnnnnnnnnnnnnnnnnnnnn");
}
void cmdstatus()
{
    colorred("nnnnnnnnnnnnnnnnnnnnnn CMD MODE  nnnnnnnnnnnnnnnnnnnnnn");
}
void high()
{
    colorgreen("Enter the command :: ");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void moveCursor(int x, int y)
{
    cout << "\033[" << x << ";" << y << "H";
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void die(const char *s)
{
    perror(s);
    exit(1);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int getch(void)
{

    int ch;
    struct termios oldt;
    struct termios newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= static_cast<unsigned int>(~(ICANON | ECHO));
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void process(string input, vector<string> &token)
{

    string word = "";
    for (auto x : input)
    {
        if (x == ' ')
        {
            token.push_back(word);
            word = "";
        }
        else
        {
            word = word + x;
        }
    }
    token.push_back(word);
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
bool checkdir(string path)
{

    struct stat st;
    if (stat(path.c_str(), &st) == -1)
    {
        printf("wrong name\n");
        return false;
    }
    else
    {
        if ((S_ISDIR(st.st_mode)))
            return true;
        else
            return false;
    }
    return false;
}

//+++++++++++++++++++++++++++++++++++++++++++COPY FILE +++++++++++++++++++++++++++++++++++++++++++++++++++++++

void copyFile(string srce, string destn)
{
    struct stat sourceInfo, destnInfo;
    int r;
    char writeBlock[1024];

    int inret = open(srce.c_str(), O_RDONLY);
    int opret = open(destn.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (stat(srce.c_str(), &sourceInfo) == -1)
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        // printf("true");

        printf("error Press a key to type next command");
    }
    else if (stat(destn.c_str(), &destnInfo) == -1)
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        // printf("true");

        printf("errorPress a key to type next command");
    }
    while (r = read(inret, writeBlock, sizeof(writeBlock)) > 0)
    {
        write(opret, writeBlock, r);
    }
    chown(destn.c_str(), sourceInfo.st_uid, sourceInfo.st_gid);
    chmod(destn.c_str(), sourceInfo.st_mode);
}

//+++++++++++++++++++++++++++++++++++++++++++COPY DIRECTORY +++++++++++++++++++++++++++++++++++++++++++++++++++++++
void copyDir(string srce, string destn)
{
    DIR *d;
    struct dirent *dir;
    int ret = mkdir(destn.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    d = opendir(srce.c_str());
    if (d == NULL)
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        // printf("true");

        printf("error Press a key to type next command");

        return;
    }
    while ((dir = readdir(d)) != NULL)
    {
        if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
            ;
        else
        {
            // string name = dir->d_name;
            string currentfilePath = srce + "/" + dir->d_name;
            // printf("current file path is %s\n", currentfilePath.c_str());
            string destPathForEachFile = destn + "/" + dir->d_name;

            struct stat st;
            if (stat(currentfilePath.c_str(), &st) == -1)
            {
                ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
                int xpos = termSize.ws_row - 2;
                moveCursor(xpos, 0);
                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));

                printf("error Press a key to type next command");

                return;
            }

            if (S_ISDIR(st.st_mode))
            {

                copyDir(currentfilePath, destPathForEachFile);
            }
            else
            {
                copyFile(currentfilePath, destPathForEachFile);
            }
        }
    }
    //close dir
    // closedir(d);
}
//+++++++++++++++++++++++++++++++++++++++++++COPY  +++++++++++++++++++++++++++++++++++++++++++++++++++++++
void copy(vector<string> &token)
{
    int wordnum = token.size();
    if (wordnum < 3)
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        // printf("true");

        printf("error Press a key to type next command");

        return;
    }

    for (int i = 1; i < wordnum - 1; i++)
    {
        string dirpath = token[wordnum - 1];
        string destn = findpath(dirpath) + "/" + token[i];

        string srce = findpath(token[i]);

        if (checkdir(srce) == false)
        {

            copyFile(srce, destn);
        }
        else if (checkdir(srce) == true)
        {

            copyDir(srce, destn);
        }
    }
}

//+++++++++++++++++++++++++++++++++++++++++++CREATE  FILE +++++++++++++++++++++++++++++++++++++++++++++++++++++++
void create_file(vector<string> &token)
{
    int wordnum = token.size();
    if (wordnum < 3)
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        // printf("true");

        printf("error Press a key to type next command");

        return;
    }

    for (int i = 1; i < wordnum - 1; i++)
    {
        string fileName = token[i];
        string destinationPath = findpath(token[wordnum - 1]);
        destinationPath = destinationPath + "/" + fileName;
        int ret = creat(destinationPath.c_str(), O_RDONLY | O_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (ret == -1)
        {
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
            int xpos = termSize.ws_row - 2;
            moveCursor(xpos, 0);
            printf("\033[2K");
            printf("\033[% d;% dH", (xpos), (0));

            printf("Unable to create file Press a key to type next command");
        }
        else
        {
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
            int xpos = termSize.ws_row - 2;
            moveCursor(xpos, 0);
            printf("\033[2K");
            printf("\033[% d;% dH", (xpos), (0));

            printf("success  a key to type next command");
        }
    }
}
//+++++++++++++++++++++++++++++++++++++++++++FIND   PATH+++++++++++++++++++++++++++++++++++++++++++++++++++++++
string findpath(string token)
{
    if (token.length() == 1)
    {
        if (token[0] == '.')
            return dot;

        else if (token[0] == '~')
            return cur_dir_path;
    }
    else
    {

        if (token[0] == '.' and token[1] != '.')
            return dot + "/" + token.substr(2, token.length() - 2);

        else if (token[0] == '/')
            return token;

        else if (token[0] == '~')
            return cur_dir_path + "/" + token.substr(2, token.length() - 2);

        else if (token[0] == '.' and token[1] == '.' and token[2] != '\0')
            return dotdot + "/" + token.substr(2, token.length() - 2);

        else if (token[0] == '.' and token[1] == '.' and token[2] == '\0')
            return dotdot;
        else
        {
            string x;

            x = prevdir.top();
            return x + "/" + token;
        }
    }
}
//+++++++++++++++++++++++++++++++++++++++++++RENAME  FILE +++++++++++++++++++++++++++++++++++++++++++++++++++++++
void rename_file(vector<string> &token)
{
    if(token.size()!=3){

        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        // printf("true");

        printf("Unable to rename file Press a key to type next command");
        return;

    }
    string oldname = token[1];
    string newname = token[2];

    oldname = findpath(oldname);
    newname = findpath(newname);

    int ret = rename(oldname.c_str(), newname.c_str());
    if (ret == -1)
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        // printf("true");

        printf("Unable to rename file Press a key to type next command");
    }
    else
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        // printf("true");

        printf("success press a key to type next command");
    }
}

//+++++++++++++++++++++++++++++++++++++++++++CREATE   DIR +++++++++++++++++++++++++++++++++++++++++++++++++++++++
void create_dir(vector<string> &token)
{
    int wordnum = token.size();
    if (wordnum < 3)
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        // printf("true");

        printf("error Press a key to type next command");

        return;
    }
    for (int i = 1; i < wordnum - 1; i++)
    {
        string destfolder = findpath(token[wordnum - 1]) + "/" + token[i];
        int ret = mkdir(destfolder.c_str(), S_IRWXU | S_IRWXG);
        if (ret == -1)
        {
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
            int xpos = termSize.ws_row - 2;
            moveCursor(xpos, 0);
            printf("\033[2K");
            printf("\033[% d;% dH", (xpos), (0));
            printf("error press a key to type next command");
        }
        else
        {
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
            int xpos = termSize.ws_row - 2;
            moveCursor(xpos, 0);
            printf("\033[2K");
            printf("\033[% d;% dH", (xpos), (0));
            printf("success press a key to type next command");
        }
    }
}

//+++++++++++++++++++++++++++++++++++++++++++SEARCH FUN +++++++++++++++++++++++++++++++++++++++++++++++++++++++

int searchfun(string path, string name)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(path.c_str());
    if (d == NULL)
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        // printf("true");

        printf("Wrong path name. Press a key to type next command");

        return 0;
    }

    while ((dir = readdir(d)) != NULL)
    {
        if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
            ;
        else
        {
            struct stat st;
            string finalPath = path + "/" + dir->d_name;
            if (stat(finalPath.c_str(), &st) == -1)
            {
                ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
                int xpos = termSize.ws_row - 2;
                moveCursor(xpos, 0);
                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));
                // printf("true");

                printf("Error .Press a key to type next command");

                return 0;
            }
            if (S_ISDIR(st.st_mode))
            {
                if (!strcmp(dir->d_name, name.c_str()))
                {

                    return 1;
                }
                else
                {
                    if (searchfun(finalPath, name) == 1)
                        return 1;
                }
            }
            else
            {
                if (!strcmp(dir->d_name, name.c_str()))
                {

                    return 1;
                }
            }
        }
    }
    return 0;
}

int search(vector<string> &token)
{
    if (token.size() < 2)
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        // printf("true");

        printf("Invalid no.of arguements. Press a key to type next command");

        return 0;
    }

    path = prevdir.top();

    return searchfun(path, token[1]);
}

//+++++++++++++++++++++++++++++++++++++++++++GOTO  +++++++++++++++++++++++++++++++++++++++++++++++++++++++

string goto_(vector<string> &token)
{
    if (token.size() < 2)
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        // printf("true");

        printf("less arguments in Goto. Press a key to type next command");
    }
    return token[1];
}
//+++++++++++++++++++++++++++++++++++++++++++DELETE FILE +++++++++++++++++++++++++++++++++++++++++++++++++++++++
void removefilefun(string filename)
{
    int ret = remove(filename.c_str());
    if (ret == -1)
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        // printf("true");

        printf("error Press a key to type next command");
    }
}

void remove_file(vector<string> &token)
{
    string filename = findpath(token[1]);
    removefilefun(filename);
}

//+++++++++++++++++++++++++++++++++++++++++++REMOVE DIR+++++++++++++++++++++++++++++++++++++++++++++++++++++++

void removedirfun(string dirpath)
{

    DIR *d = opendir(dirpath.c_str());
    if (d)
    {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL)
        {

            if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
                ;
            else
            {
                string path = dirpath + "/" + dir->d_name;

                struct stat st;

                if (stat(path.c_str(), &st) == -1)
                {
                    ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
                    int xpos = termSize.ws_row - 2;
                    moveCursor(xpos, 0);
                    printf("\033[2K");
                    printf("\033[% d;% dH", (xpos), (0));
                    cout << "error press a key to type next command";
                    return;
                }
                if (S_ISDIR(st.st_mode))
                {

                    removedirfun(path);
                }
                else
                {

                    removefilefun(path);
                }
            }
        }
        closedir(d);
        int ret = rmdir(dirpath.c_str());
        if (ret == -1)
        {
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
            int xpos = termSize.ws_row - 2;
            moveCursor(xpos, 0);
            printf("\033[2K");
            printf("\033[% d;% dH", (xpos), (0));
            cout << "error press a key to type next command";
        }
        else
        {
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
            int xpos = termSize.ws_row - 2;
            moveCursor(xpos, 0);
            printf("\033[2K");
            printf("\033[% d;% dH", (xpos), (0));
            printf("success press a key to type next command");
        }
    }
    else
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        cout << "error,cant open dir press a key to type next command";
    }
}
//+++++++++++++++++++++++++++++++++++++++++++MOVE  +++++++++++++++++++++++++++++++++++++++++++++++++++++++

void moveFile(vector<string> &token)
{
    if (token.size()< 3)
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        // printf("true");

        printf("error Press a key to type next command");

        return;
    }
    for (int i = 1; i < token.size() - 1; i++)
    {
        string destn = findpath(token[token.size() - 1]) + "/" + token[i];

        string srce = findpath(token[i]);

        if (checkdir(srce) == true) //cehck parameters
        {

            copyDir(srce, destn);
            removedirfun(srce);
        }
        else
        {

            copyFile(srce, destn);
            removefilefun(srce);
        }
    }
}
//+++++++++++++++++++++++++++++++++++++++++++DELETE DIR +++++++++++++++++++++++++++++++++++++++++++++++++++++++
void remove_dir(vector<string> &token)
{
    if (token.size() != 2)
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int xpos = termSize.ws_row - 2;
        moveCursor(xpos, 0);
        printf("\033[2K");
        printf("\033[% d;% dH", (xpos), (0));
        // printf("true");

        printf("error Press a key to type next command");

        return;
    }
    string dirpath = findpath(token[1]);
    removedirfun(dirpath);
}

//+++++++++++++++++++++++++++++++++++++++++++CMD MODE +++++++++++++++++++++++++++++++++++++++++++++++++++++++

void commandmode()
{
    mode_bit = 1;
    // cls;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
    int last = termSize.ws_row;
    moveCursor(last, 0);
    cmdstatus();
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
    int xpos = termSize.ws_row - 2;
    moveCursor(xpos, 0);
    high();
    while (1)
    {

        int ch;
        int ypos = 21;
        string input;

        while (1)
        {

            ch = getch();
            if (ch == 13 || ch == 27 || ch == 'q' || ch == 'Q')
                break;
            if (ch == 127)
            {
                if (input.length() > 0)
                {

                    input = input.substr(0, input.length() - 1);

                    ypos--;
                    printf("\b \b");
                }
            }
            else
            {
                input += ch;
                ypos++;
                ;
                moveCursor(xpos, ypos);
                printf("%c", ch);
            }
        }
        // cout<<endl<<input;
        if (ch == 13)
        {
            // xpos++;
            // ypos=0;
            // moveCursor(xpos,ypos);

            vector<string> words;
            process(input, words);

            if (words[0] == "create_file")
            {

                create_file(words);
                int c = getch();
                moveCursor(xpos, 0);
                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));

                noncanomode(prevdir.top());

                ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
                int xpos = termSize.ws_row - 2;

                moveCursor(xpos, 0);

                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));
                high();

                // printf("\033[2;0H");
            }

            else if (words[0] == "delete_dir")
            {
                remove_dir(words);
                int c = getch();
                moveCursor(xpos, 0);
                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));

                noncanomode(prevdir.top());

                ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
                int xpos = termSize.ws_row - 2;

                moveCursor(xpos, 0);

                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));
                high();
            }
            else if (words[0] == "create_dir")
            {
                create_dir(words);
                int c = getch();
                moveCursor(xpos, 0);
                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));

                noncanomode(prevdir.top());

                ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
                int xpos = termSize.ws_row - 2;

                moveCursor(xpos, 0);

                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));
                high();
            }
            else if (words[0] == "rename")
            {
                rename_file(words);
                int c = getch();
                moveCursor(xpos, 0);
                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));

                noncanomode(prevdir.top());

                ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
                int xpos = termSize.ws_row - 2;

                moveCursor(xpos, 0);

                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));
                high();
            }
            else if (words[0] == "delete_file")
            {
                remove_file(words);
                int c = getch();
                moveCursor(xpos, 0);
                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));

                noncanomode(prevdir.top());

                ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
                int xpos = termSize.ws_row - 2;

                moveCursor(xpos, 0);

                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));
                high();
            }
            else if (words[0] == "copy")
            {
                copy(words);
                int c = getch();
                moveCursor(xpos, 0);
                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));

                noncanomode(prevdir.top());

                ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
                int xpos = termSize.ws_row - 2;

                moveCursor(xpos, 0);

                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));
                high();
            }
            else if (words[0] == "move")
            {
                moveFile(words);
                int c = getch();
                moveCursor(xpos, 0);
                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));

                noncanomode(prevdir.top());

                ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
                int xpos = termSize.ws_row - 2;

                moveCursor(xpos, 0);

                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));
                high();
            }
            else if (words[0] == "search")
            {
                if (search(words) == 1)
                {
                    moveCursor(xpos, 0);
                    printf("\033[2K");
                    printf("\033[% d;% dH", (xpos), (0));
                    printf("true press a key to give next command");
                }
                else
                {
                    moveCursor(xpos, 0);
                    printf("\033[2K");
                    printf("\033[% d;% dH", (xpos), (0));
                    printf("false press a key to give next command");
                }
                int c = getch();
                moveCursor(xpos, 0);
                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));

                noncanomode(prevdir.top());

                ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
                int xpos = termSize.ws_row - 2;

                moveCursor(xpos, 0);

                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));
                high();
            }
            else if (words[0] == "goto")
            {
                string location = goto_(words);
                if (location == "..")
                {

                    string cwd = prevdir.top();
                    if (cwd == "/home")
                        continue;
                    while (cwd[cwd.length() - 1] != '/')
                    {
                        cwd.pop_back();
                    }
                    cwd.pop_back();
                    prevdir.push(cwd);
                    noncanomode(cwd);
                }
                else if (location == "~")
                {
                    char cwd[256];
                    if (getcwd(cwd, sizeof(cwd)) == NULL)
                        perror("getcwd() error");
                    else
                        w = cwd;
                    prevdir.push(w);
                    noncanomode(w);
                }
                else if (location == ".")
                {
                    location = findpath(location);
                    prevdir.push(location);
                    noncanomode(prevdir.top());
                }
                ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
                int xpos = termSize.ws_row - 2;

                moveCursor(xpos, 0);

                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));
                high();
            }

            else
            {
                ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
                int xpos = termSize.ws_row - 2;
                moveCursor(xpos, 0);
                printf("\033[2K");
                printf("\033[% d;% dH", (xpos), (0));
                // printf("true");

                printf("Wrong command, Try again. Press a key to type next command");

            }
        }
        else if (ch == 27)
        {
            //back to normal mode
            char cwd[256];
            if (getcwd(cwd, sizeof(cwd)) == NULL)
                perror("getcwd() error");
            else
                w = cwd;
            prevdir.push(w);
            mode_bit = 0;
            noncanomode(w);

            return;
        }
        else if (ch == 'q' || ch == 'Q')
        {
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
            int xpos = termSize.ws_row - 2;

            moveCursor(xpos, 0);

            printf("\033[2K");
            printf("\033[% d;% dH", (xpos), (0));
            cls;
            exit(0);
        }
    }
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

char *printsize(uint64_t bt)
{

    int i = 0;
    double dd = bt;
    char sx[][3] = {"B", "KB", "MB", "GB", "TB"};
    char length = sizeof(sx) / sizeof(sx[0]);
    if (bt > 1024)
    {
        for (i = 0; (bt / 1024) > 0 && i < length - 1; i++, bt /= 1024)
            dd = bt / 1024.0;
    }

    static char op[200];
    sprintf(op, "%.1lf%s", dd, sx[i]);
    return op;
}
//+++++++++++++++++++++++++++++++++++++++++++LIST DIR FUNCTION+++++++++++++++++++++++++++++++++++++++++++++++++++++++
int listdir(string curdirpath)
{
    cls;
    if (mode_bit == 0)
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int last = termSize.ws_row;
        moveCursor(last, 0);
        normalstatus();
    }
    else
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int last = termSize.ws_row;
        moveCursor(last, 0);
        cmdstatus();
    }
    moveCursor(1, 0);
    curdirents.clear();
    DIR *dirStream = opendir(curdirpath.c_str());
    if (!dirStream)
    {
        return -1;
    }

    else
    {
        //  cout<<endl<<"hello";
        struct dirent *d;
        d = readdir(dirStream);
        while (d != NULL)
        {
            curdirents.push_back(*d);
            d = readdir(dirStream);
        }
    }

    return 1;
}

// +++++++++++++++++++++++++++++++++++++++++++PRINT ENTRIES +++++++++++++++++++++++++++++++++++++++++++++++++++++++

void entries(string curdirpath, string dirEntryName)
{

    struct stat thestat;

    string absPathToDirItem = curdirpath + "/" + dirEntryName;

    stat(absPathToDirItem.c_str(), &thestat);

    // cout<<"heloo";
    //filename
    printf("%-15.10s", dirEntryName.c_str());

    //file size
    printf(" %-10s", printsize(thestat.st_size));

    //user owner
    string uname;
    if (getpwuid(thestat.st_uid) != NULL)
        uname = getpwuid(thestat.st_uid)->pw_name;
    printf(" %-10s", uname.c_str());

    //group
    string gname;
    if (getgrgid(thestat.st_gid) != NULL)
        gname = getgrgid(thestat.st_gid)->gr_name;
    printf(" %-5s\t", gname.c_str());

    //file type : S_IFMT flag
    switch (thestat.st_mode & S_IFMT)
    {
    case S_IFBLK:
        printf("b");
        break;
    case S_IFCHR:
        printf("c");
        break;
    case S_IFDIR:
        printf("d");
        break;
    default:
        printf("-");
        break;
    }
    //permissions

    if ((thestat.st_mode & S_IRUSR))
        printf("r");
    else
        printf("-");
    if ((thestat.st_mode & S_IWUSR))
        printf("w");
    else
        printf("-");
    if ((thestat.st_mode & S_IXUSR))
        printf("x");
    else
        printf("-");
    if ((thestat.st_mode & S_IRGRP))
        printf("r");
    else
        printf("-");
    if ((thestat.st_mode & S_IWGRP))
        printf("w");
    else
        printf("-");
    if ((thestat.st_mode & S_IXGRP))
        printf("x");
    else
        printf("-");
    if ((thestat.st_mode & S_IROTH))
        printf("r");
    else
        printf("-");
    if ((thestat.st_mode & S_IWOTH))
        printf("w");
    else
        printf("-");
    if ((thestat.st_mode & S_IXOTH))
        printf("x");
    else
        printf("-");

    //file modification time
    char *modTime = ctime(&thestat.st_mtime);
    modTime[strlen(modTime) - 1] = '\0';
    // string m=modTime;
    std::string m(modTime);
    string a = m.substr(4, 6);
    string b = m.substr(19, 5);
    cout << "\t" << (a + b);
    // m=m.substr(0,7);
    // printf(" \t%-10s", modTime);
}

// +++++++++++++++++++++++++++++++++++++++++++  RUNLOOP   +++++++++++++++++++++++++++++++++++++++++++++++++++++++

void runloop(int start, int end, string cur_dir_path)
{
    cls;
    if (mode_bit == 0)
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int last = termSize.ws_row;
        moveCursor(last, 0);
        normalstatus();
    }
    else
    {
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
        int last = termSize.ws_row;
        moveCursor(last, 0);
        cmdstatus();
    }
    moveCursor(1, 0);
    for (int i = start; i <= end; i++)
    {
        // cout<<"hii";
        entries(cur_dir_path, curdirents[i].d_name);
        printf("\n");
    }
}

//+++++++++++++++++++++++++++++++++++++++++++SHOW RAW MODE+++++++++++++++++++++++++++++++++++++++++++++++++++++++

void realrawmode()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    //   raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    //   raw.c_cc[VMIN] = 0;
    //   raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}
//+++++++++++++++++++++++++++++++++++++++++++RAW MODE +++++++++++++++++++++++++++++++++++++++++++++++++++++++

void raw_mode(int dirstart, int dirend, int st, int en)
{
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
    int height = termSize.ws_row - 4;

    realrawmode();
    moveCursor(1, 0);

    int key;
    bool tell = true;
    while (tell)
    {
        key = getch();
        // cout<<"j";

        if (key == 65) //up arrow

        {
            if (st >= 0 and en <= min(height - 1, dirend - 1))
            {
                if (tempcur != 1)
                {
                    tempcur--;
                    curs--;
                    moveCursor(tempcur, 0);
                }
            }
            else
            {
                if (tempcur != 1)
                {
                    tempcur--;
                    curs = st + tempcur - 1;
                    moveCursor(tempcur, 0);
                }
            }
        }
        if (key == 66) //scrolling
        {

            if (st >= 0 and en <= min(height - 1, dirend - 1))
            {
                if (tempcur != min(height, dirend))
                {
                    tempcur++;
                    curs++;
                    moveCursor(tempcur, 0);
                }
            }
            else
            {
                if (tempcur != min(height, dirend))
                {
                    tempcur++;
                    curs = st + tempcur - 1;
                    moveCursor(tempcur, 0);
                }
            }
        }

        if (key == 'l' || key == 'L') //scrolling
        {

            if (en != dirend - 1)
            {
                en++;
                st++;
                // curs++;
                // tempcur++;

                runloop(st, en, cur_dir_path);
                if (en > min(height - 1, dirend - 1))
                {
                    tempcur = 1;
                }
                curs = st + tempcur - 1;
                moveCursor(tempcur, 0);
            }
        }

        if (key == 'k' || key == 'K') //scrolling
        {

            if (st != 0)
            {
                en--;
                st--;

                runloop(st, en, cur_dir_path);
                if (st > 1)
                {
                    tempcur = 1;
                }
                curs = st + tempcur - 1;
                moveCursor(tempcur, 0);
            }
        }

        if (key == 67) // right arrow key
        {

            if (nextdir.size() >= 1)
            {
                prevdir.push(nextdir.top());
                // cur_dir_path = prevdir.top();
                nextdir.pop();
                noncanomode(prevdir.top());
            }
        }

        if (key == 68) //left arrow key
        {

            if (prevdir.size() > 1)
            {
                nextdir.push(prevdir.top());
                // cur_dir_path = prevdir.top();
                prevdir.pop();
                noncanomode(prevdir.top());
            }
        }

        if (key == 'q')
        {

            disableRawMode();
            cls;
            break;
        }
        if (key == ':')
        {

            commandmode();
            //enter command mode
        }

        if (key == 13)
        {

            struct dirent *myfile;
            myfile = &curdirents[curs];

            string cwf = prevdir.top();
            cwf += "/";
            string filename = myfile->d_name;
            cwf += filename;
            const char *path = cwf.c_str();
            // cout<<cwf<<"\n";

            struct stat s;
            if (stat(path, &s) == 0)
            {
                if (s.st_mode & S_IFDIR)
                {
                    prevdir.push(cwf);
                    tell = 0;
                    tempcur = 1;
                    curs = 0;
                    noncanomode(cwf);
                }
                else if (s.st_mode & S_IFREG)
                {
                    pid_t pid = fork();
                    if (pid == 0)
                    {
                        // child process
                        execlp("/usr/bin/xdg-open", "xdg-open", cwf.c_str(), (char *)0);
                        _exit(127);
                    };
                }
                else
                {
                    cout << "Anything else";
                }
            }
            else
            {
                cout << "Error";
            }
        }
        if (key == 127)
        {

            string cwd = prevdir.top();
            if (cwd == "/home")
                continue;
            while (!nextdir.empty())
                nextdir.pop();
            while (!prevdir.empty())
                prevdir.pop();
            while (cwd[cwd.length() - 1] != '/')
            {
                cwd.pop_back();
            }
            cwd.pop_back();
            prevdir.push(cwd);
            noncanomode(cwd);
        }

        if (key == 'H' || key == 'h') //homekey
        {
            char cwd[256];
            if (getcwd(cwd, sizeof(cwd)) == NULL)
                perror("getcwd() error");
            else
                w = cwd;
            prevdir.push(w);
            noncanomode(w);
        }
    }
}
//+++++++++++++++++++++++++++++++++++++++++++NON CANO MODE +++++++++++++++++++++++++++++++++++++++++++++++++++++++
void noncanomode(string cur_dir_path)
{
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
    int height = termSize.ws_row - 4;

    listdir(cur_dir_path);

    int start = 0;
    int end = 0;

    if (curdirents.size() >= height)
    {
        end = height - 1;
    }
    else
    {
        end = curdirents.size() - 1;
    }

    runloop(start, end, cur_dir_path);
    if (mode_bit == 1)
    {
        return;
    }
    else
    {
        raw_mode(0, curdirents.size(), start, end);
    }
}
//+++++++++++++++++++++++++++++++++++++++++++RESIZE HANDLING +++++++++++++++++++++++++++++++++++++++++++++++++++++++
void handle_winch(int sig)
{
    signal(SIGWINCH, SIG_IGN);

    // printf("%c[%d;%df", 0x1B, 0, 0); // cursor at 0,0;
    cout << string(22, '\n');        // Clearing the screen;
    printf("%c[%d;%df", 0x1B, 0, 0); // cursor at 0,0;

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
    int height = termSize.ws_row - 4;

    int start = 0;
    int end = 0;

    if (curdirents.size() >= height)
    {
        end = height - 1;
    }
    else
    {
        end = curdirents.size() - 1;
    }

    runloop(start, end, cur_dir_path);

    signal(SIGWINCH, handle_winch);
}
//+++++++++++++++++++++++++++++++++++++++++++MAIN FUNCTION+++++++++++++++++++++++++++++++++++++++++++++++++++++++
int main(int argc, char *argv[])
{
    signal(SIGWINCH, handle_winch);

    char cwd[256];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
        perror("getcwd() error");
    else
        cur_dir_path = cwd;
    // cout<<cur_dir_path;
    prevdir.push(cur_dir_path);

    mode_bit = 0;

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
    int last = termSize.ws_row;
    moveCursor(last, 0);
    normalstatus();
    // printf("\033[2K");
    printf("\033[% d;% dH", (0), (0));
    noncanomode(cur_dir_path);
}

//==================================================  END OF THE ASSIGNMENT =============================================

//==================================================  END OF THE ASSIGNMENT =============================================
