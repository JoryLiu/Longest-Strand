#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;
using namespace chrono;
namespace fs = std::filesystem;

typedef unsigned char BYTE;

vector<BYTE> readFile(const string filename) {
    // open the file:
    std::ifstream file(filename, std::ios::binary);

    // Stop eating new lines in binary mode!!!
    file.unsetf(std::ios::skipws);

    // get its size:
    std::streampos fileSize;

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // reserve capacity
    vector<BYTE> v;
    v.reserve(fileSize);

    // read the data:
    v.insert(v.begin(), std::istream_iterator<BYTE>(file),
             std::istream_iterator<BYTE>());

    return v;
}

vector<BYTE> LCS(vector<BYTE> X, vector<BYTE> Y)
{
    int m = X.size();
    int n = Y.size();
    // Create a table to store lengths of longest common
    // suffixes of substrings.   Note that LCSuff[i][j]
    // contains length of longest common suffix of X[0..i-1]
    // and Y[0..j-1]. The first row and first column entries
    // have no logical meaning, they are used only for
    // simplicity of program
    vector<vector<int>> LCSuff(m + 1, vector<int>(n + 1));
 
    // To store length of the longest common substring
    int len = 0;
 
    // To store the index of the cell which contains the
    // maximum value. This cell's index helps in building
    // up the longest common substring from right to left.
    int row, col;
 
    /* Following steps build LCSuff[m+1][n+1] in bottom
       up fashion. */
    for (int i = 0; i <= m; i++) {
        for (int j = 0; j <= n; j++) {
            if (i == 0 || j == 0)
                LCSuff[i][j] = 0;
 
            else if (X[i - 1] == Y[j - 1]) {
                LCSuff[i][j] = LCSuff[i - 1][j - 1] + 1;
                if (len < LCSuff[i][j]) {
                    len = LCSuff[i][j];
                    row = i;
                    col = j;
                }
            }
            else
                LCSuff[i][j] = 0;
        }
    }

    vector<BYTE> lcs(len);

    if (len) {
        // traverse up diagonally form the (row, col) cell
        // until LCSuff[row][col] != 0
        while (LCSuff[row][col] != 0) {
            lcs[--len] = X[row - 1]; // or Y[col-1]
    
            // move diagonally up to previous cell
            row--;
            col--;
        }
    }
 
    // required longest common substring
    return lcs;
}

// Fills lps[] for given pattern pat[0..m-1]
void computeLPSArray(const vector<BYTE>& pat, int m, vector<int>& lps) {
    // length of the previous longest prefix suffix
    int len = 0;

    lps[0] = 0;  // lps[0] is always 0

    // the loop calculates lps[i] for i = 1 to m-1
    int i = 1;
    while (i < m) {
        if (pat[i] == pat[len]) {
            len++;
            lps[i] = len;
            i++;
        } else  // (pat[i] != pat[len])
        {
            if (len != 0) {
                len = lps[len - 1];
            } else  // if (len == 0)
            {
                lps[i] = 0;
                i++;
            }
        }
    }
}

int KMP(const vector<BYTE>& pat, const vector<BYTE>& txt) {
    int res = -1;
    int m = pat.size();
    int n = txt.size();

    // create lps[] that will hold the longest prefix suffix
    // values for pattern
    vector<int> lps(m);

    // Preprocess the pattern (calculate lps[] array)
    computeLPSArray(pat, m, lps);

    int i = 0;  // index for txt[]
    int j = 0;  // index for pat[]

    while (i < n) {
        if (pat[j] == txt[i]) {
            j++;
            i++;
        }

        if (j == m) {
            return i - j;
            j = lps[j - 1];
        }

        // mismatch after j matches
        else if (i < n && pat[j] != txt[i]) {
            // Do not match lps[0..lps[j-1]] characters,
            // they will match anyway
            if (j != 0)
                j = lps[j - 1];
            else
                i = i + 1;
        }
    }

    return res;
}

int main() {
    // get all input files' paths
    string path = "Inputs";
    vector<string> input_file_paths;
    for (const auto& entry : fs::directory_iterator(path))
        input_file_paths.push_back(entry.path());

    // read all inputs
    vector<vector<BYTE>> inputs;
    for (string p : input_file_paths) {
        inputs.push_back(readFile(p));
        clog << "Reading from " + p << endl;
    }

    auto start = system_clock::now();

    // find the longest strand of bytes
    int size = input_file_paths.size();
    vector<BYTE> longest_strand;
    for (int i = 0; i < size - 1; i++)
        for (int j = i + 1; j < size; j++) {
            auto common_strand = LCS(inputs[i], inputs[j]);
            if (common_strand.size() > longest_strand.size())
                longest_strand = common_strand;
        }

    // clog << "The longest strand of bytes is:" << endl;
    // for (auto b : longest_strand) clog << std::hex << int(b);
    // clog << endl;

    cout << "The length of the longest strand of bytes is: " +
                to_string(longest_strand.size())
         << endl;

    auto end = system_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    clog << "Finding the longest strand of bytes took "
         << double(duration.count()) * microseconds::period::num /
                microseconds::period::den
         << " seconds" << endl;

    for (int i = 0; i < size; i++) {
        int offset = KMP(longest_strand, inputs[i]);
        if (offset != -1)
            cout << "The longest strand of bytes appears in " +
                        input_file_paths[i] + " and the offset is " +
                        to_string(offset)
                 << endl;
    }

    return 0;
}
