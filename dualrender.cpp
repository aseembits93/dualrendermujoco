//dualrender.cpp

#include "mujoco.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


#include "glfw3.h"


//-------------------------------- global data ------------------------------------------

struct vis_struct 
{// MuJoCo model and data
mjModel* m;
mjData* d;

// MuJoCo visualization
mjvScene scn;
mjvCamera cam;
mjvOption opt;
mjrContext con;
// get size of active renderbuffer
mjrRect viewport;
int W;
int H;
// allocate rgb and depth buffers
unsigned char* rgb;
float* depth;
GLFWwindow* window=NULL;
};

struct sim_struct 
{// MuJoCo model and data
mjModel* m =0;
mjData* d =0;
};

//-------------------------------- utility functions ------------------------------------

// load model, init simulation and rendering
void initMuJoCo(void)
{
// activate
mj_activate("mjkey.txt");



}

sim_struct *initsimstruct(const char* filename)
{
sim_struct *s = (sim_struct*)malloc(sizeof (sim_struct));
// load and compile
char error[1000] = "Could not load binary model";
if( strlen(filename)>4 && !strcmp(filename+strlen(filename)-4, ".mjb") )
s->m = mj_loadModel(filename, 0);
else
s->m = mj_loadXML(filename, 0, error, 1000);
if( !s->m )
mju_error_s("Load model error: %s", error);

// make data, run one computation to initialize all fields
s->d = mj_makeData(s->m);
mj_forward(s->m, s->d);
return s;

}
vis_struct *initvisstruct(sim_struct* s)
{
vis_struct *v = (vis_struct*)malloc(sizeof (vis_struct));
// create invisible window, single-buffered
v->window = glfwCreateWindow(1200, 900, "Invisible window", NULL, NULL);
if( !v->window )
mju_error("Could not create GLFW window");
// make context current
glfwMakeContextCurrent(v->window);
glfwSwapInterval(0);
v->m=s->m;
v->d=s->d;


// initialize visualization data structures
mjv_defaultCamera(&v->cam);
mjv_defaultOption(&v->opt);
mjv_defaultScene(&v->scn);
mjr_defaultContext(&v->con);

// create scene and context
mjv_makeScene(v->m, &v->scn, 2000);
mjr_makeContext(v->m, &v->con, 200);

// center and scale view
v->cam.lookat[0] = v->m->stat.center[0];
v->cam.lookat[1] = v->m->stat.center[1];
v->cam.lookat[2] = v->m->stat.center[2];
v->cam.distance = 1.5 * v->m->stat.extent;

// get size of active renderbuffer
v->viewport =  mjr_maxViewport(&v->con);
v->W = v->viewport.width;
v->H = v->viewport.height;
// allocate rgb and depth buffers
v->rgb = (unsigned char*)malloc(3*v->W*v->H);
v->depth = (float*)malloc(sizeof(float)*v->W*v->H);
if( !v->rgb || !v->depth )
mju_error("Could not allocate buffers");
return v;
}

// deallocate everything and deactivate
void closevisstruct(vis_struct* v)
{
free(v->rgb);
free(v->depth);
v->d=NULL;
v->m=NULL;
v->depth=NULL;
v->rgb=NULL;
mjr_freeContext(&v->con);
mjv_freeScene(&v->scn);
}

// deallocate everything and deactivate
void closesimstruct(sim_struct* s)
{
mj_deleteData(s->d);
mj_deleteModel(s->m);
s->d=NULL;
s->m=NULL;
free(s);
}

// deallocate everything and deactivate
void closeMuJoCo(void)
{
mj_deactivate();
}

void visdraw(vis_struct* v)
{
// Set up for rendering
glfwMakeContextCurrent(v->window);
// update abstract scene
mjv_updateScene(v->m, v->d, &v->opt, NULL, &v->cam, mjCAT_ALL, &v->scn);
// render scene in offscreen buffer
mjr_render(v->viewport, &v->scn, &v->con);
// read rgb and depth buffers
mjr_readPixels(v->rgb, v->depth, v->viewport, &v->con);
glfwSwapBuffers(v->window);
}

// create OpenGL context/window
void initOpenGL(void)
{
// init GLFW
if( !glfwInit() )
mju_error("Could not initialize GLFW");
}


// close OpenGL context/window
void closeOpenGL(vis_struct* v)
{
glfwDestroyWindow(v->window);
v->window=NULL;
free(v);
}


//-------------------------------- main function ----------------------------------------

int main(int argc, const char** argv)
{
// check command-line arguments
if( argc!=5 )
{
printf(" USAGE:  record modelfile duration fps rgbfile\n");
return 0;
}

// parse numeric arguments
double duration = 10, fps = 30;
sscanf(argv[2], "%lf", &duration);
sscanf(argv[3], "%lf", &fps);

// initialize OpenGL and MuJoCo

;




initOpenGL();
initMuJoCo();
printf("%s",argv[1]);
sim_struct* s1 = initsimstruct(argv[1]);
sim_struct* s2 = initsimstruct(argv[1]);

vis_struct* v1= initvisstruct(s1);    
vis_struct* v2 = initvisstruct(s2);




// main loop
double frametime = 0;
int framecount = 0;
while(s1->d->time<duration )
{
// render new frame if it is time (or first frame)
if( (s1->d->time-frametime)>1/fps || frametime==0 )
{

visdraw(v1);
visdraw(v2);
// print every 10 frames: '.' if ok, 'x' if OpenGL error
if( ((framecount++)%10)==0 )
{
if( mjr_getError() )
printf("x");
else
printf(".");
}

// save simulation time
frametime = s1->d->time;
}

// advance simulation
mj_step(s1->m, s1->d);
// advance simulation
mj_step(s2->m, s2->d);
}
printf("\n");

// close MuJoCo and OpenGL
closesimstruct(s1);
closevisstruct(v1);
closevisstruct(v2);
closeMuJoCo();
closeOpenGL(v1);
closeOpenGL(v2);

return 1;
}
