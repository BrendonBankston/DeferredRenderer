# DeferredRenderer
Basic deferred renderer written using OpenGL

This is a slightly modified academic project from when I was a senior. It uses OpenGL to perform deferred rendering, which is where lighting calculations are "deferred" to a second pass in order to save calculations. 
The current scene is very simple, so using deferred rendering doesn't save much time, but if you created a more complicated scene, it would ensure lighting calculations are only performed on things visible to the camera, such as things not hidden behind other objects.

The center object can be easily changed by modifying source code to use a different obj file. The current object reader cannot read in information such as UVs and vertex normals, however.
