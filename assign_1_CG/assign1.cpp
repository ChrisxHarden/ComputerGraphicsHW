/*
  CSCI 420 Computer Graphics
  Assignment 1: Height Fields
  <Enyu Zhao>
*/

#define GL_SILENCE_DEPRECATION
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>

#include <typeinfo>
#include <iostream>

using namespace std;

int g_iMenuId;

int g_vMousePos[2] = {0, 0};

int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

int display_mode=0;// defines display_mode, 0 for points, 1 for wireframes , 2 for solid triangles， 3 for 




typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;

CONTROLSTATE g_ControlState = ROTATE;


/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

/* see <your pic directory>/pic.h for type Pic */
Pic * g_pHeightData;

/* Write a screenshot to the specified filename */
void saveScreenshot (char *filename)
{
  int i, j;
  Pic *in = NULL;

  if (filename == NULL)
    return;

  /* Allocate a picture buffer */
  in = pic_alloc(640, 480, 3, NULL);

  printf("File to save to: %s\n", filename);

  for (i=479; i>=0; i--) {
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                 &in->pix[i*in->nx*in->bpp]);
  }

  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);
}

void myinit()
{
  /* setup gl view here */
  glClearColor(0.0, 0.0, 0.0, 0.0);
  // enable depth buffering
  glLoadIdentity();
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH); 

}

void display()
{
  
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

glMatrixMode(GL_PROJECTION);
glLoadIdentity();
gluPerspective(30,1.0*640/480,0.01,10.0);
gluLookAt(
  0.0,0.0,2.0,
  0.0,0.0,0.0,
  0.0,1.0,0.0
);

glMatrixMode(GL_MODELVIEW);
glLoadIdentity(); // reset transformation


  
  
glRotatef(g_vLandRotate[0],1.0,0.0,0.0);
glRotatef(g_vLandRotate[1],0.0,1.0,0.0);
glRotatef(g_vLandRotate[2],0.0,0.0,1.0);// Read the movement of mouse and rotate.
  
  
glTranslatef(g_vLandTranslate[0],g_vLandTranslate[1],g_vLandTranslate[2]);// Read the movement of mouse and translate.
    
  
glScalef(g_vLandScale[0],g_vLandScale[1],g_vLandScale[2]);// Read the movement of mouse and scale.
    

glScalef(1,1,0.5);//clip the z-axis value to fully demonstrate the landscape


int nx=g_pHeightData->nx;// use nx and ny so that the programme can deal with different size of photo
int ny=g_pHeightData->ny;
int bpp=g_pHeightData->bpp;

if (bpp==1)
{
switch(display_mode)
{
  case 1://for wireframe
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  

    
    for(int i=0;i<ny-1;i++)
    {
  
          glBegin(GL_TRIANGLE_STRIP);// at each row restart the drawing process to avoid connecting the last point on the first row to the first point on the second row
          
          for(int j=0;j<nx;j++)
          {
    
            glColor3f(0.2,(g_pHeightData->pix[j*ny+i])/511.0+0.2,(g_pHeightData->pix[j*ny+i])/511.0+0.2);// color for wireframe set to be cyan-blue, and will change the hue according to height

            glVertex3f((float(j)-nx/2)/float(nx),(float(i)-ny/2)/float(ny),(g_pHeightData->pix[j*ny+i])/255.0);//point [j,i]

            glColor3f(0.2,(g_pHeightData->pix[j*ny+i+1])/511.0+0.2,(g_pHeightData->pix[j*ny+i+1])/511.0+0.2);

            glVertex3f((float(j)-nx/2)/float(nx),(float(i+1)-ny/2)/float(ny),(g_pHeightData->pix[j*ny+i+1])/255.0);//point [j,i+1]

          }
          glEnd();
                   

          
    }
    
    break;
  }
  case 2://for solid triangle
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    
    for(int i=0;i<ny-1;i++)
    {
  
          glBegin(GL_TRIANGLE_STRIP);// at each row restart the drawing process to avoid connecting the last point on the first row to the first point on the second row
          
          for(int j=0;j<nx;j++)
          {
    
            glColor3f((g_pHeightData->pix[j*ny+i])/511.0+0.2,(g_pHeightData->pix[j*ny+i])/511.0+0.2,(g_pHeightData->pix[j*ny+i])/511.0+0.2);//color for solid triangle set to be black to white and will change according to height

            glVertex3f((float(j)-nx/2)/float(nx),(float(i)-ny/2)/float(ny),(g_pHeightData->pix[j*ny+i])/255.0);

            glColor3f(g_pHeightData->pix[j*ny+i+1]/511.0+0.2,g_pHeightData->pix[j*ny+i+1]/511.0+0.2,(g_pHeightData->pix[j*ny+i+1])/511.0+0.2);

            glVertex3f((float(j)-nx/2)/float(nx),(float(i+1)-ny/2)/float(ny),(g_pHeightData->pix[j*ny+i+1])/255.0);

          }
          glEnd();




          
    }
    
    break;
  }

  case 3:
  {
       
    for(int i=0;i<ny-1;i++)
    {
  
          glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
          glBegin(GL_TRIANGLE_STRIP);// at each row restart the drawing process to avoid connecting the last point on the first row to the first point on the second row
          
          for(int j=0;j<nx;j++)
          {
    
            glColor3f((g_pHeightData->pix[j*ny+i])/511.0+0.2,(g_pHeightData->pix[j*ny+i])/511.0+0.2,(g_pHeightData->pix[j*ny+i])/511.0+0.2);//color for solid triangle set to be black to white and will change according to height

            glVertex3f((float(j)-nx/2)/float(nx),(float(i)-ny/2)/float(ny),(g_pHeightData->pix[j*ny+i])/255.0);

            glColor3f(g_pHeightData->pix[j*ny+i+1]/511.0+0.2,g_pHeightData->pix[j*ny+i+1]/511.0+0.2,(g_pHeightData->pix[j*ny+i+1])/511.0+0.2);

            glVertex3f((float(j)-nx/2)/float(nx),(float(i+1)-ny/2)/float(ny),(g_pHeightData->pix[j*ny+i+1])/255.0);

          }
          glEnd();



          glPolygonOffset(1.0,10.0);
          
          
          glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

          glBegin(GL_TRIANGLE_STRIP);// at each row restart the drawing process to avoid connecting the last point on the first row to the first point on the second row
          
          for(int j=0;j<nx;j++)
          {
    
            glColor3f(0.2,(g_pHeightData->pix[j*ny+i])/511.0+0.2,(g_pHeightData->pix[j*ny+i])/511.0+0.2);// color for wireframe set to be cyan-blue, and will change the hue according to height

            //glColor3f(0.0,0.0,0.0);
            glVertex3f((float(j)-nx/2)/float(nx),(float(i)-ny/2)/float(ny),(g_pHeightData->pix[j*ny+i])/255.0);//point [j,i]

            glColor3f(0.2,(g_pHeightData->pix[j*ny+i+1])/511.0+0.2,(g_pHeightData->pix[j*ny+i+1])/511.0+0.2);
            //glColor3f(0.0,0.0,0.0);
            glVertex3f((float(j)-nx/2)/float(nx),(float(i+1)-ny/2)/float(ny),(g_pHeightData->pix[j*ny+i+1])/255.0);//point [j,i+1]

          }
          glEnd();


    }
    
    break;
  }



  default:// default to be point mode, if want to get back to point display mode , press 1
  { 
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    
    glBegin(GL_POINTS);
    for(int i=0;i<ny-1;i++)
    {
        for(int j=0;j<nx;j++)
        {
    
          glColor3f(0.0,0.0,(g_pHeightData->pix[j*nx+i])/511.0+0.2);//color for points sets to be blue and will change according to height
          glVertex3f((float(j)-nx/2)/float(nx),(float(i)-ny/2)/float(ny),(g_pHeightData->pix[j*ny+i])/255.0);
        }
    }
    glEnd();
  }
}
}//for grayscale pictures




else
{
switch(display_mode)
{
  case 1://for wireframe
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  

    
    for(int i=0;i<ny-1;i++)
    {
  
          glBegin(GL_TRIANGLE_STRIP);// at each row restart the drawing process to avoid connecting the last point on the first row to the first point on the second row
          
          for(int j=0;j<nx;j++)
          {
    
            glColor3f((g_pHeightData->pix[bpp*(j*ny+i)])/255.0,(g_pHeightData->pix[bpp*(j*ny+i)+1])/255.0,(g_pHeightData->pix[bpp*(j*ny+i)+2])/255.0);// color for wireframe set to be cyan-blue, and will change the hue according to height

            glVertex3f((float(j)-nx/2)/float(nx),(float(i)-ny/2)/float(ny),(g_pHeightData->pix[bpp*(j*ny+i)])/255.0);//point [j,i]

            glColor3f((g_pHeightData->pix[bpp*(j*ny+i+1)])/255.0,(g_pHeightData->pix[bpp*(j*ny+i+1)+1])/255.0,(g_pHeightData->pix[bpp*(j*ny+i+1)+2])/255.0);

            glVertex3f((float(j)-nx/2)/float(nx),(float(i+1)-ny/2)/float(ny),(g_pHeightData->pix[bpp*(j*ny+i+1)])/255.0);//point [j,i+1]

          }
          glEnd();
                   

          
    }
    
    break;
  }
  case 2://for solid triangle
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    
    for(int i=0;i<ny-1;i++)
    {
  
          glBegin(GL_TRIANGLE_STRIP);// at each row restart the drawing process to avoid connecting the last point on the first row to the first point on the second row
          
          for(int j=0;j<nx;j++)
          {
    
           glColor3f((g_pHeightData->pix[bpp*(j*ny+i)])/255.0,(g_pHeightData->pix[bpp*(j*ny+i)+1])/255.0,(g_pHeightData->pix[bpp*(j*ny+i)+2])/255.0);// color for wireframe set to be cyan-blue, and will change the hue according to height

            glVertex3f((float(j)-nx/2)/float(nx),(float(i)-ny/2)/float(ny),(g_pHeightData->pix[bpp*(j*ny+i)])/255.0);//point [j,i]

            glColor3f((g_pHeightData->pix[bpp*(j*ny+i+1)])/255.0,(g_pHeightData->pix[bpp*(j*ny+i+1)+1])/255.0,(g_pHeightData->pix[bpp*(j*ny+i+1)+2])/255.0);

            glVertex3f((float(j)-nx/2)/float(nx),(float(i+1)-ny/2)/float(ny),(g_pHeightData->pix[bpp*(j*ny+i+1)])/255.0);//point [j,i+1]

          }
          glEnd();




          
    }
    
    break;
  }

  case 3:
  {
       
    for(int i=0;i<ny-1;i++)
    {
  
          glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
          glBegin(GL_TRIANGLE_STRIP);// at each row restart the drawing process to avoid connecting the last point on the first row to the first point on the second row
          
          for(int j=0;j<nx;j++)
          {
    
            glColor3f((g_pHeightData->pix[bpp*(j*ny+i)])/255.0,(g_pHeightData->pix[bpp*(j*ny+i)+1])/255.0,(g_pHeightData->pix[bpp*(j*ny+i)+2])/255.0);// color for wireframe set to be cyan-blue, and will change the hue according to height

            glVertex3f((float(j)-nx/2)/float(nx),(float(i)-ny/2)/float(ny),(g_pHeightData->pix[bpp*(j*ny+i)])/255.0);//point [j,i]

            glColor3f((g_pHeightData->pix[bpp*(j*ny+i+1)])/255.0,(g_pHeightData->pix[bpp*(j*ny+i+1)+1])/255.0,(g_pHeightData->pix[bpp*(j*ny+i+1)+2])/255.0);

            glVertex3f((float(j)-nx/2)/float(nx),(float(i+1)-ny/2)/float(ny),(g_pHeightData->pix[bpp*(j*ny+i+1)])/255.0);//point [j,i+1]


          }
          glEnd();



          glPolygonOffset(1.0,10.0);
          
          
          glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

          glBegin(GL_TRIANGLE_STRIP);// at each row restart the drawing process to avoid connecting the last point on the first row to the first point on the second row
          
          for(int j=0;j<nx;j++)
          {
    
            glColor3f(0.0,0.0,0.0);// color for wireframe set to be cyan-blue, and will change the hue according to height

            glVertex3f((float(j)-nx/2)/float(nx),(float(i)-ny/2)/float(ny),(g_pHeightData->pix[bpp*(j*ny+i)])/255.0);//point [j,i]

            glColor3f(0.0,0.0,0.0);

            glVertex3f((float(j)-nx/2)/float(nx),(float(i+1)-ny/2)/float(ny),(g_pHeightData->pix[bpp*(j*ny+i+1)])/255.0);//point [j,i+1]

          }
          glEnd();


    }
    
    break;
  }



  default:// default to be point mode, if want to get back to point display mode , press 1
  { 
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    
    glBegin(GL_POINTS);
    for(int i=0;i<ny-1;i++)
    {
        for(int j=0;j<nx;j++)
        {
    
          glColor3f((g_pHeightData->pix[bpp*(j*ny+i)])/255.0,(g_pHeightData->pix[bpp*(j*ny+i)+1])/255.0,(g_pHeightData->pix[bpp*(j*nx+i)+2])/255.0);//color for points sets to be blue and will change according to height
          glVertex3f((float(j)-nx/2)/float(nx),(float(i)-ny/2)/float(ny),g_pHeightData->pix[bpp*(j*ny+i)]/255.0);
        }
    }
    glEnd();
  }
}
}



glutSwapBuffers();
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

int main (int argc, char ** argv)
{
  
  if (argc<2)
  {  
    printf ("usage: %s heightfield.jpg\n", argv[0]);
    exit(1);
  }
  

  g_pHeightData = jpeg_read(argv[1], NULL);
  

  if (!g_pHeightData)
  {
    printf ("error reading %s.\n", argv[1]);
    exit(1);
  }
  cout<<g_pHeightData->bpp<<endl;

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
  return(0);
}
