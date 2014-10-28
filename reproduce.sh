module swap cce/8.3.4
module load cudatoolkit
module load craype-accel-nvidia35
module load cray-netcdf
FTN="ftn -D__CRAY_FORTRAN__ -c -eZ -DHAS_IOMSG -DSINGLEPRECISION -D__COSMO__ -D_LOC_TIMING -DGSP_FIRST -DNUDGING -DRTTOV7 -DGRIBDWD -DNETCDF -N255 -ec -eC -eI -eF -hnosecond_underscore -hflex_mp=conservative -Ofp1 -hadd_paren -hacc -ra -O2 -I. -I${CRAY_MPICH2_DIR}/include -D__MPICH2"
FTN="ftn -D__CRAY_FORTRAN__ -c -eZ -DHAS_IOMSG -DCRAY_FIX_WKARR -D__COSMO__ -D_LOC_TIMING -DGSP_FIRST -DNUDGING -DRTTOV7 -DGRIBDWD -DNETCDF -N255 -ec -eC -eI -eF -hnosecond_underscore -hflex_mp=conservative -Ofp1 -hadd_paren -hacc -ra -O2 -I. -I${CRAY_MPICH2_DIR}/include -D__MPICH2"
FILES="`cat file_list.txt`"

for file in $FILES
do
    echo "---------------------------------------------------------------------"
    echo "compiling $file.f90"
    echo $FTN -o $file.o $file.f90
    $FTN -o $file.o $file.f90
done


