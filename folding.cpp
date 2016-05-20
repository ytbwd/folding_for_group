#include "folding.h"
#include "ft_spring_solver.h"
#include <fstream>
#include <sstream>

static void setCollisionFreePoints3d(INTERFACE*, Drag*);

double Folder::m_thickness = 0.001;
double Folder::max_dt = 0.01;

void Folder::addDragsFromFile(std::string fname) {
    std::ifstream ifs(fname);
    std::string line;
    double tmp;
    while (getline(ifs,line)) {
        Drag::Info info;
	std::istringstream ss(line);
	ss >> info.id();
	while (ss >> tmp) 
	    info.data().push_back(tmp);
	addDrag(Drag::dragFactory(info));
    }
}

void Folder3d::doFolding() {
    FT_SpringSolver* sp_solver = new FT_SpringSolver(m_intfc);
    CollisionSolver* cd_solver = new CollisionSolver3d();
    
    double k = 200;
    double lambda = 1.0;
    double m = 0.01;
    //configure collision solver
    cd_solver->assembleFromInterface(m_intfc,getFrameStepSize());
    cd_solver->setSpringConstant(k);
    cd_solver->setFrictionConstant(0.0);
    cd_solver->setPointMass(m);
    cd_solver->setFabricThickness(getThickness());
  
    //configure spring solver
    sp_solver->assemblePoints();

    /********debugging*********/
    sp_solver->printAdjacencyList("adj_list.dat");
    sp_solver->printPointList("pts_list.dat");
    /**************************/
    sp_solver->setParameters(k,lambda,m);
    for (std::vector<Drag*>::iterator it = drags.begin();
	 it != drags.end(); ++it) 
    {
	printf("folding base on %lu drag ...\n",it-drags.begin());
	doFolding(*it,sp_solver,cd_solver);
	sp_solver->resetVelocity();
    }
}

void Folder3d::doFolding(
     Drag* drag, 
     FT_SpringSolver* sp_solver,
     CollisionSolver* cd_solver) 
{
    double min_dt = sp_solver->getTimeStepSize();
    double max_dt = std::max(min_dt,getFrameStepSize());
    
    sp_solver->setDrag(drag);

    setCollisionFreePoints3d(m_intfc,drag);

    //call spring solver and collision solver
    //based on adaptive dt
    static double t = 0;
    double dt = max_dt;
    while (t < drag->m_t - MACH_EPS) {
	printf("--------------------------------\n");
	printf("dt = %f, t = %f, total time = %f\n",
		dt,t,drag->m_t);
	printf("--------------------------------\n");
	if (t+dt > drag->m_t) 
	    dt = drag->m_t-t;

	cd_solver->recordOriginPosition();
	cd_solver->setTimeStepSize(dt);

    	sp_solver->doSolve(dt);
	sp_solver->updateVelocityToState();

	//cd_solver->resolveCollision();
	sp_solver->updateVelocityFromState();

	t = t + dt;
	if (movie->isMovieTime(t))
	    movie->recordMovieFrame();
    }
}

void Folder3d::setupMovie(std::string dname,
			double dt) {
    movie = new Movie();
    movie->mv_dt = dt;
    movie->mv_dir = dname;
    movie->doMakeMovie = true;
    movie->mv_intfc = this->m_intfc;
    std::string sys_cmd = "mkdir -p " + dname;
    system(sys_cmd.c_str());
}

void Movie::recordMovieFrame() {
    std::string fname = mv_dir + "/intfc-" + std::to_string(mv_count++);
    gview_plot_interface(fname.c_str(),mv_intfc);
}

bool Movie::isMovieTime(double t) {
    if (doMakeMovie)
        return fabs(t-mv_count*mv_dt) < 1e-10;
    else
	return false;
}

static void setCollisionFreePoints3d(INTERFACE* intfc, Drag* drag)
{
        POINT *p;
        HYPER_SURF *hs;
        HYPER_SURF_ELEMENT *hse;
        assert (intfc->dim == 3);

        next_point(intfc,NULL,NULL,NULL);
        while(next_point(intfc,&p,&hse,&hs)){
            STATE* sl = (STATE*)left_state(p);
            sl->is_fixed = false;
            if (drag->isPresetPoint(Coords(p))||
                wave_type(hs) == NEUMANN_BOUNDARY ||
                wave_type(hs) == MOVABLE_BODY_BOUNDARY)
            {
                sl->is_fixed = true;
            }
        }

        CURVE **c;
        BOND* b;
        intfc_curve_loop(intfc,c){
            if (hsbdry_type(*c) != FIXED_HSBDRY)
                continue;
            for (b = (*c)->first; b != (*c)->last; b = b->next)
            {
                STATE* sl = (STATE*)left_state(b->end);
                sl->is_fixed = true;
            }
        }
        NODE** n;
        intfc_node_loop(intfc,n){
            STATE* sl = (STATE*)left_state((*n)->posn);
            sl->is_fixed = false;
            if (drag->isPresetPoint(Coords((*n)->posn)))
            {
                sl->is_fixed = true;
            }
        }
}       /* setCollisionFreePoints3d() */

Folder3d::Folder3d(INTERFACE* intfc, SURFACE* s) : m_intfc(intfc)
{}

Folder3d::~Folder3d() {}
