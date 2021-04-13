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

vector<BYTE> LCS(const vector<BYTE>& v1, const vector<BYTE>& v2) {
    if (v1.empty() || v2.empty()) return vector<BYTE>();

    // build L[m+1][n+1] in bottom up fashion
    int m = v1.size();
    int n = v2.size();
    vector<vector<int>> L(m + 1, vector<int>(n + 1));

    for (int i = 0; i <= m; i++) {
        for (int j = 0; j <= n; j++) {
            if (i == 0 || j == 0)
                L[i][j] = 0;
            else if (v1[i - 1] == v2[j - 1])
                L[i][j] = L[i - 1][j - 1] + 1;
            else
                L[i][j] = max(L[i - 1][j], L[i][j - 1]);
        }
    }

    // get LCS
    int index = L[m][n];

    // Create a character array to store the lcs string
    vector<BYTE> lcs(index);

    // Start from the right-most-bottom-most corner and
    // one by one store characters in lcs[]
    int i = m, j = n;
    while (i > 0 && j > 0) {
        // If current character in v1 and v2 are same, then
        // current character is part of LCS
        if (v1[i - 1] == v2[j - 1]) {
            lcs[index - 1] = v1[i - 1];  // Put current character in result
            i--;
            j--;
            index--;  // reduce values of i, j and index
        }

        // If not same, then find the larger of two and
        // go in the direction of larger value
        else if (L[i - 1][j] > L[i][j - 1])
            i--;
        else
            j--;
    }

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

    cout << "The longest strand of bytes is:" << endl;
    for (auto b : longest_strand) cout << std::hex << int(b);
    cout << endl;

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
                        to_string(i)
                 << endl;
    }

    return 0;
}