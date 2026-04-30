#include <cstdint>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

int pad_and_concatenate(ofstream *output_file, uint32_t target_address,
                        ifstream *input_file1, uint32_t file1_size,
                        ifstream *input_file2, uint32_t file2_size,
                        uint32_t max_size = 0) {

  // Check if files exceed final desired size

  cout << "File1 is " << file1_size << " (" << std::hex << file1_size
       << std::dec << ") bytes long." << endl;
  cout << "File2 is " << file2_size << " (" << std::hex << file2_size
       << std::dec << ") bytes long." << endl;

  if (file1_size > target_address) {
    cerr << "Error: File1 exceed target address " << std::hex << file1_size
         << " > " << target_address << endl;
    return -EINVAL;
  }

  uint32_t padding_size = target_address - file1_size;

  uint32_t file_size = file1_size + padding_size + file2_size;

  if (max_size && (file_size > max_size)) {
    cerr << "Error: Files + padding size (" << file_size << " (" << std::hex
         << file_size << ") bytes) exceeds maximum size, aborting." << endl;
    return -EINVAL;
  }

  cout << "Final file size is " << file_size << " (" << std::hex << file_size
       << std::dec << ") bytes." << endl;

  try {
    std::vector<uint8_t> outfile_buf(file_size, 0x0);

    input_file1->read((char *)outfile_buf.data(), file1_size);

    if (input_file1->bad()) {
      throw std::runtime_error("could not read file1");
    }

    input_file2->read((char *)outfile_buf.data() + target_address, file2_size);

    if (input_file2->bad()) {
      throw std::runtime_error("could not read file2");
    }

    output_file->write((char *)outfile_buf.data(), outfile_buf.size());

    if (output_file->bad()) {
      throw std::runtime_error("could not write to output file");
    }

  } catch (const std::exception &e) {
    cerr << e.what() << endl;
    return -EBADF;
  }

  return 0;
}

void print_usage() {
  cerr << "Concatenate and pad binary files\n"
       << "\t-v'\tVerbose mode\n"
       << "\t-o'\tOutput file name\n"
       << "\t-a \tTarget Address\n"
       << "\t-s \t Desired maximum address for result binary\n"
       << "\t--file1 \tFirst input file to concatenate\n"
       << "\t--file2 \t Second input file to concatenate" << endl;
}

int main(int argc, char *argv[]) {
  int ret;
  if (argc < 11 || argc > 13) {
    cerr << "Invalid Arguments!" << endl << "Expected Arguments: ";
    print_usage();
    return -EINVAL;
  }

  string outfile_name;
  string infile1_name;
  string infile2_name;
  uint32_t target_address;
  uint32_t max_size = 0;
  uint32_t infile1_size, infile2_size;
  bool verbose = false;

  for (int i = 1; i < argc; i++) {
    string cmd = argv[i];

    if (cmd == "-o") {
      outfile_name = argv[++i];
    } else if (cmd == "--file1") {
      infile1_name = argv[++i];
    } else if (cmd == "--file2") {
      infile2_name = argv[++i];
    } else if (cmd == "-a") {
      target_address = stoul(argv[++i], nullptr, 0);
    } else if (cmd == "-s") {
      max_size = stoul(argv[++i], nullptr, 0);
    } else if (cmd == "-v") {
      verbose = true;
    }
  }

  if (verbose == false) {
    cout.setstate(std::ios_base::badbit);
  }

  ofstream outfile;
  cout << "Outfile: " << outfile_name << endl;
  outfile.open(outfile_name, std::ofstream::binary | std::ofstream::out);

  ifstream infile1;
  cout << "Infile1: " << infile1_name << endl;
  infile1.open(infile1_name, std::ifstream::binary | std::ifstream::in);

  ifstream infile2;
  cout << "Infile2: " << infile2_name << endl;
  infile2.open(infile2_name, std::ifstream::binary | std::ifstream::in);

  cout << "Concat. address: " << std::hex << target_address << endl;
  cout << "Max. address: " << max_size << std::dec << endl;

  try {
    infile1_size = std::filesystem::file_size(infile1_name);
    infile2_size = std::filesystem::file_size(infile2_name);
  } catch (const exception &e) {
    cerr << e.what();
    return -EBADF;
  }

  ret = pad_and_concatenate(&outfile, target_address, &infile1, infile1_size,
                            &infile2, infile2_size, max_size);

  if (ret) {
    cerr << "Could not concatenate files." << endl;
  }

  outfile.close();
  infile1.close();
  infile2.close();

  return ret;
}
