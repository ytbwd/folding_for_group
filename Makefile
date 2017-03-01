CXX=mpicxx -g -rdynamic -fopenmp -pedantic -Wno-long-long -Wno-unused-result 

incs =   -I.. -I../util   -I/usr/local/pkg/hdf/include -DUSE_HDF   -DHAS_FENV    -D__MPI__  -D__GD__ -I/usr/local/pkg/gd/include  -D__HYPRE__   -I../include -D__DAMPING__
libincs =  -L/usr/local/pkg/gd/lib  -L/usr/lib -L/usr/local/pkg/hdf/lib  -L../lib/x86_64 -L.
libs = -lgd   -lmfhdf -ldf -ljpeg -lz  -lmpich -lpthread -lmpfr -lm

CGAL_DIR=/usr/local/pkg/cgal
CVODE_DIR=/usr/local/pkg/cvode

CGAL_Include=-I/usr/local/pkg/cgal/include
CGAL_Lib=-L/usr/local/pkg/cgal/lib -lCGAL_Core -lCGAL_ImageIO -lCGAL -lCGAL_Core -lCGAL_ImageIO -lCGAL

CVODE_Include=-I/usr/local/pkg/cvode/include
CVODE_Lib=-L/usr/local/pkg/cvode/lib -lsundials_cvode -lsundials_nvecserial

test: test.o folding.o folding_helper.o drag.o dcollid3d.o dcollid.o spring_solver.o drag_proto.o ex_spring_solver.o im_spring_solver.o bending.o libiF.a cgal.o
	$(CXX) $^ -lFronTier -o test $(libincs) $(libs) $(incs) $(CGAL_Include) $(CGAL_Lib) $(CVODE_Include) $(CVODE_Lib) -lgmp -lmpfr -frounding-math -liF
dcollid3d.o: ../Collision/dcollid3d.cpp
	$(CXX) $< -c $(incs) $(CGAL_Include) -frounding-math
dcollid.o: ../Collision/dcollid.cpp
	$(CXX) $< -c $(incs) $(CGAL_Include) -frounding-math
cgal.o: cgal.cpp
	$(CXX) $< -c $(incs) $(libincs) $(CGAL_Include) $(CGAL_Lib) -L/usr/lib64 -lgmp -lmpfr -frounding-math
test.o: test.cpp
	$(CXX) $< -c $(incs) $(libincs) $(CGAL_Include) -frounding-math
im_spring_solver.o: im_spring_solver.cpp
	$(CXX) $< -c $(incs) $(CVODE_Include) $(CVODE_Lib) -frounding-math
%.o: %.cpp
	$(CXX) $< -c $(incs) $(CGAL_Include) -frounding-math

-include ../devel-deps.inc

libiF.a: ../iFluid/*.cpp
	cd ../iFluid; make
	$(AR) cr libiF.a ../iFluid/*.o
	ranlib libiF.a
clean:
	rm -rf *.o test libiF.a
tagsfile:
	ctags *.h *.cpp ../Collision/*.h ../Collision/*.cpp ../src/*/*.[chf]

