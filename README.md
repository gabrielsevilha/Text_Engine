# Text_Engine

This Library is a unique header file. Fast way to draw text using freetype2 This library serves also as example of freetype2 library.

Dependences: OpenGL 1.1 and freetype2
  
  * You need orthographic projection matrix, you get this with: </br>
    * `glOrtho(0,view_port_width,view_port_height,0,0,1);` </br>
  * You need load OpenGL functions before include this library, as the exemple bellow. </br>
  * In the exemple opengl was loaded with glfw3 library. </br>


Exemple with glfw3:
```C

#include<GLFW/glfw3.h> 
#include"text_engine.h"

int main(){

	glfwInit();
	GLFWwindow* window = glfwCreateWindow(800,600,"text_engine",0,0);
	glfwMakeContextCurrent(window);
	glOrtho(0,800,600,0,0,1);
	
	Font* font = createFont("DejaVuSerif.ttf",48);
	
	while(!glfwWindowShouldClose(window)){
	
		glfwWaitEvents();
		
		glColor4f(1,1,1,1);
		drawText(font, "Hello, World!", 800/2-getSizeText(font,"Hello, World!")/2, 600/2-font->size);
    
		glfwSwapBuffers(window);
		
	}
	
	free(font);
	
	return 0;
	
}
```
Exemple was compiled with: -lglfw3 -lGL \`pkg-config --cflags --libs freetype2\` </br>
(Of course, glfw3 is opitional for this library, but only make sure that OpenGL 1.1 funcions was loaded.) </br>
