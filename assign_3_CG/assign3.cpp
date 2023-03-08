/*
CSCI 420
Assignment 3 Raytracer

Name: Enyu Zhao
*/






#include <stdio.h>
#include <limits>
#include <string.h>
#include <stdlib.h>
#include "pic.h"
#include <iostream>
#include <random>
#include <math.h>
#include <algorithm>
#include <chrono>
#include <cfloat>

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>



#define EPSILON 1e-5
#define PI 3.14159
#define GL_SILENCE_DEPRECATION



#define MAX_TRIANGLES 2000
#define MAX_SPHERES 100
#define MAX_LIGHTS 100
#define SUBLIGHTS 20
#define RECUR_THRESHOLD 3

char * filename = NULL;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2

int mode = MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480
//the field of view of the camera
#define fov 60.0


bool recursive_ref=false;
double ref_ratio=0.1;
bool anti_aliasing=true;
bool soft_shadow=true;




using namespace std;

unsigned char buffer[HEIGHT][WIDTH][3];
double cell_width, cell_height;
double x_min, y_min;

struct Ver {
    double x, y, z;                  // position, also color (r,g,b)
    Ver(double x_=0, double y_=0, double z_=0){ x=x_; y=y_; z=z_; }
    Ver operator+(const Ver &b) const { return Ver(x+b.x,y+b.y,z+b.z); }
    Ver operator-(const Ver &b) const { return Ver(x-b.x,y-b.y,z-b.z); }
    Ver operator-() const { return Ver(-x,-y,-z); }
    Ver operator*(double b) const { return Ver(x*b,y*b,z*b); }
    Ver operator/(double b) const { return Ver(x/b,y/b,z/b); }
    Ver mult(const Ver &b) const { return Ver(x*b.x,y*b.y,z*b.z); }
    Ver& norm(){ return *this = *this * (1/sqrt(x*x+y*y+z*z)); }
    double dot(const Ver &b) const { return x*b.x+y*b.y+z*b.z; }
    double length() {return sqrt(x*x + y*y + z*z);}
    Ver cross(const Ver&b) const {return Ver(y*b.z-z*b.y,z*b.x-x*b.z,x*b.y-y*b.x);}
};

Ver Clamp(Ver &a)
{
  if (a.x > 1.0) a.x = 1.0;
  if (a.y > 1.0) a.y = 1.0;
  if (a.z > 1.0) a.z = 1.0;
}

struct Ray{
  Ver o,d;
};



struct Vertex
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double normal[3];
  double shininess;
};

struct myVertex
{
  Ver position;
  Ver color_diffuse;
  Ver color_specular;
  Ver normal;
  double shininess;
};

typedef struct _Triangle
{
  struct Vertex v[3];
} Triangle;


struct myTriangle
{
  struct myVertex v[3];
};

typedef struct _Sphere
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double shininess;
  double radius;
} Sphere;

 struct mySphere
{
  Ver position;
  Ver color_diffuse;
  Ver color_specular;
  double shininess;
  double radius;
} ;


typedef struct _Light
{
  double position[3];
  double color[3];
} Light;


struct myLight
{
  Ver position;
  Ver color;
};



Triangle triangles_[MAX_TRIANGLES];
Sphere spheres_[MAX_SPHERES];
Light lights_[MAX_LIGHTS];
double ambient_light_[3];


myTriangle triangles[MAX_TRIANGLES];
mySphere spheres[MAX_SPHERES];
myLight lights[MAX_LIGHTS];
Ver ambient_light;
Ver bg_color(1.0,1.0,1.0);

int num_triangles=0;
int num_spheres=0;
int num_lights=0;

void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);
Ver generate_ref(Ver I,Ver N);




/////////////////////////////////////Intersection/////////////////////////////////////////////////////////
bool SphereIntersect(Ray r, double &t, int &id)
{
  t = DBL_MAX;
  id = -1;
  for (int i = 0; i<num_spheres; i++)
  {
    Ver oc = Ver(r.o-spheres[i].position);
    double a = 1;
    double b = 2 * oc.dot(r.d);
    double c = oc.dot(oc) - pow(spheres[i].radius,2);





    double delta = pow(b,2) - 4*a*c;



    if (delta < 0) continue;

    double t1 = (-b + sqrt(delta))/2;
    double t2 = (-b - sqrt(delta))/2;
    double _t = min(t1,t2);


    if (_t > EPSILON && _t<t)
    {
      t = _t;
      id = i;
    }




  }

  return (id != -1);
}





bool TriangleIntersect(Ray r, double &t, int &id)
{
  t = DBL_MAX;
  id = -1;
  for (int i = 0; i<num_triangles; i++)
  {
    
    Ver ab = triangles[i].v[1].position - triangles[i].v[0].position;

    Ver ac = triangles[i].v[2].position - triangles[i].v[0].position;

    Ver s = r.o - triangles[i].v[0].position;

    Ver s1 = r.d.cross(ac);

    Ver s2 = s.cross(ab);

    double k = s1.dot(ab);






    if (k > -EPSILON && k < EPSILON) continue;


    double _t = 1/k * s2.dot(ac);
    double b1 = 1/k * s1.dot(s);
    double b2 = 1/k * s2.dot(r.d);
    double b3 = 1-b1-b2;

    if (_t < EPSILON || b1<0||b1>1||b2<0||b2>1||b3<0||b3>1) continue;
    if (_t < t)
    {
      t = _t;
      id = i;
    }
  }
  
  return (id != -1);
}



///////////////////////////////Shadow//////////////////////////////////////

bool Shadowtest(myLight light,  myVertex point)
{
  Ver dir = (light.position-point.position).norm();

  Ray shadow_test_ray;

  shadow_test_ray.o=point.position;

  shadow_test_ray.d= dir;
  
  double t_sph, t_tri; 
  int i_sph, i_tri;
  bool hit_sph,hit_tri;

  hit_sph = SphereIntersect(shadow_test_ray, t_sph, i_sph);
  hit_tri = TriangleIntersect(shadow_test_ray, t_tri,i_tri);
  Ver hitpos;


  
  if (!hit_sph && !hit_tri) return false;
  

  
  if((hit_tri && hit_sph && t_tri<t_sph)||(hit_tri&&!hit_sph))
  {
    
    hitpos=shadow_test_ray.o+shadow_test_ray.d*t_tri;
  }
  else
  {
    hitpos=shadow_test_ray.o+shadow_test_ray.d*t_sph;

  }

  double dist_light = (point.position - light.position).length();
  double dist_hit = (point.position - hitpos).length();



  if (dist_hit-dist_light<EPSILON) return true;
  
  return false;




}

Ver Shading (myVertex hitPoint, myLight light)
{
  Ver diffuse, specular, light_dir,view_dir,ref_dir;
  light_dir = (light.position-hitPoint.position).norm();

  double diff = max(light_dir.dot(hitPoint.normal),0.0);
  diffuse = light.color.mult(hitPoint.color_diffuse * diff);
  
  
  view_dir = (-hitPoint.position).norm();
  ref_dir = generate_ref(-light_dir, hitPoint.normal);

  double spec = pow(max(view_dir.dot(ref_dir),0.0), hitPoint.shininess);
  specular =  light.color.mult(hitPoint.color_specular * spec);
  
  return diffuse + specular;
}









Ver generate_ref(Ver I,Ver N)   ///make sure I and N are normalized
{
  Ver reflect_ray;

  reflect_ray=I-N*2*N.dot(I);
  return reflect_ray;
  
}



/////////////////////////lighting/////////////////////////////



Ver get_coef(int &idx, Ver pos)
{
  Ver coef;
  Ver ab = triangles[idx].v[1].position - triangles[idx].v[0].position;
  Ver ac = triangles[idx].v[2].position - triangles[idx].v[0].position;

  double ABC_Area = ab.cross(ac).length() * 0.5f;


  Ver pa = triangles[idx].v[0].position - pos;
  Ver pb = triangles[idx].v[1].position - pos;
  Ver pc = triangles[idx].v[2].position - pos;
  
  double PBC_Area = pb.cross(pc).length() * 0.5f;
  double PCA_Area = pc.cross(pa).length() * 0.5f;
  double PAB_Area = pa.cross(pb).length() * 0.5f;
  
  coef.x = PBC_Area / ABC_Area;
  coef.y = PCA_Area / ABC_Area;
  coef.z = PAB_Area / ABC_Area;

  return coef;


}


Ver interpolate(Ver A,Ver B,Ver C,Ver coef)
{
  Ver result_A,result_B,result_C;
  result_A=A*coef.x;
  result_B=B*coef.y;
  result_C=C*coef.z;

  return result_A+result_B+result_C;
}

double interpolate(double A,double B,double C,Ver coef)
{
  
  Ver result(A,B,C);
  return result.dot(coef);
}




Ver phonglighting(Ray r, int times)
{
  double t_tri,t_sph;
  int i_tri,i_sph;

  bool hit_tri,hit_sph;

  hit_tri=TriangleIntersect(r,t_tri,i_tri);
  hit_sph=SphereIntersect(r,t_sph,i_sph);




  if (!hit_sph && !hit_tri) return bg_color;
  
  myVertex hitVertex;
  
  if((hit_tri && hit_sph && t_tri<t_sph)||(hit_tri&&!hit_sph))
  {//hit triangle
   

    hitVertex.position=r.o + r.d * t_tri;
    Ver coef=get_coef(i_tri,hitVertex.position);
    myTriangle hit_triangle=triangles[i_tri];


    hitVertex.color_diffuse=interpolate(hit_triangle.v[0].color_diffuse,hit_triangle.v[1].color_diffuse,hit_triangle.v[2].color_diffuse,coef);

    hitVertex.color_specular=interpolate(hit_triangle.v[0].color_specular,hit_triangle.v[1].color_specular,hit_triangle.v[2].color_specular,coef);

    hitVertex.normal=interpolate(hit_triangle.v[0].normal,hit_triangle.v[1].normal,hit_triangle.v[2].normal,coef);
    hitVertex.normal=hitVertex.normal.norm();


    hitVertex.shininess=interpolate(hit_triangle.v[0].shininess,hit_triangle.v[1].shininess,hit_triangle.v[2].shininess,coef);

   //return Ver(1.0,0.0,0.0);


  }
  
  else
  {//hit sphere

    hitVertex.position=r.o+r.d*t_sph;
    hitVertex.color_diffuse=spheres[i_sph].color_diffuse;
    hitVertex.color_specular=spheres[i_sph].color_specular;
    hitVertex.normal=(r.o + r.d * t_sph - spheres[i_sph].position).norm();
    hitVertex.shininess=spheres[i_sph].shininess;

    //return Ver(0.0,0.0,0.0);
    
  }
  
  Ver hit_color = Ver(0,0,0);
  // for each light
  /////////////shadowtest//////////////
  for (int i = 0; i<num_lights; i++)
  {
    if (!Shadowtest(lights[i], hitVertex))
    {
      hit_color =  hit_color + Shading(hitVertex, lights[i]);
    }
  }
  
  
  if (times >= RECUR_THRESHOLD||!recursive_ref) return hit_color;
  
  times++;
  
  Ver ref_ray = generate_ref(r.d, hitVertex.normal);
  Ray reflectRay;

  reflectRay.o =hitVertex.position;
  reflectRay.d =ref_ray;

  Ver reflectColor = phonglighting(reflectRay, times);
  
  return hit_color * (1-ref_ratio) +  reflectColor * ref_ratio;
}





void init_rays_super( int row, int col, Ray (&r)[4])
{
  
  double x = x_min + (2*row+1)*cell_width/2.0f;
  double y = y_min + (2*col+1)*cell_height/2.0f;
  double z = -1;
  
  r[0].o= Ver(0,0,0);
  r[0].d= Ver(x-cell_width/4.0f,y,z).norm();

  r[1].o= Ver(0,0,0);
  r[1].d=Ver(x+cell_width/4.0f,y,z).norm();

  r[2].o= Ver(0,0,0);
  r[2].d =Ver(x,y-cell_height/4.0f,z).norm();


  r[3].o= Ver(0,0,0);
  r[3].d=Ver(x,y+cell_height/4.0f,z).norm();
}



inline double Random_norm()
{
  // unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  // std::default_random_engine gen(seed);
  // std::normal_distribution<double> dis(0,1);
  // return dis(gen);


  std::random_device dev;
  std::mt19937 gen(dev());
  std::uniform_real_distribution<double> dis(0.f, 1);
  
  return dis(gen);

  
}


void init_lights_super()
{
  if (!soft_shadow)
    return;
  
  
  int origin_light_num = num_lights;
  
  for (int i = 0; i<origin_light_num; i++)
  {
    Ver color = lights[i].color/SUBLIGHTS;
    Ver center = lights[i].position;
    
    lights[i].color = color;
    for (int j = 0; j<(SUBLIGHTS-1); j++)
    {
      lights[num_lights].color = color;
      lights[num_lights].position = Ver (center.x+Random_norm(), center.y+Random_norm(), center.z+Random_norm());
      num_lights++;
    }
  }
  return;
}


void InitScreen()
{
  double aspectRatio = (double)WIDTH/HEIGHT;
  
  float rad = fov * PI / 180;
  double screen_width = 2*aspectRatio*tan(rad/2);
  double screen_height = 2*tan(rad/2);
  cell_width =  screen_width/WIDTH;
  cell_height = screen_height/HEIGHT;
  x_min = -aspectRatio*tan(rad/2);
  y_min = -tan(rad/2);
}

//MODIFY THIS FUNCTION
void draw_scene()
{
  double aspectRatio = (double)WIDTH/HEIGHT;
  
  float rad = fov * PI / 180;
  double screen_width = 2*aspectRatio*tan(rad/2);
  double screen_height = 2*tan(rad/2);
  cell_width =  screen_width/WIDTH;
  cell_height = screen_height/HEIGHT;
  x_min = -aspectRatio*tan(rad/2);
  y_min = -tan(rad/2);
  
  for(unsigned int x=0; x<WIDTH; x++)
  {
    glPointSize(2.0);  
    glBegin(GL_POINTS);
    for(unsigned int y=0; y<HEIGHT; y++)
    {
      
      //if (soft_shadow) init_rays_super();

      if (anti_aliasing)
      {
        Ray rays[4];
        Ver color;
        init_rays_super(x,y,rays);
        for (int k = 0; k<4; k++)
        {
          color = color + phonglighting(rays[k], 0);
        }
        color = color/4;
        plot_pixel(x, y, (int)(color.x * 255), (int)(color.y * 255), (int)(color.z * 255));
      }
      else
      {
        Ray ray;
        Ver color;
        double pos_x = x_min + (2*x+1)*cell_width/2.0f;
        double pos_y = y_min + (2*y+1)*cell_height/2.0f;
        double pos_z = -1;
  
        ray.o= Ver(0,0,0);
        ray.d=Ver(pos_x,pos_y,pos_z).norm();
        color = phonglighting(ray, 1) + ambient_light;
        Clamp(color);
        plot_pixel(x, y, (int)(color.x * 255), (int)(color.y * 255), (int)(color.z * 255));
      }
      
    }








  
    glEnd();
    glFlush();
      
    

  }
    

  // Ray test_ray;
  // double t;
  // int idx; 
  // test_ray.o=Ver(0,0,0);
  // test_ray.d=Ver(x_min+cell_width/2.0f,y_min+(HEIGHT+1)*cell_height/2.0f,-1);
  // bool test_out=SphereIntersect(test_ray,t,idx);
  // cout<<"The test result is "<<test_out<<" "<<t<<" "<<idx<<endl;



  printf("Done!\n"); fflush(stdout);
}

inline void UpdateProgress(float progress)
{
  int barWidth = 30;
  
  std::cout << "[";
  int pos = barWidth * progress;
  for (int i = 0; i < barWidth; ++i) {
    if (i < pos) std::cout << "=";
    else if (i == pos) std::cout << ">";
    else std::cout << " ";
  }
  std::cout << "] " << int(progress * 100.0) << " %\r";
  std::cout.flush();
};

void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  glColor3f(((double)r)/255.0f,((double)g)/255.0f,((double)b)/255.0f);
  glVertex2i(x,y);
}

void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  buffer[HEIGHT-y-1][x][0]=r;
  buffer[HEIGHT-y-1][x][1]=g;
  buffer[HEIGHT-y-1][x][2]=b;
}

void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
  plot_pixel_display(x,y,r,g,b);
  if(mode == MODE_JPEG)
    plot_pixel_jpeg(x,y,r,g,b);
}

void save_jpg()
{
  Pic *in = NULL;

  in = pic_alloc(640, 480, 3, NULL);
  printf("Saving JPEG file: %s\n", filename);

  memcpy(in->pix,buffer,3*WIDTH*HEIGHT);
  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);      

}

void parse_check(char *expected,char *found)
{
  if(strcasecmp(expected,found))
    {
      char error[100];
      printf("Expected '%s ' found '%s '\n",expected,found);
      printf("Parse error, abnormal abortion\n");
      exit(0);
    }

}

void parse_doubles(FILE*file, char *check, double p[3])
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check(check,str);
  fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
  printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE*file,double *r)
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check("rad:",str);
  fscanf(file,"%lf",r);
  printf("rad: %f\n",*r);
}

void parse_shi(FILE*file,double *shi)
{
  char s[100];
  fscanf(file,"%s",s);
  parse_check("shi:",s);
  fscanf(file,"%lf",shi);
  printf("shi: %f\n",*shi);
}

int loadScene(char *argv)
{
  FILE *file = fopen(argv,"r");
  int number_of_objects;
  char type[50];
  int i;
  Triangle t;
  Sphere s;
  Light l;
  fscanf(file,"%i",&number_of_objects);

  printf("number of objects: %i\n",number_of_objects);
  char str[200];

  parse_doubles(file,"amb:",ambient_light_);

  for(i=0;i < number_of_objects;i++)
    {
      fscanf(file,"%s\n",type);
      printf("%s\n",type);
      if(strcasecmp(type,"triangle")==0)
	{

	  printf("found triangle\n");
	  int j;

	  for(j=0;j < 3;j++)
	    {
	      parse_doubles(file,"pos:",t.v[j].position);
	      parse_doubles(file,"nor:",t.v[j].normal);
	      parse_doubles(file,"dif:",t.v[j].color_diffuse);
	      parse_doubles(file,"spe:",t.v[j].color_specular);
	      parse_shi(file,&t.v[j].shininess);
	    }

	  if(num_triangles == MAX_TRIANGLES)
	    {
	      printf("too many triangles, you should increase MAX_TRIANGLES!\n");
	      exit(0);
	    }
	  triangles_[num_triangles++] = t;
	}
      else if(strcasecmp(type,"sphere")==0)
	{
	  printf("found sphere\n");

	  parse_doubles(file,"pos:",s.position);
	  parse_rad(file,&s.radius);
	  parse_doubles(file,"dif:",s.color_diffuse);
	  parse_doubles(file,"spe:",s.color_specular);
	  parse_shi(file,&s.shininess);

	  if(num_spheres == MAX_SPHERES)
	    {
	      printf("too many spheres, you should increase MAX_SPHERES!\n");
	      exit(0);
	    }
	  spheres_[num_spheres++] = s;
	}
      else if(strcasecmp(type,"light")==0)
	{
	  printf("found light\n");
	  parse_doubles(file,"pos:",l.position);
	  parse_doubles(file,"col:",l.color);

	  if(num_lights == MAX_LIGHTS)
	    {
	      printf("too many lights, you should increase MAX_LIGHTS!\n");
	      exit(0);
	    }
	  lights_[num_lights++] = l;
	}
      else
	{
	  printf("unknown type in scene description:\n%s\n",type);
	  exit(0);
	}
    }
  return 0;
}

void Translate()
{
  Triangle t_;
  myTriangle t;
  Sphere s_;
  mySphere s;
  Light l_;
  myLight l;
  Vertex v0_,v1_,v2_;
  myVertex v0,v1,v2;


  for(int i=0;i<num_triangles;i++)
  {
    t_=triangles_[i];
    t=triangles[i];
    v0_=t_.v[0];
    v1_=t_.v[1];
    v2_=t_.v[2];

    v0.position=Ver(v0_.position[0],v0_.position[1],v0_.position[2]);
    v0.color_diffuse=Ver(v0_.color_diffuse[0],v0_.color_diffuse[1],v0_.color_diffuse[2]);
    v0.color_specular=Ver(v0_.color_specular[0],v0_.color_specular[1],v0_.color_specular[2]);
    v0.normal=Ver(v0_.normal[0],v0_.normal[1],v0_.normal[2]);
    v0.shininess=v0_.shininess;

    v1.position=Ver(v1_.position[0],v1_.position[1],v1_.position[2]);
    v1.color_diffuse=Ver(v1_.color_diffuse[0],v1_.color_diffuse[1],v1_.color_diffuse[2]);
    v1.color_specular=Ver(v1_.color_specular[0],v1_.color_specular[1],v1_.color_specular[2]);
    v1.normal=Ver(v1_.normal[0],v1_.normal[1],v1_.normal[2]);
    v1.shininess=v1_.shininess;

    v2.position=Ver(v2_.position[0],v2_.position[1],v2_.position[2]);
    v2.color_diffuse=Ver(v2_.color_diffuse[0],v2_.color_diffuse[1],v2_.color_diffuse[2]);
    v2.color_specular=Ver(v2_.color_specular[0],v2_.color_specular[1],v2_.color_specular[2]);
    v2.normal=Ver(v2_.normal[0],v2_.normal[1],v2_.normal[2]);
    v2.shininess=v2_.shininess;


    triangles[i].v[0]=v0;
    triangles[i].v[1]=v1;
    triangles[i].v[2]=v2;


  }
  for(int i=0;i<num_spheres;i++)
  {
    s_=spheres_[i];
    

    spheres[i].position=Ver(s_.position[0],s_.position[1],s_.position[2]);
    spheres[i].color_diffuse=Ver(s_.color_diffuse[0],s_.color_diffuse[1],s_.color_diffuse[2]);
    spheres[i].color_specular=Ver(s_.color_specular[0],s_.color_specular[1],s_.color_specular[2]);
    spheres[i].shininess=s_.shininess;
    spheres[i].radius=s_.radius;
  }
  for(int i=0;i<num_lights;i++)
  {
    l_=lights_[i];
    

    lights[i].position=Ver(l_.position[0],l_.position[1],l_.position[2]);
    lights[i].color=Ver(l_.color[0],l_.color[1],l_.color[2]);
 
  }

  ambient_light=Ver(ambient_light_[0],ambient_light_[1],ambient_light_[2]);



}

void display()
{

}

void init()
{
  glMatrixMode(GL_PROJECTION);
  glOrtho(0,WIDTH,0,HEIGHT,1,-1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void idle()
{
  //hack to make it only draw once
  static int once=0;
  if(!once)
  {
      init_lights_super();
      draw_scene();
      if(mode == MODE_JPEG)
	save_jpg();
    }
  once=1;
}


void print_scene()
{
  for (int i=0;i<num_lights;i++)
  {
    cout<<"light"<<" "<<i<<":color{"<<lights[i].color.x<<","<<lights[i].color.y<<","<<lights[i].color.z<<"}"<<endl;
  }








  // for (int i=0;i<num_spheres;i++)
  // {
  //   cout<<"sphere"<<" "<<i<<":pos{"<<spheres[i].position.x<<","<<spheres[i].position.y<<","<<spheres[i].position.z<<"}"<<endl;
  // }


  //  for (int i=0;i<num_triangles;i++)
  // {
  //   cout<<"triangle"<<" "<<i<<":pos{"<<triangles[i].v[0].position.x<<","<<triangles[i].v[0].position.y<<","<<triangles[i].v[0].position.z<<"}"<<endl;
  // }


  


}

int main (int argc, char ** argv)
{
 if (argc<2 || argc > 3)
  {  
    printf ("usage: %s <scenefile> [jpegname]\n", argv[0]);
    exit(0);
  }
  if(argc == 3)
    {
      mode = MODE_JPEG;
      filename = argv[2];
    }
  else if(argc == 2)
    mode = MODE_DISPLAY;

  glutInit(&argc,argv);
  loadScene(argv[1]);
  Translate();
  print_scene();

  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(WIDTH,HEIGHT);
  int window = glutCreateWindow("Ray Tracer");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  init();
  glutMainLoop();
}

