/*
  CSCI 420
  Assignment 2 Roller Coasters
  Enyu Zhao

 */
#define GL_SILENCE_DEPRECATION
#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include "pic.h"
#include <algorithm>
#include <typeinfo>
#include <iostream>
#include <math.h>


using namespace std;

int g_iMenuId;

int g_vMousePos[2] = {0, 0};

int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

int display_mode=0;// defines display_mode

float param_u=0.0;
int control_point_0=1;
double x_max=0,y_max=0,z_max=0,x_min=0,y_min=0,z_min=0;


typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;

CONTROLSTATE g_ControlState = ROTATE;


/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

/* represents one control point along the spline */
struct point {
   double x;
   double y;
   double z;
};

/* spline struct which contains how many control points, and an array of control points */
struct spline {
   int numControlPoints;
   struct point *points;
};


// for cross section construction//
struct cross_sec {
   point pos;
   point tan;
   point norm;
   point binorm;
   point railpoint[16];
};


double sec_coef[16][2]={
  -0.5,-1.0,//v0
  -0.35,-1.0,//v1
  -0.5,-0.98,//v2
  -0.35,-0.98,//v3

  -0.45,-0.98,//v4
  -0.4,-0.98,//v5

  -0.45,-0.83,//v6
  -0.4,-0.83,//v7


  0.35,-1.0,//v8
  0.5,-1.0,//v9
  0.35,-0.98,//v10
  0.5,-0.98,//v11

  0.4,-0.98,//v12
  0.45,-0.98,//v13

  0.4,-0.83,//v14
  0.45,-0.83//v15



};



/* the spline array */
struct spline *g_Splines;

/* total number of splines */
int g_iNumOfSplines;



int texture[3];



int border=100;

double bruteforce_u_stepsize=0.1;
double s_of_catmull=0.5;
double max_line_length=0.1;
double time_step=0.001;
bool start=false;
point b_now;// for camera movement

point start_b0;






int loadSplines(char *argv) {
  char *cName = (char *)malloc(128 * sizeof(char));
  FILE *fileList;
  FILE *fileSpline;
  int iType, i = 0, j, iLength;


  /* load the track file */
  fileList = fopen(argv, "r");
  if (fileList == NULL) {
    printf ("can't open file\n");
    exit(1);
  }
  
  /* stores the number of splines in a global variable */
  fscanf(fileList, "%d", &g_iNumOfSplines);//first number in txt file

  g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

  /* reads through the spline files */
  for (j = 0; j < g_iNumOfSplines; j++) {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL) {
      printf ("can't open file\n");
      exit(1);
    }

    /* gets length for spline file */
    fscanf(fileSpline, "%d %d", &iLength, &iType);//ilength for the number of control points

    /* allocate memory for all the points */
    g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
    g_Splines[j].numControlPoints = iLength;

    /* saves the data to the struct */
    while (fscanf(fileSpline, "%lf %lf %lf", 
	   &g_Splines[j].points[i].x, 
	   &g_Splines[j].points[i].y, 
	   &g_Splines[j].points[i].z) != EOF) {
      i++;
    }
  }

  free(cName);

  for(int i=0;i<g_iNumOfSplines;i++)
  {
    
    for (int j=0;j<g_Splines[i].numControlPoints-1;j++)
    {
          x_max=g_Splines[i].points[j].x>x_max?g_Splines[i].points[j].x:x_max;
          y_max=g_Splines[i].points[j].y>y_max?g_Splines[i].points[j].y:y_max;
          z_max=g_Splines[i].points[j].z>z_max?g_Splines[i].points[j].z:z_max;

          x_min=g_Splines[i].points[j].x<x_min?g_Splines[i].points[j].x:x_min;
          y_min=g_Splines[i].points[j].y<y_min?g_Splines[i].points[j].y:y_min;
          z_min=g_Splines[i].points[j].z<z_min?g_Splines[i].points[j].z:z_min;

          border=fabs(g_Splines[i].points[j].x)>border?fabs(g_Splines[i].points[j].x):border;
          border=fabs(g_Splines[i].points[j].y)>border?fabs(g_Splines[i].points[j].y):border;
          border=fabs(g_Splines[i].points[j].z)>border?fabs(g_Splines[i].points[j].z):border;

    }

  }
  double max_values[6];
  max_values[0]=fabs(x_max);
  max_values[1]=fabs(x_min);
  max_values[2]=fabs(y_max);
  max_values[3]=fabs(y_min);
  max_values[4]=fabs(z_max);
  max_values[5]=fabs(z_min);
  border=max_values[0];

  for(int m=0;m<6;m++)
  {
    border=max_values[m]>border?max_values[m]:border;

  }
  border*=5;
  



  return 0;
}



point  drawsplinepoints(float s, double u, point p1, point p2,point p3,point p4)//draw certain point on given parameter u
{
  //point point_status[2];
  point spline_point;

  double u_2=pow(u,2);
  double u_3=pow(u,3);

  double coef_1=-1*s*u_3+2*s*u_2-s*u;
  //double coef_1_t=-3*u_2+4*s*u-s;

  double coef_2=(2-s)*u_3+(s-3)*u_2+1;
  //double coef_2_t=3*(2-s)*u_2+2*(s-3)*u;

  double coef_3=(s-2)*u_3+(3-2*s)*u_2+s*u;
  //double coef_3_t=3*(s-2)*u_2+2*(3-2*s)*u+s;

  double coef_4=s*u_3-s*u_2;
  //double coef_4_t=3*s*u_2-2*s*u;

  spline_point.x=coef_1*p1.x+coef_2*p2.x+coef_3*p3.x+coef_4*p4.x;
  spline_point.y=coef_1*p1.y+coef_2*p2.y+coef_3*p3.y+coef_4*p4.y;
  spline_point.z=coef_1*p1.z+coef_2*p2.z+coef_3*p3.z+coef_4*p4.z;

  // point_status[0].x=coef_1*p1.x+coef_2*p2.x+coef_3*p3.x+coef_4*p4.x;
  // point_status[0].y=coef_1*p1.y+coef_2*p2.y+coef_3*p3.y+coef_4*p4.y;
  // point_status[0].z=coef_1*p1.z+coef_2*p2.z+coef_3*p3.z+coef_4*p4.z;

  // point_status[1].x=coef_1_t*p1.x+coef_2_t*p2.x+coef_3_t*p3.x+coef_4_t*p4.x;
  // point_status[1].y=coef_1_t*p1.y+coef_2_t*p2.y+coef_3_t*p3.y+coef_4_t*p4.y;
  // point_status[1].z=coef_1_t*p1.z+coef_2_t*p2.z+coef_3_t*p3.z+coef_4_t*p4.z;





  return spline_point;
  //return point_status;




}


point tanVec(float s, double u, point p1, point p2,point p3,point p4)//get norm tan vec about certain point based on given parameter u , combined with the drawsplinepoints
{
  point tan;
  double total_length=1;
  double u_2=pow(u,2);


  double coef_1_t=-3.f*s*u_2+4.f*s*u-s;
  double coef_2_t=3.f*(2-s)*u_2+2.f*(s-3)*u;
  double coef_3_t=3.f*(s-2)*u_2+2.f*(3-2*s)*u+s;
  double coef_4_t=3.f*s*u_2-2.f*s*u;

  tan.x=coef_1_t*p1.x+coef_2_t*p2.x+coef_3_t*p3.x+coef_4_t*p4.x;
  tan.y=coef_1_t*p1.y+coef_2_t*p2.y+coef_3_t*p3.y+coef_4_t*p4.y;
  tan.z=coef_1_t*p1.z+coef_2_t*p2.z+coef_3_t*p3.z+coef_4_t*p4.z;

  total_length=sqrt(pow(tan.x,2)+pow(tan.y,2)+pow(tan.z,2));

  tan.x/=total_length;
  tan.y/=total_length;
  tan.z/=total_length;//normalization





  return tan;
}


point norm_matmul(point p1,point p2)//Do the multiplication for two 3d vectors and normalize the result
{
  point norm_res;

  norm_res.x=p1.y*p2.z-p1.z*p2.y;
  norm_res.y=p1.z*p2.x-p1.x*p2.z;
  norm_res.z=p1.x*p2.y-p1.y*p2.x;

  double total_length=sqrt(pow(norm_res.x,2)+pow(norm_res.y,2)+pow(norm_res.z,2));

  norm_res.x/=total_length;
  norm_res.y/=total_length;
  norm_res.z/=total_length;
  


  return norm_res;
}


//////////////for mouse and keyboard control like assign1 ///////////



void myinit()
{
  /* setup gl view here */
  glClearColor(0.0, 0.0, 0.0, 0.0);
  // enable depth buffering
  glLoadIdentity();
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH); 


  
  //load texture here

  Pic* tex_0;
  tex_0=jpeg_read("0.jpg",NULL);
  //glBindTexture(GL_TEXTURE_2D,texture[0]);
    glBindTexture(GL_TEXTURE_2D,0);

  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,tex_0->nx,tex_0->ny,0,GL_RGB,GL_UNSIGNED_BYTE,&tex_0->pix[0]);
  
  Pic* tex_1;
  tex_1=jpeg_read("1.jpg",NULL);
  //glBindTexture(GL_TEXTURE_2D,texture[0]);
    glBindTexture(GL_TEXTURE_2D,1);

  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,tex_1->nx,tex_1->ny,0,GL_RGB,GL_UNSIGNED_BYTE,&tex_1->pix[0]);

    Pic* tex_2;
  tex_2=jpeg_read("2.jpg",NULL);
  //glBindTexture(GL_TEXTURE_2D,texture[0]);
    glBindTexture(GL_TEXTURE_2D,2);

  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,tex_2->nx,tex_2->ny,0,GL_RGB,GL_UNSIGNED_BYTE,&tex_2->pix[0]);

    Pic* tex_3;
  tex_3=jpeg_read("3.jpg",NULL);
  //glBindTexture(GL_TEXTURE_2D,texture[0]);
    glBindTexture(GL_TEXTURE_2D,3);

  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,tex_3->nx,tex_3->ny,0,GL_RGB,GL_UNSIGNED_BYTE,&tex_3->pix[0]);

    Pic* tex_4;
  tex_4=jpeg_read("4.jpg",NULL);
  //glBindTexture(GL_TEXTURE_2D,texture[0]);
    glBindTexture(GL_TEXTURE_2D,4);

  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,tex_4->nx,tex_4->ny,0,GL_RGB,GL_UNSIGNED_BYTE,&tex_4->pix[0]);

    Pic* tex_5;
  tex_5=jpeg_read("5.jpg",NULL);
  //glBindTexture(GL_TEXTURE_2D,texture[0]);
    glBindTexture(GL_TEXTURE_2D,5);

  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,tex_5->nx,tex_5->ny,0,GL_RGB,GL_UNSIGNED_BYTE,&tex_5->pix[0]);

  Pic* tex_6;
  tex_6=jpeg_read("wood.jpg",NULL);
  //glBindTexture(GL_TEXTURE_2D,texture[0]);
    glBindTexture(GL_TEXTURE_2D,6);

  glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,tex_6->nx,tex_6->ny,0,GL_RGB,GL_UNSIGNED_BYTE,&tex_6->pix[0]);
  gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGB,128,128,GL_RGB,GL_UNSIGNED_BYTE,&tex_6->pix[0]);
  
  
  


  

}

void menufunc(int value)
{
  switch (value)
  {
    case 0:
      exit(0);
      break;
  }
}



void doIdle()
{
  /* do some stuff... */

  /* make the screen update */
  glutPostRedisplay();
}


/* converts mouse drags into information about 
rotation/translation/scaling */
void mousedrag(int x, int y)
{
  int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
  
  switch (g_ControlState)
  {
    case TRANSLATE:  
      if (g_iLeftMouseButton)
      {
        g_vLandTranslate[0] += vMouseDelta[0]*0.01;
        g_vLandTranslate[1] -= vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandTranslate[2] += vMouseDelta[1]*0.01;
      }
      break;
    case ROTATE:
      if (g_iLeftMouseButton)
      {
        g_vLandRotate[0] += vMouseDelta[1];
        g_vLandRotate[1] += vMouseDelta[0];
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandRotate[2] += vMouseDelta[1];
      }
      break;
    case SCALE:
      if (g_iLeftMouseButton)
      {
        g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
        g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
      }
      break;
  }
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void keyboard(unsigned char key, int x, int y)
{
  if (key=='t' || key=='T')// T for translate
  {
    g_ControlState= TRANSLATE;
  }

  if (key == 's' || key == 'S')// S for scale
  {
    g_ControlState=SCALE;
  }


  if (key == '1')// 1 for point mode
  {
    display_mode=0;
  }

  if (key=='2')// 2 for wireframe mode
  {
    display_mode=1;

  }

  if (key=='3')// 3 for triangle mode
  {
    display_mode=2;
  }

  if (key=='4')// 4 for triangle and line mode
  {
    display_mode=3;
  }

}
void mousebutton(int button, int state, int x, int y)
{

  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
  }
 
  switch(glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      g_ControlState = TRANSLATE;
      break;
    case GLUT_ACTIVE_SHIFT:
      g_ControlState = SCALE;
      break;
    default:
      g_ControlState = ROTATE;
      break;
  }

  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}
///////for mouse control like in assign1 for overview and control/////////

void update_cam_one_way()
{
  point movep1,movep2,movep3,movep4,cur_pos,cur_tan,cur_b,cur_n;

  if (param_u>=1)
  {
    control_point_0=control_point_0+1;
    param_u=0;

  }

  if(control_point_0>g_Splines[0].numControlPoints-2)
  {
    control_point_0=1;
    param_u=0;
    display_mode=0;
    return;

  }

  movep1=g_Splines[0].points[control_point_0-1];
  movep2=g_Splines[0].points[control_point_0];
  movep3=g_Splines[0].points[control_point_0+1];
  movep4=g_Splines[0].points[control_point_0+2];

  cur_pos=drawsplinepoints(s_of_catmull,param_u,movep1,movep2,movep3,movep4);
  cur_tan=tanVec(s_of_catmull,param_u,movep1,movep2,movep3,movep4);

  if (control_point_0==1&&param_u==0)
  {
    point v0;
    v0.x=-0.77;
    
    v0.y=-0.5;
    v0.z=-0.5;

    cur_n=norm_matmul(cur_tan,v0);
    cur_b=norm_matmul(cur_tan,cur_n);
    b_now=cur_b;


  }
  else
  {
      cur_n=norm_matmul(b_now,cur_tan);
      cur_b=norm_matmul(cur_tan,cur_n);
      b_now=cur_b;
  }

      



      // cur_pos=drawsplinepoints(s_of_catmull,param_u,movep1,movep2,movep3,movep4);
      // if (param_u==0 && control_point_0>0)
      // {
      //   cur_tan.x=g_Splines[0].points[control_point_0+1].x-g_Splines[0].points[control_point_0-1].x;
      //   cur_tan.y=g_Splines[0].points[control_point_0+1].y-g_Splines[0].points[control_point_0-1].y;
      //   cur_tan.z=g_Splines[0].points[control_point_0+1].z-g_Splines[0].points[control_point_0-1].z;
      //   double total_length=sqrt(pow(cur_tan.x,2)+pow(cur_tan.y,2)+pow(cur_tan.z,2));
      //   cur_tan.x=cur_tan.x/total_length;
      //   cur_tan.y=cur_tan.y/total_length;
      //   cur_tan.z=cur_tan.z/total_length;
      // }
      // else if (control_point_0>0)
      // {cur_tan=tanVec(s_of_catmull,param_u,movep1,movep2,movep3,movep4);}


      //cout<<cur_pos.x <<" "<<cur_pos.y<<" "<<cur_pos.z<<" tangent "<<" "<<cur_tan.x<<" "<<cur_tan.y<<" "<<cur_tan.z<<" "<<param_u<<endl;
      
      


      // if (!start)
      // {
      //   start=true;
      //   point v0;
      //   v0.x=0;
      //   v0.y=0;
      //   v0.z=-1;

      //   cur_n=norm_matmul(cur_tan,v0);
      //   cur_b=norm_matmul(cur_tan,cur_n);
      //   b_now=cur_b;
        


      // }
      // else
      // {
      //   cur_n=norm_matmul(b_now,cur_tan);
      //   cur_b=norm_matmul(cur_tan,cur_n);
      //   b_now=cur_b;
      // }

      
      

      
      gluLookAt(
      cur_pos.x+0.15*cur_n.x+0.15*cur_tan.x, cur_pos.y+0.15*cur_n.y+0.15*cur_tan.y, cur_pos.z+0.15*cur_n.z+0.15*cur_tan.z,
      

      cur_pos.x+10*cur_tan.x, cur_pos.y+10*cur_tan.y, cur_pos.z+10*cur_tan.z, 
      cur_n.x, cur_n.y, cur_n.z
      );
      double delta_u;
      delta_u=time_step*sqrt(2*9.8*(y_max-cur_pos.y))/(sqrt(pow(cur_tan.x,2)+pow(cur_tan.y,2)+pow(cur_tan.z,2)));

      if(delta_u>0.005)
      {param_u+=delta_u;}
      else
      {
        param_u+=0.005;
      }


      //param_u+=0.01;

}



void draw_quad(int i,int j,float grayness,cross_sec sec_before,cross_sec sec_now)
{
  //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glBegin(GL_QUADS);
  glColor3f(grayness, grayness, grayness);
  glVertex3f(sec_now.railpoint[i].x, sec_now.railpoint[i].y, sec_now.railpoint[i].z);

  //glColor3f(grayness, grayness, grayness);
  glVertex3f(sec_before.railpoint[i].x, sec_before.railpoint[i].y, sec_before.railpoint[i].z);


  //glColor3f(grayness, grayness, grayness);
  glVertex3f(sec_before.railpoint[j].x, sec_before.railpoint[j].y, sec_before.railpoint[j].z);

  //glColor3f(grayness, grayness, grayness);
  glVertex3f(sec_now.railpoint[j].x, sec_now.railpoint[j].y, sec_now.railpoint[j].z);
  glEnd();
 

}

void connect_sec(cross_sec sec_before,cross_sec sec_now)
{
  //left lower rail
  draw_quad(0,1,0.0,sec_before,sec_now);
  draw_quad(1,3,0.1,sec_before,sec_now);
  draw_quad(3,2,0.1,sec_before,sec_now);
  draw_quad(2,0,0.1,sec_before,sec_now);

  
  //left higher rail
  draw_quad(4,6,0.2,sec_before,sec_now);
  draw_quad(6,7,0.3,sec_before,sec_now);
  draw_quad(7,5,0.2,sec_before,sec_now);
  draw_quad(5,4,0.1,sec_before,sec_now);







  //right lower rail
  draw_quad(8,9,0.0,sec_before,sec_now);
  draw_quad(9,11,0.1,sec_before,sec_now);
  draw_quad(11,10,0.1,sec_before,sec_now);
  draw_quad(10,8,0.1,sec_before,sec_now);
  //right higher rail
  draw_quad(12,14,0.2,sec_before,sec_now);
  draw_quad(14,15,0.3,sec_before,sec_now);
  draw_quad(15,13,0.2,sec_before,sec_now);
  draw_quad(13,12,0.1,sec_before,sec_now);

  for (int m=0;m<8;m++)
  {glBegin(GL_LINE_STRIP);
  glColor3f(0.0,0.0,0.0);
  glVertex3f(sec_before.railpoint[m].x,sec_before.railpoint[m].y,sec_before.railpoint[m].z);

  glColor3f(0.0,0.0,0.0);
  glVertex3f(sec_now.railpoint[m].x,sec_now.railpoint[m].y,sec_now.railpoint[m].z);
  
  glEnd();
  }//underline for left rail, could be painted with different color to distinguish the two rails

  for (int m=8;m<16;m++)
  {glBegin(GL_LINE_STRIP);
  glColor3f(0.0,0.0,0.0);
  glVertex3f(sec_before.railpoint[m].x,sec_before.railpoint[m].y,sec_before.railpoint[m].z);

  glColor3f(0.0,0.0,0.0);
  glVertex3f(sec_now.railpoint[m].x,sec_now.railpoint[m].y,sec_now.railpoint[m].z);
  
  glEnd();}
}//underline for right rail, could be painted with different color to distinguish the two rails


double dist(point u0,point u1)
{
  return sqrt(pow((u0.x-u1.x),2)+pow((u0.y-u1.y),2)+pow((u0.z-u1.z),2));

}

point subdivide_point(double u0,double u1,double maxline_length,point p1,point p2,point p3,point p4,point b_start)
{
  point pos_0,pos_1,b_end;
  pos_0=drawsplinepoints(s_of_catmull,u0,p1,p2,p3,p4);
  pos_1=drawsplinepoints(s_of_catmull,u1,p1,p2,p3,p4);
  if (dist(pos_0,pos_1)>maxline_length)
  {
    double umid=u0+0.5*(u1-u0);
  
    b_end=subdivide_point(u0,umid,maxline_length,p1,p2,p3,p4,b_start);
    b_end=subdivide_point(umid,u1,maxline_length,p1,p2,p3,p4,b_end);
  }
  else
  {
    // glBegin(GL_LINE_STRIP);
    // glColor3f(0.0,0.0,1.0);
    // glVertex3f(pos_0.x,pos_0.y,pos_0.z);
    // glVertex3f(pos_1.x,pos_1.y,pos_1.z);
    // glEnd();


    cross_sec sec_0,sec_1;

    sec_0.pos=pos_0;
    sec_1.pos=pos_1;

    sec_0.tan=tanVec(s_of_catmull,u0,p1,p2,p3,p4);
    sec_1.tan=tanVec(s_of_catmull,u1,p1,p2,p3,p4);

    //sec_0.binorm=b_start;
    sec_0.norm=norm_matmul(b_start,sec_0.tan);
    sec_0.binorm=norm_matmul(sec_0.tan,sec_0.norm);

    sec_1.norm=norm_matmul(sec_0.binorm,sec_1.tan);
    sec_1.binorm=norm_matmul(sec_1.tan,sec_1.norm);

    for (int m=0;m<16;m++)
    {
      sec_0.railpoint[m].x=sec_0.pos.x+(sec_coef[m][0]*sec_0.binorm.x)+(sec_coef[m][1]*sec_0.norm.x);
      sec_0.railpoint[m].y=sec_0.pos.y+(sec_coef[m][0]*sec_0.binorm.y)+(sec_coef[m][1]*sec_0.norm.y);
      sec_0.railpoint[m].z=sec_0.pos.z+(sec_coef[m][0]*sec_0.binorm.z)+(sec_coef[m][1]*sec_0.norm.z);
                  
      sec_1.railpoint[m].x=sec_1.pos.x+(sec_coef[m][0]*sec_1.binorm.x)+(sec_coef[m][1]*sec_1.norm.x);
      sec_1.railpoint[m].y=sec_1.pos.y+(sec_coef[m][0]*sec_1.binorm.y)+(sec_coef[m][1]*sec_1.norm.y);
      sec_1.railpoint[m].z=sec_1.pos.z+(sec_coef[m][0]*sec_1.binorm.z)+(sec_coef[m][1]*sec_1.norm.z);
    }

    connect_sec(sec_0,sec_1);
    if (u0==0.1||u0==0.2||u0==0.3||u0==0.4||u0==0.5||u0==0.6||u0==0.7||u0==0.8||u0==0.9||u0==0.0)
    {
        glEnable(GL_TEXTURE_2D);
	      
	      glBindTexture(GL_TEXTURE_2D, 6);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        
        glBegin(GL_QUADS);
        glTexCoord2f(1.0, 0.0);//4
        glVertex3f(sec_0.railpoint[10].x-0.05*sec_0.tan.x,sec_0.railpoint[10].y-0.05*sec_0.tan.y,sec_0.railpoint[10].z-0.05*sec_0.tan.z);
        
        glTexCoord2f(0.0, 0.0);//2
        glVertex3f(sec_0.railpoint[3].x-0.05*sec_0.tan.x,sec_0.railpoint[3].y-0.05*sec_0.tan.y,sec_0.railpoint[3].z-0.05*sec_0.tan.z);
        
        glTexCoord2f(0.0, 1.0);//1
        glVertex3f(sec_0.railpoint[3].x+0.05*sec_0.tan.x,sec_0.railpoint[3].y+0.05*sec_0.tan.y,sec_0.railpoint[3].z+0.05*sec_0.tan.z);


        glTexCoord2f(1.0, 1.0);//3
        glVertex3f(sec_0.railpoint[10].x+0.05*sec_0.tan.x,sec_0.railpoint[10].y+0.05*sec_0.tan.y,sec_0.railpoint[10].z+0.05*sec_0.tan.z);

        
        glEnd();
        
      
      
      
      

      

      glBegin(GL_QUADS);
      //glColor3f(0.4, 0.3, 0.2);

      glTexCoord2f(0.0, 1.0);
      glVertex3f(sec_0.railpoint[1].x+0.05*sec_0.tan.x,sec_0.railpoint[1].y+0.05*sec_0.tan.y,sec_0.railpoint[1].z+0.05*sec_0.tan.z);
      glTexCoord2f(0.0, 0.0);
      glVertex3f(sec_0.railpoint[1].x-0.05*sec_0.tan.x,sec_0.railpoint[1].y-0.05*sec_0.tan.y,sec_0.railpoint[1].z-0.05*sec_0.tan.z);
      glTexCoord2f(1.0, 1.0);
      glVertex3f(sec_0.railpoint[8].x+0.05*sec_0.tan.x,sec_0.railpoint[8].y+0.05*sec_0.tan.y,sec_0.railpoint[8].z+0.05*sec_0.tan.z);
      glTexCoord2f(1.0, 0.0);
      glVertex3f(sec_0.railpoint[8].x-0.05*sec_0.tan.x,sec_0.railpoint[8].y-0.05*sec_0.tan.y,sec_0.railpoint[8].z-0.05*sec_0.tan.z);

      glEnd();


      glBegin(GL_QUADS);
      //glColor3f(0.4, 0.3, 0.2);
      glTexCoord2f(0.0, 1.0);
      glVertex3f(sec_0.railpoint[3].x+0.05*sec_0.tan.x,sec_0.railpoint[3].y+0.05*sec_0.tan.y,sec_0.railpoint[3].z+0.05*sec_0.tan.z);
      glTexCoord2f(0.0, 0.0);
      glVertex3f(sec_0.railpoint[1].x+0.05*sec_0.tan.x,sec_0.railpoint[1].y+0.05*sec_0.tan.y,sec_0.railpoint[1].z+0.05*sec_0.tan.z);
      glTexCoord2f(1.0, 1.0);
      glVertex3f(sec_0.railpoint[10].x+0.05*sec_0.tan.x,sec_0.railpoint[10].y+0.05*sec_0.tan.y,sec_0.railpoint[10].z+0.05*sec_0.tan.z);
      glTexCoord2f(1.0, 0.0);
      glVertex3f(sec_0.railpoint[8].x+0.05*sec_0.tan.x,sec_0.railpoint[8].y+0.05*sec_0.tan.y,sec_0.railpoint[8].z+0.05*sec_0.tan.z);

      glEnd();

      glBegin(GL_QUADS);
      //glColor3f(0.4, 0.3, 0.2);
      glTexCoord2f(0.0, 1.0);
      glVertex3f(sec_0.railpoint[3].x-0.05*sec_0.tan.x,sec_0.railpoint[3].y-0.05*sec_0.tan.y,sec_0.railpoint[3].z-0.05*sec_0.tan.z);
      glTexCoord2f(0.0, 0.0);
      glVertex3f(sec_0.railpoint[1].x-0.05*sec_0.tan.x,sec_0.railpoint[1].y-0.05*sec_0.tan.y,sec_0.railpoint[1].z-0.05*sec_0.tan.z);
      glTexCoord2f(1.0, 1.0);
      glVertex3f(sec_0.railpoint[10].x-0.05*sec_0.tan.x,sec_0.railpoint[10].y-0.05*sec_0.tan.y,sec_0.railpoint[10].z-0.05*sec_0.tan.z);
      glTexCoord2f(1.0, 0.0);
      glVertex3f(sec_0.railpoint[8].x-0.05*sec_0.tan.x,sec_0.railpoint[8].y-0.05*sec_0.tan.y,sec_0.railpoint[8].z-0.05*sec_0.tan.z);

      glEnd();

      glDisable(GL_TEXTURE_2D);




      //for lines that marks the support wood bar clearer
      
      // glBegin(GL_LINE_STRIP);
      // glColor3f(0.0, 0.0, 0.0);
      // glVertex3f(sec_0.railpoint[3].x+0.05*sec_0.tan.x,sec_0.railpoint[3].y+0.05*sec_0.tan.y,sec_0.railpoint[3].z+0.05*sec_0.tan.z);
      // glVertex3f(sec_0.railpoint[10].x+0.05*sec_0.tan.x,sec_0.railpoint[10].y+0.05*sec_0.tan.y,sec_0.railpoint[10].z+0.05*sec_0.tan.z);
      // glEnd();


      

      // glBegin(GL_LINE_STRIP);
      // glColor3f(0.0, 0.0, 0.0);
      // glVertex3f(sec_0.railpoint[3].x-0.05*sec_0.tan.x,sec_0.railpoint[3].y-0.05*sec_0.tan.y,sec_0.railpoint[3].z-0.05*sec_0.tan.z);
      // glVertex3f(sec_0.railpoint[10].x-0.05*sec_0.tan.x,sec_0.railpoint[10].y-0.05*sec_0.tan.y,sec_0.railpoint[10].z-0.05*sec_0.tan.z);
      // glEnd();
      // glBegin(GL_LINE_STRIP);
      // glColor3f(0.0, 0.0, 0.0);
      // glVertex3f(sec_0.railpoint[1].x-0.05*sec_0.tan.x,sec_0.railpoint[1].y-0.05*sec_0.tan.y,sec_0.railpoint[1].z-0.05*sec_0.tan.z);
      // glVertex3f(sec_0.railpoint[8].x-0.05*sec_0.tan.x,sec_0.railpoint[8].y-0.05*sec_0.tan.y,sec_0.railpoint[8].z-0.05*sec_0.tan.z);
      // glEnd();






    }
    return sec_1.binorm;
  }
}





/////// display function////////
void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity(); // reset transformation

  

  glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);
	glRotatef(g_vLandRotate[0], 1, 0, 0);
	glRotatef(g_vLandRotate[1], 0, 1, 0);
	glRotatef(g_vLandRotate[2], 0, 0, 1);
	glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);
  


  point p1,p2,p3,p4,spline_point,last_b;
  cross_sec sec_before;
  cross_sec sec_now;

  

  

  //point * pointstatus;

  

  if (display_mode==1)
  {
    update_cam_one_way();
  }


    
    //// This is for overlook the big picture///////////////
   
      glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    gluPerspective(60, 1.0*640/480, .01, 1000);
      gluLookAt(
        0.0,0.0,2.0,
        0.0,0.0,0.0,
        0.0,1.0,0.0
        );
      
      
 

      
    if(display_mode!=0)//rail will be drawn in subdivision fashion
    {
      for (int i=0;i<g_iNumOfSplines;i++)
      {
        for (int j=0;j<g_Splines[i].numControlPoints-3;j++)
        {

          

          p1=g_Splines[i].points[j];
          p2=g_Splines[i].points[j+1];
          p3=g_Splines[i].points[j+2];
          p4=g_Splines[i].points[j+3];


          if (j==0)
          {
            point startV;
            startV.x=0;
            startV.y=1/sqrt(1+0.25);
            startV.z=0.5/sqrt(1+0.25);


            last_b=subdivide_point(0,1,max_line_length,p1,p2,p3,p4,startV);
          }
          else
          {
            last_b=subdivide_point(0,1,max_line_length,p1,p2,p3,p4,last_b);
          }


        }

      }



    }
    else//rail will be drawn in brute force fashion
    { 
      
      /*normally drawing splines*/
      glBegin(GL_LINE_STRIP);
      for (int i=0;i<g_iNumOfSplines;i++)
      {
        // point start_point,start_tan,start_v,start_b;
        // start_point=drawsplinepoints(s_of_catmull,0.0,g_Splines[i].points[0],g_Splines[i].points[1],g_Splines[i].points[2],g_Splines[i].points[4]);
          for (int j=0;j<g_Splines[i].numControlPoints-3;j++)
          {
            p1= g_Splines[i].points[j];
            p2=g_Splines[i].points[j+1];
            p3=g_Splines[i].points[j+2];
            p4=g_Splines[i].points[j+3];
            for (double u=0.0;u<1.0;u+=bruteforce_u_stepsize)
            {
              spline_point=drawsplinepoints(s_of_catmull,u,p1,p2,p3,p4);
              glColor3f(0.0,1.0,1.0);
              glVertex3f(spline_point.x,spline_point.y,spline_point.z);
            }
          }
        }
      glEnd();



      ///draw rails///
      for (int i=0;i<g_iNumOfSplines;i++)
      {
        // cross_sec sec_before;
        // cross_sec sec_now;

          for (int j=0;j<g_Splines[i].numControlPoints-3;j++)
          {
            
            //cross_sec sec_now;

            p1=g_Splines[i].points[j];
            p2=g_Splines[i].points[j+1];
            p3=g_Splines[i].points[j+2];
            p4=g_Splines[i].points[j+3];

            

            for (double u=0.0;u<1.0;u+=bruteforce_u_stepsize)
            {
              sec_now.pos=drawsplinepoints(s_of_catmull,u,p1,p2,p3,p4);
              sec_now.tan=tanVec(s_of_catmull,u,p1,p2,p3,p4);
              //cout<<"sec_pos="<<" "<<sec_now.tan.x<<" "<<sec_now.tan.y<<" "<<sec_now.tan.z<<endl;


              if (u==0.0 && j==0)
              {
                point startV;
                startV.x=0;
                startV.y=-1;
                startV.z=-0.5;

                sec_now.norm=norm_matmul(sec_now.tan,startV);
                sec_now.binorm=norm_matmul(sec_now.tan,sec_now.norm);
                // cout<<"sec_norm="<<" "<<sec_now.norm.x<<" "<<sec_now.norm.y<<" "<<sec_now.norm.z<<endl;
                // cout<<"sec_binorm="<<" "<<sec_now.binorm.x<<" "<<sec_now.binorm.y<<" "<<sec_now.binorm.z<<endl;
                // cout<<"sec_tan="<<" "<<sec_now.tan.x<<" "<<sec_now.tan.y<<" "<<sec_now.tan.z<<endl;

                for (int m=0;m<16;m++)
                {
                  
                  sec_now.railpoint[m].x=sec_now.pos.x+(sec_coef[m][0]*sec_now.binorm.x)+(sec_coef[m][1]*sec_now.norm.x);
                  sec_now.railpoint[m].y=sec_now.pos.y+(sec_coef[m][0]*sec_now.binorm.y)+(sec_coef[m][1]*sec_now.norm.y);
                  sec_now.railpoint[m].z=sec_now.pos.z+(sec_coef[m][0]*sec_now.binorm.z)+(sec_coef[m][1]*sec_now.norm.z);
                 

                  //cout<<"sec_0_railpoint_value"<<m<<" "<<sec_now.railpoint[m].x<<" "<<sec_now.railpoint[m].y<<" "<<sec_now.railpoint[m].z<<endl;

                }

                sec_before=sec_now;
                connect_sec(sec_before,sec_now);

                // draw_quad(0,1,sec_before,sec_now);
                // draw_quad(0,2,sec_before,sec_now);
                // draw_quad(1,3,sec_before,sec_now);
                // draw_quad(2,3,sec_before,sec_now);
                // draw_quad(4,6,sec_before,sec_now);
                // draw_quad(3,7,sec_before,sec_now);
                // draw_quad(6,7,sec_before,sec_now);

                // draw_quad(8,9,sec_before,sec_now);
                // draw_quad(8,10,sec_before,sec_now);
                // draw_quad(9,11,sec_before,sec_now);
                // draw_quad(10,11,sec_before,sec_now);
                // draw_quad(12,14,sec_before,sec_now);
                // draw_quad(13,15,sec_before,sec_now);
                // draw_quad(14,15,sec_before,sec_now);


              }
              else
              {
                                
                sec_now.norm=norm_matmul(sec_before.binorm,sec_now.tan);
                sec_now.binorm=norm_matmul(sec_now.tan,sec_now.norm);

                for (int m=0;m<16;m++)
                {
                  sec_now.railpoint[m].x=sec_now.pos.x+(sec_coef[m][0]*sec_now.binorm.x)+(sec_coef[m][1]*sec_now.norm.x);
                  sec_now.railpoint[m].y=sec_now.pos.y+(sec_coef[m][0]*sec_now.binorm.y)+(sec_coef[m][1]*sec_now.norm.y);
                  sec_now.railpoint[m].z=sec_now.pos.z+(sec_coef[m][0]*sec_now.binorm.z)+(sec_coef[m][1]*sec_now.norm.z);

                }
                connect_sec(sec_before,sec_now);
                sec_before=sec_now;
                //cout<<sec_before.pos.x<<" "<<sec_before.pos.y<<" "<<sec_before.pos.z<<" "<<sec_before.pos.x<<" "<<sec_before.pos.y<<" "<<sec_before.pos.z<<" "<<endl;

              }
            }
          }
      }
    }
      




  
  
  
  //////////////// texture mapping for the skybox//////////////////////////////////////////////////////
  
  glEnable(GL_TEXTURE_2D);
	///////for the sky/////
	glBindTexture(GL_TEXTURE_2D, 0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_POLYGON);
	
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-border, border, border);
	
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-border, border, -border);
	
	glTexCoord2f(0.0, 1.0);
	glVertex3f(border, border, -border);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(border, border, border);
	
	glEnd();
  



  /////////for the land//////////
 
  glBindTexture(GL_TEXTURE_2D, 5);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBegin(GL_POLYGON);



	// glTexCoord2f(1.0, 0.0);
	// glVertex3f(-border, -border, border);

	// glTexCoord2f(0.0, 0.0);
	// glVertex3f(-border, -border, -border);

	// glTexCoord2f(0.0, 1.0);
	// glVertex3f(border, -border, -border);

	// glTexCoord2f(1.0, 1.0);
	// glVertex3f(border, -border, border);


  glTexCoord2f(1.0, 0.0);
	glVertex3f(border, -border, border);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-border, -border, border);

	glTexCoord2f(0.0, 1.0);
	glVertex3f(-border, -border, -border);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(border, -border, -border);
	glEnd();
  


  ///for the walls around///
  
  glBindTexture(GL_TEXTURE_2D, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBegin(GL_POLYGON);




	glTexCoord2f(1.0, 0.0);
	glVertex3f(-border, border, border);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-border, border, -border);

	glTexCoord2f(0.0, 1.0);
	glVertex3f(-border, -border, -border);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(-border, -border, border);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 2);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBegin(GL_POLYGON);



	glTexCoord2f(1.0, 0.0);
	glVertex3f(border, border, border);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-border, border, border);

	glTexCoord2f(0.0, 1.0);
	glVertex3f(-border, -border, border);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(border, -border, border);

	glEnd();

	glBindTexture(GL_TEXTURE_2D, 3);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBegin(GL_POLYGON);



	glTexCoord2f(1.0, 0.0);
	glVertex3f(border, border, -border);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(border, border, border);

	glTexCoord2f(0.0, 1.0);
	glVertex3f(border, -border, border);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(border, -border, -border);
	glEnd();


	glBindTexture(GL_TEXTURE_2D, 4);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBegin(GL_POLYGON);



	glTexCoord2f(1.0, 0.0);
	glVertex3f(-border, border, -border);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(border, border, -border);

	glTexCoord2f(0.0, 1.0);
	glVertex3f(border, -border, -border);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(-border, -border, -border);
	glEnd();

	glDisable(GL_TEXTURE_2D);
  
  
  
  glPopMatrix();
  glutSwapBuffers();
}
//////////////////////

int main (int argc, char ** argv)
{
  if (argc<2)
  {  
  printf ("usage: %s <trackfile>\n", argv[0]);
  exit(0);
  }

  loadSplines(argv[1]);


  ////for intialization , copied from assign1////////
  glutInit(&argc,argv);
  //reset double buffer
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
  //glLoadIdentity();

  // window initialization
  glutInitWindowSize(640,480);
  glutInitWindowPosition(0,0);
  glutCreateWindow("test");
  

  /* tells glut to use a particular display function to redraw */


  glutDisplayFunc(display);
  //glutDisplayFunc(display_strips);
  

  /* allow the user to quit using the right mouse button menu */
  g_iMenuId = glutCreateMenu(menufunc);
  glutSetMenu(g_iMenuId);
  glutAddMenuEntry("Quit",0);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  
  /* replace with any animate code */
  glutIdleFunc(doIdle);
  //cout<<sec_coef[1][0]<<" "<<sec_coef[1][1]<<endl;

  /* callback for mouse drags */
  glutMotionFunc(mousedrag);
  /* callback for idle mouse movement */
  glutPassiveMotionFunc(mouseidle);

  glutKeyboardFunc(keyboard);

  /* callback for mouse button changes */
  glutMouseFunc(mousebutton);

  /* do initialization */
  myinit();

  glutMainLoop();
  ////////reuse assign1////////////







  
  return 0;
}
