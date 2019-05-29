#include <U8g2lib.h>
#include "Projection.h"

#define UPDATE_DELAY  0

U8G2_SH1107_64X128_F_4W_HW_SPI u8g2(U8G2_R0,14, /* dc=*/ 27, /* reset=*/ 33);

#define LCDWIDTH     64
#define LCDHEIGHT   128

// Transform for plot
Transform plotTrans;
float angle = 0;

// Array of plot points
point3 **points;
unsigned int numPtsX, numPtsY;

// Create a camera
Camera cam(LCDWIDTH, LCDHEIGHT);

void setup()
{
    u8g2.begin();

    // Set camera back and up a few units and angle downward.
    // 
    // Note that camera's projection axis is its own Z+ axis,
    // so an x-angle of -90 deg aligns it with the world Y+
    // axis, and less than -90 will angle the camera's
    // projection axis further downward.
    cam.transform = Transform(-90 - 10, 0, 0, 1, 1, 1, 0, -7, 2);
    
    // Uncomment these two lines for orthographic projection
    //cam.projMode = PROJ_ORTHO;
    //cam.orthoViewWidth = 3.0;
    
    // Compute the points in the Gaussian plot
    computePoints();
}

void loop()
{
   u8g2.clearBuffer();
   u8g2.setFont(u8g2_font_6x12_tr);
   u8g2.drawStr(0, 7, "Gaussian");    
   
    // Update plot transform and draw points
    plotTrans = Transform(0, 0, angle += 2, 1, 1, 3, 0, 0, 0);
    drawPoints();
    
    u8g2.sendBuffer();
    delay(UPDATE_DELAY);
}

void computePoints()
{
    float ux = 0, uy = 0;      // Means
    float sx = 0.5, sy = 0.7;  // Standard deviations
    
    // Plotting options
    float minX = -3, maxX = 3;
    float minY = -3, maxY = 3;
    float xStep = 0.75, yStep = 0.75;
    
    numPtsX = (maxX - minX) / xStep;
    numPtsY = (maxY - minY) / yStep;
    
    points = new point3*[numPtsX];
    
    if (points != NULL)
    {
        for (unsigned int n = 0; n < numPtsX; n++)
        {
            // Try to allocate next row of points
            points[n] = new point3[numPtsY];
            
            if (points[n] != NULL)
            {
                for (unsigned int m = 0; m < numPtsY; m++)
                {
                    // Compute Gaussian point
                    float x = (n * xStep) + minX;
                    float y = (m * yStep) + minY;
                    float z = gauss(x, y, ux, uy, sx, sy);
                    
                    points[n][m] = { x, y, z };
                }
            }
        }
    }
}

float gauss(float x, float y, float ux, float uy, float sx, float sy)
{
    return exp(-(
        ((x - ux) * (x - ux) / (2 * sx * sx)) +
        ((y - uy) * (y - uy) / (2 * sy * sy))))
        / (2 * PI * sx * sy);
}

void drawPoints()
{
    for (unsigned int n = 0; n < numPtsX; n++)
    {
        // Check for low memory
        if (points == NULL || points[n] == NULL)
        {
            // uView.clear(PAGE);
            // uView.setCursor(0, 0);
            // uView.print("Out of\nmemory!\n\nIncrease\nX, Y step\nsize");

            u8g2.clearBuffer();            
            u8g2.setCursor(0, 0);
            u8g2.print("Out of\nmemory!\n\nIncrease\nX, Y step\nsize");
        }
        else
        {
            for (unsigned int m = 0; m < numPtsY; m++)
            {
                // Transform and project 3D point
                point2 pt = cam.project(plotTrans * points[n][m]);
                    
                // Draw point if not clipped
                if (!isnan(pt.x))
                {
                    u8g2.drawPixel(pt.x, pt.y);
                }
            }
        }
    }
}
