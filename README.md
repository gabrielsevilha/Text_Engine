# Text_Engine

This is a unique header library, that serves as a fast way to draw text using OpenGL and freetype2, and serves also as example of freetype2 library. </br>

Dependences: OpenGL 1.1+ and FreeType2
  
  - You need load OpenGL functions before include this library, as the exemple bellow. </br>
  - In the exemple OpenGL was loaded with Glfw3 library. </br>


Exemple with glfw3:
```C

#include<GLFW/glfw3.h>
#include"text_engine.h"

int main(){

	glfwInit();
	GLFWwindow* window = glfwCreateWindow(800,600,"text_engine",0,0);
	glfwMakeContextCurrent(window);
	
	Font* font = createFont("font.ttf",48);
	
	while(!glfwWindowShouldClose(window)){
	
		glfwWaitEvents();
		
		glClear(GL_COLOR_BUFFER_BIT);
		
		drawText(font, "Hello, World!", getTextAlignCenter(font,"Hello, World!",400), 600/2-getFontHeight(font));
    
		glfwSwapBuffers(window);
		
	}
	
	free(font);
	
	return 0;
	
}

```
Exemple was compiled with: -lglfw -lGL \`pkg-config --cflags --libs freetype2\` </br>

Obs: (Of course, glfw3 is opitional for this library, but just make sure that OpenGL funcions was loaded). </br>
