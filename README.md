# tinyrenderer study

My personal study of the tinyrenderer [lessons](https://github.com/ssloy/tinyrenderer/wiki) by ssloy


# Notes

## Model Space (aka: Local Space)
3D object coordinates without any distortion

a road, a car, a tree;

## World Space
The space that contains 3D objects located in.

a road, on top of that road a car that sits by a tree;

## Camera Space (aka: View Space)
A space that re-defines(translates) the each vertices of all 3D objects within world space according to view direction (look direction)

## Backface culling
Simply discards any surface that out of view. If I draw an object on top of a paper and if the object I am drawing for an example is a cube and,
if I am looking at it on a certain direction and an angle the back faces of that cube that I can't see simply I don't draw.

## Projection
With drawing anology this can be described like; If I am going to draw a landscape that is in my view, that i am looking at (camera|view space) and,
let's say I want to take into account of the perspective the way I see, I would draw the objects that are far away becomes smaller and the ones are close to me are bigger.

I apply this translation for each vertex of all the objects in my view to get perspectively accurate drawing.

## Clipping space
With the same anology, if I am looking towards to a landscape and then drawing it means I see certain portion of the landscape, so I have a certain clipped area of view.
This clipped volume/area is my clipping space. In perspective view this called a frustum.

## Perpective divide
Each vertices depth value used to transform into image space (NDC - normalized device coordinate).
// TODO: study more understand more!!!

## Screen space
Each point in NDC translated into screen coordinate (Monitor 800x600)
