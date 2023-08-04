
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include "utility/utility.h"

using namespace std;

bool DecryptText(const vector<uint8_t>& in, vector<uint8_t>& out) {
  if (in.size() <= 5)
    return false;
  if (in[0] != 0xfe || in[1] != 0xfe)
    return false;
  if (in[2] != 1)
    return false;
  if (in[3] != 0xff || in[4] != 0xfe)
    return false;
  if ((in.size() - 5) % 2)
    return false;
  out.resize(in.size() - 5 + 2);
  out[0] = 0xff;
  out[1] = 0xfe; // Unicode BOM
  for (size_t i = 5, j = 0; i < in.size() - 1; i += 2, j += 2) {
    wchar_t c = *(wchar_t*)&in[i];
    *(wchar_t*)&out[j] = ((c & 0xaaaaaaaa) >> 1) | ((c & 0x55555555) << 1);
  }
  return true;
}

int main(int argc, char** argv) {
  if (argc != 2 && argc != 3) {
    cout << "usage: DecryptText.exe <input_file> [<output_file>])\n";
    return 1;
  }

  ifstream infile(argv[1], ios::binary);
  if (!infile.is_open()) {
    cout << "can't open input file: " << argv[1] << endl;
    return 2;
  }
  infile.seekg(0, ios::end);
  vector<uint8_t> indata(static_cast<size_t>(infile.tellg()));
  infile.seekg(0, ios::beg);
  infile.read((char*)indata.data(), indata.size());
  infile.close();

  vector<uint8_t> outdata;
  if (!DecryptText(indata, outdata)) {
    cout << "decryption failed\n";
    return 3;
  }

  string outfilename = (argc == 2 ? argv[1] : argv[2]);
  ofstream outfile(outfilename, ios::binary);
  if (!outfile.is_open()) {
    cout << "can't open output file: " << outfilename << endl;
    return 4;
  }
  outfile.write((char*)outdata.data(), outdata.size());
  outfile.close();
  cout << "decryption success.\n";
  return 0;
}