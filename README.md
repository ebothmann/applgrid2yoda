# applgrid2yoda
This script convolutes an APPLgrid with an LHAPDF and writes out the resulting histograms as YODA files.

## usage

```
./applgrid2yoda <applgrid_file_name> <rivet_id> [[pdf_set_name] subset_start[...subset_end]]
```

- the `applgrid_file_name` is the path to the ROOT file written out by APPLgrid (required)
- the `rivet_id` is given by `/analysis/histogram` (required)
- the `pdf_set_name` is the name of the PDF set passed to LHAPDF (optional, default: "CT10")
- subsets indizes can be given using `subset_start` and `subset_end` (optional, default: "0", i.e. the CV PDF member)
    - if only `subset_start` is given, then only the PDF member for this index will be used
    - if both indizes are given (e.g. "0...52"), they are interpreted as a range including the endpoints
    
## example

```
./applgrid2yoda mcgrid/MCgrid_CDF_2009_S8383952/d02-x01-y01.root /MCgrid_CDF_2009_S8383952/d02-x01-y01 CT10 0...52
```

This will convolute the APPLgrid `d02-x01-y01.root` with all CT10 members, writing out one YODA file per member.
This could be used to generate a PDF error band for the observable.
