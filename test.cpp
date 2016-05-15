#include <FronTier.h>
#include <string>
#include "folding.h"
#include "spring_solver.h"
#include "folding_state.h"
#include <fenv.h>

//test module for folding algorithm
static void initTestModule(Front*,SURFACE*&);
static void initAirbag(Front*,FILE*,SURFACE*&);
static void initFabric(Front*,FILE*,SURFACE*&);
//static void myOptimizeSurf(Front *);

int main(int argc, char** argv)
{
	feenableexcept(FE_INVALID | FE_OVERFLOW);
	static Front front;
	static F_BASIC_DATA f_basic;
        static LEVEL_FUNC_PACK level_func_pack;
        f_basic.dim = 3;
        FT_Init(argc,argv,&f_basic);
        f_basic.size_of_intfc_state = sizeof(STATE);

        /* Initialize basic computational data */
        char* in_name                 = f_basic.in_name;
	//initialize interface and velocity
        FT_ReadSpaceDomain(in_name,&f_basic);
        FT_StartUp(&front,&f_basic);
        FT_InitDebug(in_name);

        level_func_pack.pos_component = 2;
        FT_InitIntfc(&front,&level_func_pack);
	
	SURFACE* surf;
	initTestModule(&front,surf);

	Folder* folder = new Folder3d(front.interf,surf);
	
	//point drag
	/*double center[] = {0.5, 0.5, 0.2};
	double vel[] = {0.0,0.0,0.0};
 	double a[] = {0.0,0.0,0.5};
	double rad = 0.1;
	double t = 1.0;
	Drag* drag = new PointDrag(center,rad,vel,a,t);		
	*/
	
	//gravity drag
	double center[] = {0.5, 0.5, 0.2};
	double a[] = {0.0,0.0,4.0};
	double rad = 1.1*(front.rect_grid)->h[0];
	double t = 2;
	Drag* drag = new GravityDrag(center,rad,a,t);

	folder->addDrag(drag);

	folder->doFolding();
	FT_Draw(&front);
	FT_AddTimeStepToCounter(&front);
	clean_up(0);
}

void initTestModule(Front* front, SURFACE* &surf) {
	FILE *infile = fopen(InName(front),"r");
	char mesg[256];
	CursorAfterString(infile,"Enter problem type:");
	fscanf(infile,"%s",mesg);
	std::string prob_type(mesg);
	if (prob_type == "Airbag")
	    initAirbag(front,infile,surf);
	else if (prob_type == "Fabric")
	    initFabric(front,infile,surf);
	else
	    printf("Unknown type = %s\n",mesg);
	fclose(infile);
}

void initAirbag(Front* front, FILE* infile, SURFACE* &surf) {
	double center[3], radius[3];
	CursorAfterString(infile,"Enter center of airbag:");
	fscanf(infile,"%lf %lf %lf",center,center+1,center+2);
	printf("%f %f %f\n",center[0],center[1],center[2]);

	CursorAfterString(infile,"Enter radius of airbag:");
	fscanf(infile,"%lf %lf %lf",radius,radius+1,radius+2);
	printf("%f %f %f\n",radius[0],radius[1],radius[2]);

	COMPONENT neg_comp = 2;
	COMPONENT pos_comp = 3;
	int w_type = ELASTIC_BOUNDARY;
	FT_MakeEllipticSurf(front,center,
                            radius,
                            neg_comp,pos_comp,
                            w_type,1,
                            &surf);
}

void initFabric(Front* front, FILE* infile, SURFACE* &surf) {
 	static PLANE_PARAMS plane_params;
	static LEVEL_FUNC_PACK level_func_pack;

	CursorAfterString(infile,"Enter the height of the plane:");
        fscanf(infile,"%lf",&plane_params.P[2]);
        (void) printf("%f\n",plane_params.P[2]);
        plane_params.N[0] = plane_params.N[1] = 0.0;
        plane_params.N[2] = 1.0;
	

	static CIRCLE_PARAMS circle_constr_params;

	level_func_pack.wave_type = ELASTIC_BOUNDARY;
	level_func_pack.set_3d_bdry = YES;
        level_func_pack.neg_component = 3;
        level_func_pack.pos_component = 3;
	level_func_pack.func_params = (POINTER)&plane_params;
        level_func_pack.func = plane_func;
        level_func_pack.is_mono_hs = YES;
        level_func_pack.constr_params = (POINTER)&circle_constr_params;
        level_func_pack.constr_func = circle_constr_func;
        level_func_pack.num_mono_hs = 1;
        CursorAfterString(infile,"Enter circle center:");
        fscanf(infile,"%lf %lf",&circle_constr_params.cen[0],
                                &circle_constr_params.cen[1]);
        (void) printf("%f %f\n",circle_constr_params.cen[0],
                                circle_constr_params.cen[1]);
	CursorAfterString(infile,"Enter circle radius:");
        fscanf(infile,"%lf",&circle_constr_params.R);
        (void) printf("%f\n",circle_constr_params.R);

	FT_InitIntfc(front,&level_func_pack);	
	SURFACE** s;
	intfc_surface_loop(front->interf,s) {
	    if (wave_type(*s) == level_func_pack.wave_type) {
		surf = *s;
		break;
	    }
	}
}
