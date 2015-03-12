// system
#include <libgen.h>
#include <iostream>
#include <string>
#include <utility>
#include <sstream>

// lhapdf
#include "LHAPDF/LHAPDF.h"

// applgrid
#include "appl_grid/appl_grid.h"

// yoda
#include <YODA/WriterYODA.h>

using namespace std;

// declare the LHAPDF functions we will pass to APPLgrid
extern "C" void evolvepdf_(const double& , const double& , double* );
extern "C" double alphaspdf_(const double& Q);

typedef pair<size_t, size_t> range;

range range_from_string(string const & range_string) {
  range ret(0, 1);
  size_t separator_pos = range_string.find("...");
  if (separator_pos != string::npos) {
    ret.first = atoi(range_string.substr(0, separator_pos).c_str());
    ret.second = atoi(range_string.substr(separator_pos + 3).c_str()) - ret.first + 1;
  } else {
    ret.first = atoi(range_string.c_str());
  }
  return ret;
}

void print_usage(string const & program_name) {
  cout << "usage: " << program_name << " <applgrid_file_name> <rivet_path> [[pdf_set_name] subset_start[...subset_end]]" << endl;
}

static const std::string::size_type bannerWidth = 54;
static const char bannerFrameChar = '*';

void print_banner_content_line(const string line)
{
  const string::size_type totalMarginSize = bannerWidth - line.size() - 2;
  const string::size_type oneSideMarginSize = totalMarginSize / 2;
  const string margin(oneSideMarginSize, ' ');
  cout << bannerFrameChar << margin << line << margin;
  if (totalMarginSize % 2 == 1) {
    cout << ' ';
  }
  cout << bannerFrameChar << endl;
}

void print_banner_frame_line()
{
  static const string frame(bannerWidth, bannerFrameChar);
  cout << frame << endl;
}

void print_banner()
{
  cout << endl;
  print_banner_frame_line();
  print_banner_content_line("");
  print_banner_content_line("applgrid2yoda");
  print_banner_content_line("E. Bothmann");
  print_banner_content_line("");
  print_banner_content_line("WARNING: the errors and entry numbers in the");
  print_banner_content_line("resulting YODA histograms are not meaningful");
  print_banner_content_line("");
  print_banner_content_line("for an automated creation of APPLgrids and");
  print_banner_content_line("fastNLO tables, check out mcgrid.hepforge.org");
  print_banner_frame_line();
  cout << endl;
}

void print_pdf_feedback(string const & pdf_set_name, range const & pdf_subset_range) {
  cout << "Will use ";
  if (pdf_subset_range.second == 1 && pdf_subset_range.first == 0) {
    cout << "central value set";
  } else if (pdf_subset_range.second == 1) {
    cout << "subset " << pdf_subset_range.first;
  } else {
    cout << "subsets " << pdf_subset_range.first;
    cout << " to " << pdf_subset_range.first + pdf_subset_range.second - 1;
  }
  cout << " of " << pdf_set_name << endl;
}

vector<YODA::HistoBin1D> bins_from_grid(appl::grid const & g)
{
  vector<YODA::HistoBin1D> bins;
  size_t bin_count = g.Nobs();
  for (size_t bin_index(0); bin_index < bin_count; bin_index++) {
    double left(g.obslow(bin_index));
    double right(left + g.deltaobs(bin_index));
    bins.push_back(YODA::HistoBin1D(left, right));
  }
  return bins;
}

string basename_without_extension_from_path(string const & path) {
  char path_c_str[path.size() + 1];
  path.copy(path_c_str, sizeof path_c_str);
  path_c_str[path.size()] = '\0';
  const char *file_name_c_str = basename(path_c_str);
  const string file_name(file_name_c_str);
  const size_t delimiter_pos = file_name.find_last_of('.');
  return file_name.substr(0, delimiter_pos);
}

int main(int argc, char* argv[]) {

  // Set defaults
  range pdf_subset_range(0, 1);
  string pdf_set_name("CT10");

  if (argc < 3) {
    print_usage(argv[0]);
    exit(-1);
  }

  print_banner();

  // Read grid name
  string grid_file_path;
  grid_file_path.assign(argv[1]);

  // Read Rivet ID
  string rivet_path;
  rivet_path.assign(argv[2]);

  // Read PDF set name and subset index
  if (argc > 3) {
    pdf_set_name.assign(argv[3]);
  }
  if (argc > 4) {
    string pdf_subset_arg;
    pdf_subset_arg.assign(argv[4]);
    pdf_subset_range = range_from_string(pdf_subset_arg);
  }

  print_pdf_feedback(pdf_set_name, pdf_subset_range);

  // Read grid
  appl::grid g(grid_file_path);

  // Obtain bins
  vector<YODA::HistoBin1D> bins = bins_from_grid(g);

  // Setup YODA file output
  YODA::Writer & writer = YODA::WriterYODA::create();
  string histogram_file_name_prefix = basename_without_extension_from_path(grid_file_path) + "_" + pdf_set_name + "_";

  // Do convolutions and write histograms
  const int loops_count = 1;
  for (size_t subset_index(pdf_subset_range.first);
       subset_index < pdf_subset_range.first + pdf_subset_range.second;
       subset_index++)
  {
    LHAPDF::initPDFSet(pdf_set_name, LHAPDF::LHGRID, subset_index);
    g.convolute(evolvepdf_, alphaspdf_, loops_count);
    vector<double> cross_sections = g.vconvolute(evolvepdf_, alphaspdf_, loops_count);
    ostringstream histogram_file_name;
    histogram_file_name << histogram_file_name_prefix << subset_index << ".yoda";
    YODA::Histo1D *histogram = new YODA::Histo1D(bins, rivet_path, histogram_file_name.str());
    for (size_t bin_index(0); bin_index < bins.size(); bin_index++) {
      histogram->fillBin(bin_index, cross_sections[bin_index] * bins[bin_index].xWidth());
    }
    std::vector<YODA::AnalysisObject *> analysis_objects(1, histogram);
    writer.write(histogram_file_name.str(), analysis_objects);
    delete histogram;
  }
}

// Ensure compatibility to LHAPDF v.5.x
#ifndef LHAPDF_MAJOR_VERSION
#include "LHAPDF/FortranWrappers.h"
#ifdef FC_DUMMY_MAIN
int FC_DUMMY_MAIN() { return 1; }
#endif
#endif
