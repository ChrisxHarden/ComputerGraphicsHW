# HW1 for Computer Graphics
## Abstract
This program is designed to take in grayscale picture and produce the heightmap according to the grayscale value  at the specific pixel by using OpenGL. It can also handle color pictures by using the red value in the RGB tuple stored at the pixel.
*************************
## How to run the program
Using command line is a preferred way, as it's been programmed on MacOS, only the command line in MacOS is provided. 

    ./assign1 OhioPyle-256.jpg
    
Notice: Feel free to test any picture you like, no need for the size to be [256,256].
*************************
### Display Mode
The default display mode is points (in blue), by press 1 , you can get back this mode. Press 2 for wireframe mode (in cran-blue) and 3 for solid triangle mode (white and grey). Press 4, you can access to the extra mode of adding wireframe and solid triangle. The color of each vertex is related to the grayscale value with some regulation*, or the actual color on that pixel if you are not using grayscale picture. 

<font size=1> *If I had just directly used the grayscale value, the lowest part of the whole landscape would vanish in the darkness as there will be set to have the color close to black, so I added 0.2 at each channel of the RGB color for each vertex and let the grayscale value multiplied 0.5 to make the value at each RGB channel has the value between [0.2,0.7] to gain a better look.</font> 
*************************

### Transform
 You can rotate the landscale by holding on the left button of the mouse and move it around. Or as you are using Macbook, just press the touchpad and move to rotate the landscape against **x-axis and y-axis**. If you want to rotate it against **z-axis**, keep pressing the **option** key and move the touchpad or holding the middle button of the mouse and move it.

If you want to translate the scene, press the **T** key and move the mouse. Also, any action on **z-axis** requires an additional press on **option** key or middle button of the mouse. 

If you want to scale the landscape, press **S** and move the mouse. Also, holding on **option** or middle button of the mouse if you want to scale the scene in **z-axis**.

*************************
## For extra credits
1. handle bpp=3 picture
2. adding wireframe and solid triangle together. :)