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

// IO colors
#include "IOColors.hh"

// Argument parser
#include "argparse.hpp"

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

static const string::size_type bannerWidth = 54;
static const char bannerFrameChar = '*';

void print_banner_content_line(const string line, const string::size_type length)
{
  const string::size_type totalMarginSize = bannerWidth - length - 2;
  const string::size_type oneSideMarginSize = totalMarginSize / 2;
  const string margin(oneSideMarginSize, ' ');
  cout << bannerFrameChar << margin << line << margin;
  if (totalMarginSize % 2 == 1) {
    cout << ' ';
  }
  cout << bannerFrameChar << endl;
}

void print_banner_content_line(const string line)
{
  print_banner_content_line(line, line.size());
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

  ostringstream colored_program_name;
  string program_name("applgrid2yoda");
  colored_program_name << bold << program_name << clearatt;
  string colored_program_name_str(colored_program_name.str());
  print_banner_content_line(colored_program_name_str, program_name.size());

  print_banner_content_line("E. Bothmann");
  print_banner_content_line("");

  ostringstream colored_warning;
  string warning_title("Warning");
  string warning_message(": the errors and entry numbers in the");
  colored_warning << red << underline << warning_title << clearatt << warning_message;
  string colored_warning_str(colored_warning.str());
  print_banner_content_line(colored_warning_str, warning_title.size() + warning_message.size());

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

void print_scale_feedback(vector<string> const & scale_factors) {
  cout << "Will use ";
  if (scale_factors.size() == 1) {
    cout << "scale factor ";
  } else {
    cout << "scale factors ";
  }
  for(vector<string>::const_iterator it = scale_factors.begin(); it != scale_factors.end(); ++it) {
    cout << *it;
    if (it+1 != scale_factors.end()) {
      cout << ", ";
    }
  }
  cout << endl;
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

int main(int argc, const char* argv[]) {

  // Set defaults
  range pdf_subset_range_default(0, 1);
  string pdf_set_name_default("CT10");

  print_banner();

  ArgumentParser parser;
  parser.addArgument("-i", "--rivet-id", 1);
  parser.addArgument("-p", "--pdf-set", 1);
  parser.addArgument("-m", "--pdf-members", 1);
  parser.addArgument("-s", "--scale-factors", '+');
  parser.addFinalArgument("grid");

  parser.parse(argc, argv);

  // Read grid file path
  string grid_file_path = parser.retrieve<string>("grid");
  if (grid_file_path == "") {
    cout << parser.usage();
    exit(-1);
  }

  // Read or construct Rivet ID
  string rivet_id = parser.retrieve<string>("rivet-id");
  if (rivet_id == "") {
    cout << "Rivet ID not given explicitly. Try to parse from grid path ...  ";
    // "[...]/ANALYSIS/HISTO.root" -> "/ANALYSIS/HISTO"
    string::size_type dot_index = grid_file_path.find_last_of(".");
    if (dot_index != string::npos) {
      string base_path = grid_file_path.substr(0, dot_index);
      string::size_type slash_index = base_path.find_last_of("/");
      if (slash_index != string::npos) {
        string::size_type second_slash_index = base_path.find_last_of("/", slash_index - 1);
        if (second_slash_index == string::npos) {
          rivet_id = "/" + base_path;
        } else {
          rivet_id = base_path.substr(second_slash_index);
        }
      }
    }
    if (rivet_id == "") {
      cout << "not found" << endl;
      cerr << "Please provide a Rivet ID using the `--rivet-id' option." << endl;
      exit(-1);
    } else {
      cout << rivet_id << endl;
    }
  }

  // Read PDF set name and subset index
  string pdf_set_name = parser.retrieve<string>("pdf-set");
  if (pdf_set_name == "") {
    pdf_set_name = pdf_set_name_default;
  }
  range pdf_subset_range;
  string pdf_members = parser.retrieve<string>("pdf-members");
  if (pdf_members == "") {
    pdf_subset_range = pdf_subset_range_default;
  } else {
    pdf_subset_range = range_from_string(pdf_members);
  }

  print_pdf_feedback(pdf_set_name, pdf_subset_range);

  // Read scale factors
  vector<string> scale_factors;
  if (parser.count("scale-factors") == 0) {
    scale_factors.push_back("0.0");
  } else if (parser.count("scale-factors") == 1) {
    scale_factors.push_back(parser.retrieve<string>("scale-factors"));
  } else if (parser.count("scale-factors") > 1) {
    scale_factors = parser.retrieve<vector<string> >("scale-factors");
  }

  print_scale_feedback(scale_factors);

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
    for(vector<string>::const_iterator scale_factor_it = scale_factors.begin();
        scale_factor_it != scale_factors.end();
        ++scale_factor_it)
    {
        const double scale_factor = (const double)(atof((*scale_factor_it).c_str()));
        LHAPDF::initPDFSet(pdf_set_name, LHAPDF::LHGRID, subset_index);
        g.convolute(evolvepdf_, alphaspdf_, loops_count);
        vector<double> cross_sections = g.vconvolute(evolvepdf_, alphaspdf_, loops_count, scale_factor, scale_factor);
        ostringstream histogram_file_name;
        histogram_file_name << histogram_file_name_prefix << subset_index << '_' << *scale_factor_it << ".yoda";
        YODA::Histo1D *histogram = new YODA::Histo1D(bins, rivet_id, histogram_file_name.str());
        for (size_t bin_index(0); bin_index < bins.size(); bin_index++) {
          histogram->fillBin(bin_index, cross_sections[bin_index] * bins[bin_index].xWidth());
        }
        vector<YODA::AnalysisObject *> analysis_objects(1, histogram);
        writer.write(histogram_file_name.str(), analysis_objects);
        delete histogram;
    }
  }
}

// Ensure compatibility to LHAPDF v.5.x
#ifndef LHAPDF_MAJOR_VERSION
#include "LHAPDF/FortranWrappers.h"
#ifdef FC_DUMMY_MAIN
int FC_DUMMY_MAIN() { return 1; }
#endif
#endif
