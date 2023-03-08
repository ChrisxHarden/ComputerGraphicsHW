# HW2 for Computer Graphics
## Abstract
This program is designed to use Catmull-Rom to draw splines and move camera along the spline drawn. 
*************************
## How to run the program
Using command line is a preferred way, as it's been programmed on MacOS, only the command line in MacOS is provided. 

    ./assign2 ./tracks.txt
    
Notice: store the address of spline file(.sp) in that 'tracks.ext' file for different tracks, although it can drawn several splines, the moing along will only be on the first rail.
*************************
### Display Mode
The default display mode is showing the rail drawn by bruteforce staticly, by press 1 , you can get back this mode.It also demonstrate the actuall spline in cyan color. Press 2 for moving mode (on the rail drawn by subdivision) and 3 for overlook the track drawn by subdivision. 

*************************

### Transform
 You can rotate the landscale by holding on the left button of the mouse and move it around. Or as you are using Macbook, just press the touchpad and move to rotate the landscape against **x-axis and y-axis**. If you want to rotate it against **z-axis**, keep pressing the **option** key and move the touchpad or holding the middle button of the mouse and move it.

If you want to translate the scene, press the **T** key and move the mouse. Also, any action on **z-axis** requires an additional press on **option** key or middle button of the mouse. 

If you want to scale the landscape, press **S** and move the mouse. Also, holding on **option** or middle button of the mouse if you want to scale the scene in **z-axis**.

*************************
## For extra credits
1. Draw t-shape cross section;
2. Draw double rail;
3. Draw the spline in both brute-force and subdivision fashion;
4. Modify the velocity of the camera to be more realistic*;
   <font size=1> *I set the velocity of the movement to be at least 0.005 so that the rollercoaster won't stop.</font> 
5. Draw the textture mapped-support bar of the rail using mipmapping for anti-aliasing;Using the t-picture for skybox;
