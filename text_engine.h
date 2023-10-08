/*
===================================== LICENSE ============================================

	MIT License

	Copyright (c) 2021-2023 Gabriel Sevilha

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

==========================================================================================
*/

/*

Text Engine 1.2.25 Copyright (C) Gabriel Sevilha.

This is a unique header library, that serves as a fast way to draw text using OpenGL and FreeType2, and serves also as example of freetype2 library.

Dependences: OpenGL 1.1+ and freetype2

You need load OpenGL functions before include this library, as the exemple bellow.
In the exemple OpenGL was loaded with glfw3 library (Of course, glfw3 is opitional for this library).
You must define "TEXT_ENGINE_IMPLEMENTATION" before the LAST include call of this library.

Exemple compiled on linux with: -lglfw -lGL `pkg-config --cflags --libs freetype2`

	#include<GLFW/glfw3.h>

	#define TEXT_ENGINE_MAX_GLYPHS_COUNT 1024 //If you do not define it, the default is 512
	
	#define TEXT_ENGINE_USE_MODERN_OPENGL //If you do not define it, text engine will use the 1.1 immediate opengl mode

	#define TEXT_ENGINE_IMPLEMENTATION
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

*/
	
#ifndef _TEXT_ENGINE
#define _TEXT_ENGINE

#include<math.h>

#include<ft2build.h>
#include FT_FREETYPE_H

#ifndef TEXTENGINEDEF
	#ifndef TEXT_ENGINE_STATIC
		#ifdef TEXT_ENGINE_STATIC_INLINE
			#define TEXTENGINEDEF static inline
		#else
			#define TEXTENGINEDEF extern
		#endif
	#else
		#ifdef TEXT_ENGINE_STATIC_INLINE
			#define TEXTENGINEDEF static inline
		#else
			#define TEXTENGINEDEF static
		#endif
	#endif
#endif

#ifndef TEXT_ENGINE_MAX_GLYPHS_COUNT 
#define TEXT_ENGINE_MAX_GLYPHS_COUNT 512
#endif

typedef struct{

	unsigned int texture;
	int left, top;
	unsigned int width, rows;
	long int advance;

}Letter;

typedef struct{

	int size, tab_size;
	float scale_x, scale_y;
	Letter letters[TEXT_ENGINE_MAX_GLYPHS_COUNT];
	
	float depth;
	
	float color_r, color_g, color_b, color_a;
	
	float transform_matrix[16];
	float projection_matrix[16];
	int canvas_width, canvas_height, canvas_depth;
	
	unsigned int shader, vertex_array;
	
	int free_transform;

}Font;

TEXTENGINEDEF Font* createFont(const char* font_name, int size);

TEXTENGINEDEF void drawText(Font* font, const unsigned char* text, int x, int y);

TEXTENGINEDEF void setFontFreeTransform(Font* font, int free_transform); //Able you to change font->matrix_transform variable by yourself.

TEXTENGINEDEF void setFontDepth(Font* font, float depth);

TEXTENGINEDEF void setFontColor(Font* font, float r, float g, float b, float a);

TEXTENGINEDEF void setFontCanvasSize(Font* font, int width, int height, int depth); //Set size of values that will be share with orthographic matrix.

TEXTENGINEDEF void setTabSize(Font* font, const int tab_size);

TEXTENGINEDEF void setFontScale(Font* font, float scale); //Scale is not equal as font pixels size

TEXTENGINEDEF void setFontScaleInPixels(Font* font, float scale_in_pixels); //Simulate pixels size on scale

TEXTENGINEDEF int getSizeText(Font* font, const unsigned char* text);

TEXTENGINEDEF int getFontHeight(Font* font); //Return font->size * font->scale_y;

TEXTENGINEDEF int getTextAlignRight(Font* font, const unsigned char* text, int position_x);

TEXTENGINEDEF int getTextAlignCenter(Font* font, const unsigned char* text, int position_x);

//Internal Math Functions
TEXTENGINEDEF void fontMultiplyMatrix4x4(float* m1, float* m2, float* dest);

TEXTENGINEDEF void fontIdentityMatrix4x4(float* m);

TEXTENGINEDEF void fontTranslateMatrix4x4(float* m, float* v);

TEXTENGINEDEF void fontScaleMatrix4x4(float* m, float* v);

TEXTENGINEDEF void fontRotateMatrix4x4(float* m, float angle, float* v);

TEXTENGINEDEF void fontCreateOrthographicMatrix(float left, float right, float bottom, float top, float near, float far, float* matrix);

#endif //_TEXT_ENGINE

//============================================= End of Header File =============================================

#ifdef TEXT_ENGINE_IMPLEMENTATION

//============================== If Using Modern OpenGL ==============================

#ifdef TEXT_ENGINE_USE_MODERN_OPENGL

TEXTENGINEDEF Font* createFont(const char* font_name, int size){

	Font* font = (Font*)malloc(sizeof(Font));
	font->size = size;
	font->tab_size = 4;
	font->scale_x = 1.0f, font->scale_y = 1.0f;
	float gl_current_view_port[4];
	glGetFloatv(GL_VIEWPORT,gl_current_view_port);
	font->canvas_width = gl_current_view_port[2];
	font->canvas_height = gl_current_view_port[3];
	font->canvas_depth = 1000000;
	font->depth = 0.0;
	font->color_r = 1.0f;
	font->color_g = 1.0f;
	font->color_b = 1.0f;
	font->color_a = 1.0f;
	font->free_transform = 0;
	
	for(int i = 0; i < 16; i++) font->projection_matrix[i] = 0.0f;
	font->projection_matrix[0] = font->projection_matrix[5] = font->projection_matrix[10] = font->projection_matrix[15] = 1.0f;
	fontCreateOrthographicMatrix(0,font->canvas_width,font->canvas_height,0,-font->canvas_depth,font->canvas_depth,font->projection_matrix);
	
	for(int i = 0; i < 16; i++) font->transform_matrix[i] = 0.0f;
	font->transform_matrix[0] = font->transform_matrix[5] = font->transform_matrix[10] = font->transform_matrix[15] = 1.0f;
	fontCreateOrthographicMatrix(0,font->canvas_width,font->canvas_height,0,-font->canvas_depth,font->canvas_depth,font->transform_matrix);
	
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

	for(int i = 0; i < TEXT_ENGINE_MAX_GLYPHS_COUNT; i++){
	
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
		
		uniform mat4 model;
		uniform mat4 projection;
		
		uniform vec3 position;
		uniform vec2 size;
		
		varying vec2 out_uv;
		
		void main(){
			out_uv = in_uv;
			gl_Position = projection * model * vec4( position.xy + (in_vertex * size) , position.z, 1.0);
		}
		
	)";
	
	const char* fragment_shader_source = R"(
	
		#version 110
		
		varying vec2 out_uv;
		
		uniform sampler2D texture;
		uniform vec4 color;
		
		void main(){
			float a = texture2D(texture,out_uv).a;
			if(a <= 0.0) discard;
			gl_FragColor = color * a;
		}
		
	)";
	
	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader,1,&vertex_shader_source,0);
	glCompileShader(vertex_shader);
	int success;
	char infolog[512];
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
		1.0,0.0,
		1.0,0.0,
		0.0,1.0,
		1.0,1.0
	};
	float uv[] = {
		0.0,0.0,
		0.0,1.0,
		1.0,0.0,
		1.0,0.0,
		0.0,1.0,
		1.0,1.0
	};
	
	glGenVertexArrays(1,&font->vertex_array);
	glBindVertexArray(font->vertex_array);
	
	unsigned int vbo[2];
	glGenBuffers(2,vbo);
	glBindBuffer(GL_ARRAY_BUFFER,vbo[0]);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*12,vertex,GL_STATIC_DRAW);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,0);
	glEnableVertexAttribArray(0);
	
	glBindBuffer(GL_ARRAY_BUFFER,vbo[1]);
	glBufferData(GL_ARRAY_BUFFER,sizeof(float)*12,uv,GL_STATIC_DRAW);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,0);
	glEnableVertexAttribArray(1);
	
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);

	return font;
}

TEXTENGINEDEF void drawText(Font* font, const unsigned char* text, int x, int y){

	glUseProgram(font->shader);
	glBindVertexArray(font->vertex_array);
	
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(font->shader,"texture"),0);
	
	glUniform4f(glGetUniformLocation(font->shader,"color"),font->color_r,font->color_g,font->color_b,font->color_a);
	
	if(!font->free_transform){
		float vector[3] = {x,y,0.0};
		fontIdentityMatrix4x4(font->transform_matrix);
		fontTranslateMatrix4x4(font->transform_matrix,vector);
		vector[0] = font->scale_x, vector[1] = font->scale_y, vector[2] = 1.0;
		fontScaleMatrix4x4(font->transform_matrix,vector);
		vector[0] = -x, vector[1] = -y, vector[2] = 0.0;
		fontTranslateMatrix4x4(font->transform_matrix,vector);
	}
	glUniformMatrix4fv(glGetUniformLocation(font->shader,"model"),1,GL_FALSE,font->transform_matrix);
	glUniformMatrix4fv(glGetUniformLocation(font->shader,"projection"),1,GL_FALSE,font->projection_matrix);

	int is_gltexture2d_active;
	glGetIntegerv(GL_TEXTURE_2D,&is_gltexture2d_active);
	if(!is_gltexture2d_active)
		glEnable(GL_TEXTURE_2D);
		
	int is_gldepthtest_active;
	glGetIntegerv(GL_DEPTH_TEST,&is_gldepthtest_active);
	if(!is_gldepthtest_active)
		glEnable(GL_DEPTH_TEST);
	
	int gldepth_func;
	glGetIntegerv(GL_DEPTH_FUNC,&gldepth_func);
	glDepthFunc(GL_LEQUAL);
		
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
	
	for(unsigned int i = 0; i < strlen((char*)text); i++){
	
		if(text[i] == '\n'){
		
			line += font->size;
			x = initial_x;
			
		}else if(text[i] == '	'){
		
			x += font->letters[31].width * font->tab_size;
			
		}else{
		
			y = line + (initial_y-font->letters[(int)text[i]].top);
			
			glUniform3f(glGetUniformLocation(font->shader,"position"),x,y,font->depth);
			glUniform2f(glGetUniformLocation(font->shader,"size"),font->letters[(int)text[i]].width,font->letters[(int)text[i]].rows);

			glBindTexture(GL_TEXTURE_2D,font->letters[(int)text[i]].texture);
			
			glDrawArrays(GL_TRIANGLES,0,6);
			
			x += font->letters[(int)text[i]].advance>>6;
			
		}
		
	}
	
	if(!is_gldepthtest_active)
		glDisable(GL_DEPTH_TEST);
	
	glDepthFunc(gldepth_func);
	
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

#else //TEXT_ENGINE_USE_MODERN_OPENGL

//============================== If Using OpenGL Compatibility Mode (Imediate Mode) ==============================

TEXTENGINEDEF Font* createFont(const char* font_name, int size){

	Font* font = (Font*)malloc(sizeof(Font));
	font->size = size;
	font->tab_size = 4;
	font->scale_x = 1.0f, font->scale_y = 1.0f;
	float gl_current_view_port[4];
	glGetFloatv(GL_VIEWPORT,gl_current_view_port);
	font->canvas_width = gl_current_view_port[2];
	font->canvas_height = gl_current_view_port[3];
	font->canvas_depth = 1000000;
	font->depth = 0.0;
	font->color_r = 1.0f;
	font->color_g = 1.0f;
	font->color_b = 1.0f;
	font->color_a = 1.0f;
	font->free_transform = 0;
	
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

	for(int i = 0; i < TEXT_ENGINE_MAX_GLYPHS_COUNT; i++){
	
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

TEXTENGINEDEF void drawText(Font* font, const unsigned char* text, int x, int y){

	int matrix_mode;
	glGetIntegerv(GL_MATRIX_MODE,&matrix_mode);
	glMatrixMode(GL_PROJECTION);
	float gl_current_ortho[16];
	glGetFloatv(GL_PROJECTION_MATRIX,gl_current_ortho);
	glLoadIdentity();
	glOrtho(0,font->canvas_width,font->canvas_height,0,-font->canvas_depth,font->canvas_depth);
	glMatrixMode(GL_MODELVIEW);

	int is_gltexture2d_active;
	glGetIntegerv(GL_TEXTURE_2D,&is_gltexture2d_active);
	if(!is_gltexture2d_active)
		glEnable(GL_TEXTURE_2D);
		
	int is_gldepthtest_active;
	glGetIntegerv(GL_DEPTH_TEST,&is_gldepthtest_active);
	if(!is_gldepthtest_active)
		glEnable(GL_DEPTH_TEST);
	
	int gldepth_func;
	glGetIntegerv(GL_DEPTH_FUNC,&gldepth_func);
	glDepthFunc(GL_LEQUAL);
	
	int is_glalphatest_active;
	glGetIntegerv(GL_ALPHA_TEST, &is_glalphatest_active);
	if(!is_glalphatest_active)
		glEnable(GL_ALPHA_TEST);
	
	int glalpha_func;
	glGetIntegerv(GL_ALPHA_TEST_FUNC, &glalpha_func);
	glAlphaFunc(GL_GREATER, 0.0);
		
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
	
	glPushMatrix();
	
	if(!font->free_transform){
		glLoadIdentity();
		glTranslatef(x,y,0);
		glScalef(font->scale_x,font->scale_y,1.0);
		glTranslatef(-x,-y,0);
	}
	
	for(unsigned int i = 0; i < strlen((char*)text); i++){
	
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
				glVertex3i(x, y, font->depth);
				glTexCoord2f(0,1);
				glVertex3i(x, y+font->letters[(int)text[i]].rows, font->depth);
				glTexCoord2f(1,1);
				glVertex3i(x+font->letters[(int)text[i]].width, y+font->letters[(int)text[i]].rows, font->depth);
				glTexCoord2f(1,0);
				glVertex3i(x+font->letters[(int)text[i]].width, y, font->depth);
			glEnd();
			glBindTexture(GL_TEXTURE_2D,0);
			
			x += font->letters[(int)text[i]].advance>>6;
			
		}
		
	}
	
	glPopMatrix();
	
	glColor4f(old_color[0], old_color[1], old_color[2], old_color[3]);
	
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(gl_current_ortho);
	glMatrixMode(matrix_mode);
	
	if(!is_gldepthtest_active)
		glDisable(GL_DEPTH_TEST);
	
	glDepthFunc(gldepth_func);
	
	if(!is_glalphatest_active)
		glDisable(GL_ALPHA_TEST);
		
	glAlphaFunc(glalpha_func, 0.0);
	
	if(temp_gl_blend_src != GL_SRC_ALPHA || temp_gl_blend_dst != GL_ONE_MINUS_SRC_ALPHA)
		glBlendFunc(temp_gl_blend_src,temp_gl_blend_dst);
	
	if(!is_gltexture2d_active)
		glDisable(GL_TEXTURE_2D);
	
	if(!is_glblend_active)
		glDisable(GL_BLEND);

}

#endif //#else TEXT_ENGINE_USE_MODERN_OPENGL

TEXTENGINEDEF void setFontFreeTransform(Font* font, int free_transform){
	font->free_transform = free_transform;
}

TEXTENGINEDEF void setFontDepth(Font* font, float depth){
	font->depth = depth;
}

TEXTENGINEDEF void setFontColor(Font* font, float r, float g, float b, float a){
	font->color_r = r;
	font->color_g = g;
	font->color_b = b;
	font->color_a = a;
}

TEXTENGINEDEF void setFontCanvasSize(Font* font, int width, int height, int depth){
	font->canvas_width = width;
	font->canvas_height = height;
	font->canvas_depth = depth;
	fontCreateOrthographicMatrix(0,font->canvas_width,font->canvas_height,0,-font->canvas_depth,font->canvas_depth,font->projection_matrix);
}

TEXTENGINEDEF void setTabSize(Font* font, const int tab_size){
	font->tab_size = tab_size;
}

TEXTENGINEDEF void setFontScale(Font* font, float scale){
	font->scale_x = scale;
	font->scale_y = scale;
}

TEXTENGINEDEF void setFontScaleInPixels(Font* font, float scale_in_pixels){
	font->scale_x = scale_in_pixels / (float)font->size;
	font->scale_y = scale_in_pixels / (float)font->size;
}

TEXTENGINEDEF int getSizeText(Font* font,const unsigned char* text){

	int max_width = 0;
	int x = 0;
	
	for(unsigned int i = 0; i < strlen((char*)text); i++){
	
		if(text[i] == '\n'){
			if(max_width < x) max_width = x;
			x = 0;
		}else x += font->letters[(int)text[i]].advance>>6;
		
	}
	
	if(max_width < x) max_width = x;
	
	return max_width * font->scale_x;
}

TEXTENGINEDEF int getFontHeight(Font* font){
	
	return font->size * font->scale_y;	
	
}

TEXTENGINEDEF int getTextAlignRight(Font* font, const unsigned char* text, int position_x){
	
	return position_x - getSizeText(font,text);
	
}

TEXTENGINEDEF int getTextAlignCenter(Font* font, const unsigned char* text, int position_x){
	
	return position_x - (getSizeText(font,text) * 0.5);
	
}


//============================== Internal Math Functions ==============================

TEXTENGINEDEF void fontMultiplyMatrix4x4(float* m1, float* m2, float* dest){
	
	float a0 = m1[0], a1 = m1[1], a2 = m1[2], a3 = m1[3],
	a4 = m1[4], a5 = m1[5], a6 = m1[6], a7 = m1[7],
	a8 = m1[8], a9 = m1[9], a10 = m1[10], a11 = m1[11],
	a12 = m1[12], a13 = m1[13], a14 = m1[14], a15 = m1[15],

	b0 = m2[0], b1 = m2[1], b2 = m2[2], b3 = m2[3],
	b4 = m2[4], b5 = m2[5], b6 = m2[6], b7 = m2[7],
	b8 = m2[8], b9 = m2[9], b10 = m2[10], b11 = m2[11],
	b12 = m2[12], b13 = m2[13], b14 = m2[14], b15 = m2[15];

	dest[0] = a0 * b0 + a4 * b1 + a8 * b2 + a12 * b3;
	dest[1] = a1 * b0 + a5 * b1 + a9 * b2 + a13 * b3;
	dest[2] = a2 * b0 + a6 * b1 + a10 * b2 + a14 * b3;
	dest[3] = a3 * b0 + a7 * b1 + a11 * b2 + a15 * b3;

	dest[4] = a0 * b4 + a4 * b5 + a8 * b6 + a12 * b7;
	dest[5] = a1 * b4 + a5 * b5 + a9 * b6 + a13 * b7;
	dest[6] = a2 * b4 + a6 * b5 + a10 * b6 + a14 * b7;
	dest[7] = a3 * b4 + a7 * b5 + a11 * b6 + a15 * b7;

	dest[8] = a0 * b8 + a4 * b9 + a8 * b10 + a12 * b11;
	dest[9] = a1 * b8 + a5 * b9 + a9 * b10 + a13 * b11;
	dest[10] = a2 * b8 + a6 * b9 + a10 * b10 + a14 * b11;
	dest[11] = a3 * b8 + a7 * b9 + a11 * b10 + a15 * b11;

	dest[12] = a0 * b12 + a4 * b13 + a8 * b14 + a12 * b15;
	dest[13] = a1 * b12 + a5 * b13 + a9 * b14 + a13 * b15;
	dest[14] = a2 * b12 + a6 * b13 + a10 * b14 + a14 * b15;
	dest[15] = a3 * b12 + a7 * b13 + a11 * b14 + a15 * b15;
		
}

TEXTENGINEDEF void fontIdentityMatrix4x4(float* m){
	m[0] = 1.0, m[1] = 0.0, m[2] = 0.0, m[3] = 0.0,
	m[4] = 0.0, m[5] = 1.0, m[6] = 0.0, m[7] = 0.0,
	m[8] = 0.0, m[9] = 0.0, m[10] = 1.0, m[11] = 0.0,
	m[12] = 0.0, m[13] = 0.0, m[14] = 0.0, m[15] = 1.0;
}

TEXTENGINEDEF void fontTranslateMatrix4x4(float* m, float* v){
	
	float r[] = {
		1.0,0.0,0.0,0.0,
		0.0,1.0,0.0,0.0,
		0.0,0.0,1.0,0.0,
		v[0],v[1],v[2],1.0
	};
	
	fontMultiplyMatrix4x4(m,r,m);
	
}

TEXTENGINEDEF void fontScaleMatrix4x4(float* m, float* v){

	float r[] = {
		v[0],0.0,0.0,0.0,
		0.0,v[1],0.0,0.0,
		0.0,0.0,v[2],0.0,
		0.0,0.0,0.0,1.0
	};
	
	fontMultiplyMatrix4x4(m,r,m);
	
}

TEXTENGINEDEF void fontRotateMatrix4x4(float* m, float angle, float* v){

	float c = cosf(angle);
	float s = sinf(angle);
	float t = (1.0f - c);
	
	float r[] = {
		1.0,0.0,0.0,0.0,
		0.0,1.0,0.0,0.0,
		0.0,0.0,1.0,0.0,
		0.0,0.0,0.0,1.0
	};

	float lenght = sqrtf( (v[0]*v[0]) + (v[1]*v[1]) + (v[2]*v[2]) );
	
	v[0] = v[0] / lenght;
	
	r[0] = c + (v[0]*v[0]) * t;
	r[1] = t * v[0] * v[1] + s * v[2];
	r[2] = t * v[0] * v[2] - s * v[1];
	
	r[4] = t * v[0] * v[1] - s * v[2];
	r[5] = t * (v[1]*v[1]) + c;
	r[6] = t * v[1] * v[2] + s * v[0];
	
	r[8] = t * v[0] * v[2] + s * v[1];
	r[9] = t * v[1] * v[2] - s * v[0];
	r[10] = t * (v[2]*v[2]) + c;
	
	fontMultiplyMatrix4x4(m,r,m);

}

TEXTENGINEDEF void fontCreateOrthographicMatrix(float left, float right, float bottom, float top, float near, float far, float* matrix){

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

#endif //TEXT_ENGINE_IMPLEMENTATION

