#define SYMBOL_TYPE wchar_t

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cwchar>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>

using namespace std;

class CDir
{
    private:
    string <SYMBOL_TYPE> _name;
    vector <CDir> _branchDir;

    public:
    CDir();
    ~CDir();
    void open(string path);
    void close();
    void create(string path);
    CDir* copy(string <SYMBOL_TYPE>);

    protected:

}

int main(int argc, char** argv, char** envp)
{
    CDir mainDir;

    //if-block
    if (argc < 3)
    {
        cout << "Incorrect input of function my_cp"
        return 0;
    }
    if (!strcmp(argv[1], argv[2]))
    {
        return 0;
    }
    //

    mainDir.open(argv[1]);
    mainDir.copy(argv[2]);

    return 0;
}

void CDir.open(string path)
{
    DIR *dir;
    dir = opendir(path);
    if (!dir)
    {
        perror("No such file or directory");
        exit(1);
    }
    while ((dirent curr = readdir(dir)) != NULL)
    {
    _name = path;
    _branchDir.push_back();
    }
}