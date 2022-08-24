/*
.=================================================================================.

MIT License

Copyright (c) 2021-2022 Gabriel Sevilha

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

This is a unique header library, that serves that a fast way to draw text using OpenGL and FreeType2, and serves also as example of freetype2 library.

Dependences: OpenGL 1.1+ and freetype2

You need load OpenGL functions before include this library, as the exemple bellow.
In the exemple opengl was loaded with glfw3 library (Of course, glfw3 is opitional for this library):

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
		
		drawText(font, "Hello, World!", 800/2-getSizeText(font,"Hello, World!")/2, 600/2-font->size);
    
		glfwSwapBuffers(window);
		
	}
	
	free(font);
	
	return 0;
	
}

//Exemple was compiled with: -lglfw -lGL `pkg-config --cflags --libs freetype2`

*/

/*

REFERENCE:

	#define TEXT_ENGINE_USE_MODERN_OPENGL //Use this before you include to use Modern OpenGL
	
	typedef struct Letter;
	typedef struct Font;

	Font* createFont(const char* font_name, int size);
	void drawText(Font* font, const unsigned char* text, int x, int y);

	void setFontColor(Font* font, float r, float g, float b, float a);
	void setFontCanvasSize(Font* font, int width, int height);
	void setTabSize(Font* font, const int tab_size);
	int getSizeText(Font* font,const char* text);
	
	//Internal use, but you also can use:
		void fontCreateOrthographicMatrix(float left, float right, float bottom, float top, float near, float far, float* matrix);
	
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

	int size, tab_size;
	Letter letters[255];
	
	float color_r, color_g, color_b, color_a;
	
	float projection_matrix[16];
	int canvas_width, canvas_height;
	
	unsigned int shader, vertex_array;

}Font;

void fontCreateOrthographicMatrix(float left, float right, float bottom, float top, float near, float far, float* matrix){

	float dif_right_left = right - left;
	float dif_top_bottom = top - bottom;
	float dif_far_near = far - near;

	matrix[0] = 2.0f / (dif_right_left);
	matrix[5] = 2.0f / (dif_top_bottom);
	matrix[10] = -2.0f / (dif_far_near);
	matrix[12] = -( (right+left)/(dif_right_left) );
	matrix[13] = -( (top+bottom)/(dif_top_bottom) );
	matrix[14] = -( (far+near)/(dif_far_near) );
	matrix[15] = 1.0f;

}

//============================== If Using Modern OpenGL ==============================

#ifdef TEXT_ENGINE_USE_MODERN_OPENGL

Font* createFont(const char* font_name, int size){

	Font* font = (Font*)malloc(sizeof(Font));
	font->size = size;
	font->tab_size = 4;
	font->canvas_width = 800;
	font->canvas_height = 600;
	font->color_r = 1.0f;
	font->color_g = 1.0f;
	font->color_b = 1.0f;
	font->color_a = 1.0f;
	
	for(int i = 0; i < 16; i++) font->projection_matrix[i] = 0.0f;
	font->projection_matrix[0] = font->projection_matrix[5] = font->projection_matrix[10] = font->projection_matrix[15] = 1.0f;
	fontCreateOrthographicMatrix(0,font->canvas_width,font->canvas_height,0,-1.0,1.0,font->projection_matrix);
	
	FT_Library ft;
	if( FT_Init_FreeType(&ft) ){
		puts("Text Engine: Failed to init FreeType2 Library");
	}
	FT_Face face;
	if( FT_New_Face(ft,font_name,0,&face) ){
		printf("Text Engine: Failed to load font: %s\n",font_name);
	}
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
	
	//Setting Shader
	const char* vertex_shader_source = R"(
	
		#version 110
		
		attribute vec2 in_vertex;
		attribute vec2 in_uv;
		
		uniform mat4 projection;
		uniform vec2 position;
		uniform vec2 size;
		
		varying vec2 out_uv;
		
		void main(){
			out_uv = in_uv;
			gl_Position = projection * vec4(in_vertex * size + position,0.0,1.0);
		}
		
	)";
	
	const char* fragment_shader_source = R"(
	
		#version 110
		
		varying vec2 out_uv;
		
		uniform sampler2D texture;
		uniform vec4 color;
		
		void main(){
			gl_FragColor = color * texture2D(texture,out_uv).a;
		}
		
	)";
	
	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader,1,&vertex_shader_source,0);
	glCompileShader(vertex_shader);
	unsigned int success;
	unsigned char infolog[512];
	glGetShaderiv(vertex_shader,GL_COMPILE_STATUS,&success);
	if(!success){
		glGetShaderInfoLog(vertex_shader,512,0,infolog);
		printf("Text Engine: Font creation error on create Vertex Shader: %s\n",infolog);
	}
	
	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader,1,&fragment_shader_source,0);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader,GL_COMPILE_STATUS,&success);
	if(!success){
		glGetShaderInfoLog(fragment_shader,512,0,infolog);
		printf("Text Engine: Font creation error on create Fragment Shader: %s\n",infolog);
	}
	
	font->shader = glCreateProgram();
	glAttachShader(font->shader,vertex_shader);
	glAttachShader(font->shader,fragment_shader);
	glLinkProgram(font->shader);
	
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	
	//Setting Vertex Array
	float vertex[] = {
		0.0,0.0,
		0.0,1.0,
		1.0,1.0,
		1.0,0.0
	};
	float uv[] = {
		0.0,0.0,
		0.0,1.0,
		1.0,1.0,
		1.0,0.0
	};
	
	glGenVertexArrays(1,&font->vertex_array);
	glBindVertexArray(font->vertex_array);
	
	unsigned int vbo[2];
	glGenBuffers(2,vbo);
	glBindBuffer(GL_ARRAY_BUFFER,vbo[0]);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*8,vertex,GL_STATIC_DRAW);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(GL_ARRAY_BUFFER,vbo[1]);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*8,uv,GL_STATIC_DRAW);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,0);
	glEnableVertexAttribArray(1);
	
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);

	return font;
}

void drawText(Font* font, const unsigned char* text, int x, int y){

	glUseProgram(font->shader);
	glBindVertexArray(font->vertex_array);
	
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(font->shader,"texture"),0);
	
	glUniform4f(glGetUniformLocation(font->shader,"color"),font->color_r,font->color_g,font->color_b,font->color_a);
	
	glUniformMatrix4fv(glGetUniformLocation(font->shader,"projection"),1,GL_FALSE,font->projection_matrix);

	int is_gltexture2d_active;
	glGetIntegerv(GL_TEXTURE_2D,&is_gltexture2d_active);
	if(!is_gltexture2d_active)
		glEnable(GL_TEXTURE_2D);
		
	int is_gldepthtest_active;
	glGetIntegerv(GL_DEPTH_TEST,&is_gldepthtest_active);
	if(is_gldepthtest_active)
		glDisable(GL_DEPTH_TEST);
		
	int is_glblend_active;
	glGetIntegerv(GL_BLEND,&is_glblend_active);
	if(!is_glblend_active)
		glEnable(GL_BLEND);

	int temp_gl_blend_src, temp_gl_blend_dst;
	glGetIntegerv(GL_BLEND_SRC,&temp_gl_blend_src);
	glGetIntegerv(GL_BLEND_DST,&temp_gl_blend_dst);
	if(temp_gl_blend_src != GL_SRC_ALPHA || temp_gl_blend_dst != GL_ONE_MINUS_SRC_ALPHA)
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	int initial_x = x, initial_y = y, line = font->size;
	
	for(int i = 0; i < strlen(text); i++){
	
		if(text[i] == '\n'){
		
			line += font->size;
			x = initial_x;
			
		}else if(text[i] == '	'){
		
			x += font->letters[31].width * font->tab_size;
			
		}else{
		
			y = line + (initial_y-font->letters[(int)text[i]].top);
			
			glUniform2f(glGetUniformLocation(font->shader,"position"),x,y);
			glUniform2f(glGetUniformLocation(font->shader,"size"),font->letters[(int)text[i]].width,font->letters[(int)text[i]].rows);

			glBindTexture(GL_TEXTURE_2D,font->letters[(int)text[i]].texture);
			
			glDrawArrays(GL_QUADS,0,4);
			
			x += font->letters[(int)text[i]].advance>>6;
			
		}
		
	}
	
	if(is_gldepthtest_active)
		glEnable(GL_DEPTH_TEST);
	
	if(temp_gl_blend_src != GL_SRC_ALPHA || temp_gl_blend_dst != GL_ONE_MINUS_SRC_ALPHA)
		glBlendFunc(temp_gl_blend_src,temp_gl_blend_dst);
	
	if(!is_gltexture2d_active)
		glDisable(GL_TEXTURE_2D);
	
	if(!is_glblend_active)
		glDisable(GL_BLEND);
	
	glBindTexture(GL_TEXTURE_2D,0);
	glBindVertexArray(0);
	glUseProgram(0);

}

#else

//============================== If Using OpenGL Compatibility Mode (Imediate Mode) ==============================

Font* createFont(const char* font_name, int size){

	Font* font = (Font*)malloc(sizeof(Font));
	font->size = size;
	font->tab_size = 4;
	font->canvas_width = 800;
	font->canvas_height = 600;
	font->color_r = 1.0f;
	font->color_g = 1.0f;
	font->color_b = 1.0f;
	font->color_a = 1.0f;
	
	for(int i = 0; i < 16; i++) font->projection_matrix[i] = 0.0f;
	font->projection_matrix[0] = font->projection_matrix[5] = font->projection_matrix[10] = font->projection_matrix[15] = 1.0f;
	fontCreateOrthographicMatrix(0,font->canvas_width,font->canvas_height,0,-1.0,1.0,font->projection_matrix);
	
	FT_Library ft;
	if( FT_Init_FreeType(&ft) ){
		puts("Text Engine: Failed to init FreeType2 Library");
	}
	FT_Face face;
	if( FT_New_Face(ft,font_name,0,&face) ){
		printf("Text Engine: Failed to load font: %s\n",font_name);
	}
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

void drawText(Font* font, const unsigned char* text, int x, int y){

	int matrix_mode;
	glGetIntegerv(GL_MATRIX_MODE,&matrix_mode);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,font->canvas_width,font->canvas_height,0,-1.0,1.0);
	glMatrixMode(matrix_mode);

	int is_gltexture2d_active;
	glGetIntegerv(GL_TEXTURE_2D,&is_gltexture2d_active);
	if(!is_gltexture2d_active)
		glEnable(GL_TEXTURE_2D);
		
	int is_gldepthtest_active;
	glGetIntegerv(GL_DEPTH_TEST,&is_gldepthtest_active);
	if(is_gldepthtest_active)
		glDisable(GL_DEPTH_TEST);
		
	int is_glblend_active;
	glGetIntegerv(GL_BLEND,&is_glblend_active);
	if(!is_glblend_active)
		glEnable(GL_BLEND);

	int temp_gl_blend_src, temp_gl_blend_dst;
	glGetIntegerv(GL_BLEND_SRC,&temp_gl_blend_src);
	glGetIntegerv(GL_BLEND_DST,&temp_gl_blend_dst);
	if(temp_gl_blend_src != GL_SRC_ALPHA || temp_gl_blend_dst != GL_ONE_MINUS_SRC_ALPHA)
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		
	float old_color[4];
	glGetFloatv(GL_CURRENT_COLOR,old_color);
	glColor4f(font->color_r,font->color_g,font->color_b,font->color_a);

	int initial_x = x, initial_y = y, line = font->size;
	
	for(int i = 0; i < strlen(text); i++){
	
		if(text[i] == '\n'){
		
			line += font->size;
			x = initial_x;
			
		}else if(text[i] == '	'){
		
			x += font->letters[31].width * font->tab_size;
			
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
	
	glColor4f(old_color[0], old_color[1], old_color[2], old_color[3]);
	
	if(is_gldepthtest_active)
		glEnable(GL_DEPTH_TEST);
	
	if(temp_gl_blend_src != GL_SRC_ALPHA || temp_gl_blend_dst != GL_ONE_MINUS_SRC_ALPHA)
		glBlendFunc(temp_gl_blend_src,temp_gl_blend_dst);
	
	if(!is_gltexture2d_active)
		glDisable(GL_TEXTURE_2D);
	
	if(!is_glblend_active)
		glDisable(GL_BLEND);

}

#endif

void setFontColor(Font* font, float r, float g, float b, float a){
	font->color_r = r;
	font->color_g = g;
	font->color_b = b;
	font->color_a = a;
}

void setFontCanvasSize(Font* font, int width, int height){
	font->canvas_width = width;
	font->canvas_height = height;
	fontCreateOrthographicMatrix(0,font->canvas_width,font->canvas_height,0,-1.0,1.0,font->projection_matrix);
}

void setTabSize(Font* font, const int tab_size){
	font->tab_size = tab_size;
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
