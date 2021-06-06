/*
.=================================================================================.

MIT License

Copyright (c) 2021 Gabriel Sevilha

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

.=================================================================================.

README:

This is a unique header library, that serves that a fast way to draw text using OpenGL and freetype2, and serves also as example of freetype2 library.
The code are write using OpenGL Imediate Mode because beside a library, this code need be understandeble to learn freetype2, and also be used in low-end computers.

Dependences: OpenGL 1.1 and freetype2

This library use OpenGL Imediate Mode.
You need orthographic projection matrix, you get this with:
glOrtho(0,view_port_width,view_port_height,0,0,1);
You need load OpenGL functions before include this library, as the exemple bellow.
In the exemple opengl was loaded with glfw3 library.
Exemple with glfw3:

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

Exemple was compiled with: -lglfw -lGL `pkg-config --cflags --libs freetype2`

Obs: Of course, glfw3 is opitional for this library, but only make sure that OpenGL 1.1 funcions was loaded.

*/
	
#ifndef TEXT_ENGINE
#define TEXT_ENGINE

#include<ft2build.h>
#include FT_FREETYPE_H

typedef struct{

	unsigned int texture;
	int left, top;
	unsigned int width, rows;
	long int advance;

}Letter;

typedef struct{

	int size;
	Letter letters[255];

}Font;

Font* createFont(const char* font_name, int size){

	Font* font = (Font*)malloc(sizeof(Font));
	font->size = size;
	
	FT_Library ft;
	FT_Init_FreeType(&ft);
	FT_Face face;
	FT_New_Face(ft,font_name,0,&face);
	FT_Set_Pixel_Sizes(face,0,size);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	for(int i = 0; i < 255; i++){
	
		FT_Load_Char(face,i,FT_LOAD_RENDER);
		
		glGenTextures(1,&font->letters[i].texture);
		glBindTexture(GL_TEXTURE_2D,font->letters[i].texture);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D,0,GL_ALPHA,face->glyph->bitmap.width,face->glyph->bitmap.rows,0,GL_ALPHA,GL_UNSIGNED_BYTE,face->glyph->bitmap.buffer);
		glBindTexture(GL_TEXTURE_2D,0);
		
		font->letters[i].left = face->glyph->bitmap_left;
		font->letters[i].top = face->glyph->bitmap_top;
		font->letters[i].width = face->glyph->bitmap.width;
		font->letters[i].rows = face->glyph->bitmap.rows;
		font->letters[i].advance = face->glyph->advance.x;
		
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	return font;
}

void drawText(Font* font, const char* text, int x, int y){

	unsigned int is_gltexture2d_active;
	glGetIntegerv(GL_TEXTURE_2D,&is_gltexture2d_active);
	if(!is_gltexture2d_active)
		glEnable(GL_TEXTURE_2D);
		
	unsigned int is_glblend_active;
	glGetIntegerv(GL_BLEND,&is_glblend_active);
	if(!is_glblend_active)
		glEnable(GL_BLEND);

	unsigned int temp_gl_blend_src, temp_gl_blend_dst;
	glGetIntegerv(GL_BLEND_SRC,&temp_gl_blend_src);
	glGetIntegerv(GL_BLEND_DST,&temp_gl_blend_dst);
	if(temp_gl_blend_src != GL_SRC_ALPHA || temp_gl_blend_dst != GL_ONE_MINUS_SRC_ALPHA)
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	int initial_x = x, initial_y = y, line = font->size;
	
	for(int i = 0; i < strlen(text); i++){
	
		if(text[i] == '\n'){
		
			line += font->size;
			x = initial_x;
			
		}else{
		
			y = line + (initial_y-font->letters[(int)text[i]].top);
			
			glBindTexture(GL_TEXTURE_2D,font->letters[(int)text[i]].texture); 
			glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex2i(x,y);
				glTexCoord2f(0,1);
				glVertex2i(x,y+font->letters[(int)text[i]].rows);
				glTexCoord2f(1,1);
				glVertex2i(x+font->letters[(int)text[i]].width,y+font->letters[(int)text[i]].rows);
				glTexCoord2f(1,0);
				glVertex2i(x+font->letters[(int)text[i]].width,y);
			glEnd();
			glBindTexture(GL_TEXTURE_2D,0);
			
			x += font->letters[(int)text[i]].advance>>6;
			
		}
		
	}
	
	if(temp_gl_blend_src != GL_SRC_ALPHA || temp_gl_blend_dst != GL_ONE_MINUS_SRC_ALPHA)
		glBlendFunc(temp_gl_blend_src,temp_gl_blend_dst);
	
	if(!is_gltexture2d_active)
		glDisable(GL_TEXTURE_2D);
	
	if(!is_glblend_active){
		glDisable(GL_BLEND);
	}

}

int getSizeText(Font* font,const char* text){

	int max_width = 0;
	int x = 0;
	
	for(int i = 0; i < strlen(text); i++){
	
		if(text[i] == '\n'){
			if(max_width < x) max_width = x;
			x = 0;
		}else x += font->letters[(int)text[i]].advance>>6;
		
	}
	
	if(max_width < x) max_width = x;
	
	return max_width;
}

#endif
