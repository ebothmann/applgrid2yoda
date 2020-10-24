*NOTE: This repository has been moved to https://gitlab.com/mcgrid/applgrid2yoda.*

# applgrid2yoda
This script convolutes an APPLgrid with an LHAPDF and writes out the resulting histograms as YODA files.

## usage

```
./applgrid2yoda [--rivet-id RIVET-ID] \
        [--pdf-set PDF-SET] [--pdf-members PDF-MEMBERS] \
        [--scale-factors REN-SCALE-FACTOR,FAC-SCALE-FACTOR [SCALE-FACTORS...]] GRID
```

- `GRID` is the path to the ROOT file written out by APPLgrid (required)
- the `RIVET-ID` is given by `/analysis/histogram`, if it is not given, the script will try to construct the Rivet ID from the last two components of the `GRID` path
- the `PDF-SET` is the name of the PDF set passed to LHAPDF (optional, default: "CT10")
- `PDF-MEMBERS` can be given as a single number or a range (optional, default: "0", i.e. the CV PDF member)
    - if a single number (e.g. "7") is given, then only the PDF member for this index will be used
    - if two numbers connected with three dots are given (e.g. "0...52"), they are interpreted as a range including the endpoints
- `REN-SCALE-FACTOR` the (unsquared) renormalisation scale factor
- `FAC-SCALE-FACTOR` the (unsquared) factorisation scale factor
- the scale factors are separated by a comma, additional scale pairs are separates by spaces
- identical scale factors in a pair can be given as a single value (e.g. "2.0" instead of "2.0,2.0")
    
## example

```
./applgrid2yoda --pdf-set CT10 --pdf-members 0...52 \
        mcgrid/MCgrid_CDF_2009_S8383952/d02-x01-y01.root
```

This will convolute the APPLgrid `d02-x01-y01.root` with all CT10 members, writing out one YODA file per member.
This could be used to generate a PDF error band for the observable.
Note that the last two components of the path here give the Rivet ID, so it is not necessary to specify it explicitly.
