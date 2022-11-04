#include <iostream>
using namespace std;

int main()
{
    string input_line;
    long line_count = 0;

    while (cin) {
        getline(cin, input_line);
        if (!cin.eof()) line_count++;
    }

    cout << line_count << endl;

    return 0;
}