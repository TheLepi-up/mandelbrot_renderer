# Mandelbrot set renderer
This Mandelbrot set renderer allows you to zoom in indefinitly by utilising arbitrary precision fixed-point numbers. The SFML-Windows allows you to zoom into different regions of the fractal and start a thread to render a particular view in high resolution.

## Input
* Left-click: zoom in where the cursor is
* Right-click: zoom out and move the current mouse position to the center
* space: start rendering the current view as an image of 2048x2048px.
* page-up: increase the maximum number of iterations
* page-down: decrease the maximum number of iterations
* s: save the current state to resume from in case of a crash or abnormal termination
* esc: go to 0, 0 with a zoom of 2^1 (fully zoomed out)

## Features
When closing the Window, the progress of all render-threads is saved as images and the metadata is saved into state.tsv. When the program starts up again, it will load the progress and continue where it left off, including the position and zoom of the explorer.