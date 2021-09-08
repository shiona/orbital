
#ifdef __MINGW32__
#define _GLIBCXX_USE_CXX11_ABI 0
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtx/closest_point.hpp>

#include <SFML/Graphics/Image.hpp>

#include <iostream>

#include "models.hh"


bool Model::setTexture(const std::string& tex_file_name)
{
	sf::Image img_data;

	if(!img_data.loadFromFile(tex_file_name)) {
		printf("Could not load texture '%s'\n", tex_file_name.c_str());
		return false;
	}
	glGenTextures(1, &texture_handle_);
	glBindTexture(GL_TEXTURE_2D, texture_handle_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_data.getSize().x, img_data.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data.getPixelsPtr());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return true;
}

void Model::setShader(GLuint shader_program)
{
	this->shader_program_ = shader_program;
}

Tower::Tower() {
	GLuint vert_count = 4*3;
	std::vector<GLfloat> vertArray(vert_count*3);
	std::vector<GLfloat> texArray(vert_count*2);

	for(int side = 0; side < 4; side++) {
		// top
		vertArray[side*3*3+0] = 0.0f;
		vertArray[side*3*3+1] = 1.0f;
		vertArray[side*3*3+2] = 0.0f;

		texArray[side*3*2 +0] = 1 - (0.125f + (float)side*0.25f);
		texArray[side*3*2 +1] = 0.0f;

		texArray[side*3*2 + 1*2 +0] = 1 - (float)side*0.25f;
		texArray[side*3*2 + 1*2 +1] = 1.0f;

		texArray[side*3*2 + 2*2 +0] = 1 - (float)(side+1)*0.25f;
		texArray[side*3*2 + 2*2 +1] = 1.0f;

		GLfloat sign_hack_to_fix_winding = 1.0f;
		sign_hack_to_fix_winding= side == 1 || side == 2? -1.0f: 1.0f;

		if(side % 2) {
			// change on x
			vertArray[side*3*3 + 1*3 +0] = -0.2f * sign_hack_to_fix_winding;
			vertArray[side*3*3 + 2*3 +0] = +0.2f * sign_hack_to_fix_winding;
			// static z on this side
			vertArray[side*3*3 + 1*3 +2] = vertArray[side*3*3 + 2*3 +2] = 0.4f * (side < 2) -0.2f;
			// y is zero
			vertArray[side*3*3 + 1*3 +1] = vertArray[side*3*3 + 2*3 +1] = 0.0f;
		} else {
			// change on z
			vertArray[side*3*3 + 1*3 +2] = -0.2f * sign_hack_to_fix_winding;
			vertArray[side*3*3 + 2*3 +2] = +0.2f * sign_hack_to_fix_winding;
			// static x on this side
			vertArray[side*3*3 + 1*3 +0] = vertArray[side*3*3 + 2*3 +0] = 0.4f * (side < 2) -0.2f;
			// y is zero
			vertArray[side*3*3 + 1*3 +1] = vertArray[side*3*3 + 2*3 +1] = 0.0f;
		}
	}

	/*
	   for(int i = 0; i < vert_count; i++) {
	   printf(" %0.2f, %0.2f, %0.2f\n", vertArray[i*3], vertArray[i*3+1], vertArray[i*3+2]);
	   }
	   for(int i = 0; i < vert_count; i++) {
	   printf(" %0.2f, %0.2f\n", texArray[i*2], texArray[i*2+1]);
	   }
	   */

	vert_vbo_ = 0;
	glGenBuffers(1, &vert_vbo_);
	glBindBuffer (GL_ARRAY_BUFFER, vert_vbo_);
	glBufferData (GL_ARRAY_BUFFER, 3*vert_count*sizeof(float), &vertArray[0], GL_STATIC_DRAW);

	// Vertex texture uv-coordinate attribute vbo;
	tex_vbo_ = 1;
	glGenBuffers(1, &tex_vbo_);
	glBindBuffer (GL_ARRAY_BUFFER, tex_vbo_);
	glBufferData (GL_ARRAY_BUFFER, 2*vert_count*sizeof(float), &texArray[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glBindBuffer(GL_ARRAY_BUFFER, vert_vbo_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, tex_vbo_);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	// Unbind vao, (so changes to vbos don't matter ?)
	glBindVertexArray(0);
}

/*
   bool Tower::setTexture(const std::string& tex_file_name) {
   sf::Image img_data;

   if(!img_data.loadFromFile(tex_file_name)) {
   printf("Could not load texture '%s'\n");
   return false;
   }
   glGenTextures(1, &texture_handle_);
   glBindTexture(GL_TEXTURE_2D, texture_handle_);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_data.getSize().x, img_data.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data.getPixelsPtr());
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

   return true;
   }
   */

void Tower::draw(const Position& pos) {

	glUseProgram (this->shader_program_);

	GLuint model_handle = glGetUniformLocation(this->shader_program_, "model_mat");
	glUniformMatrix4fv(model_handle, 1, GL_FALSE, &pos.trans_[0][0]);

	glBindTexture(GL_TEXTURE_2D, texture_handle_);

	glBindVertexArray(vao_);
	glDrawArrays (GL_TRIANGLES, 0, 3*4);
	glBindVertexArray(0);
}


Satellite::Satellite() {
	//sides   top&bot  panels
	GLuint vert_count = 2*(4+1)  +  2*4   +  4;

	std::vector<GLfloat> vertArray(vert_count*3);
	std::vector<GLfloat> texArray(vert_count*2);

	// Sides
	for(int corner = 0; corner < 4+1; corner++) {
		int i = corner % 4;
		float x = (i == 0 || i == 3) ? -0.1f : 0.1f;
		float z = (i == 0 || i == 1) ? -0.1f : 0.1f;

		vertArray[corner*2*3 + 0] = vertArray[corner*2*3 + 3] = x;
		vertArray[corner*2*3 + 1] = -0.1f;
		vertArray[corner*2*3 + 4] = 0.1f;
		vertArray[corner*2*3 + 2] = vertArray[corner*2*3 + 5] = z;
	}
	// Top x coords
	vertArray[5*2*3 + 0*3 + 0] = vertArray[5*2*3 + 3*3 + 0] = -0.1f;
	vertArray[5*2*3 + 1*3 + 0] = vertArray[5*2*3 + 2*3 + 0] = 0.1f;
	// Top y coords
	vertArray[5*2*3 + 0*3 + 1] = vertArray[5*2*3 + 1*3 + 1] = vertArray[5*2*3 + 2*3 + 1] = vertArray[5*2*3 + 3*3 + 1] = 0.1f;
	// Top z coords
	vertArray[5*2*3 + 0*3 + 2] = vertArray[5*2*3 + 1*3 + 2] = -0.1f;
	vertArray[5*2*3 + 2*3 + 2] = vertArray[5*2*3 + 3*3 + 2] = 0.1f;

	// Bottom x coords
	vertArray[5*2*3 + 4*3 + 0] = vertArray[5*2*3 + 7*3 + 0] = -0.1f;
	vertArray[5*2*3 + 5*3 + 0] = vertArray[5*2*3 + 6*3 + 0] = 0.1f;
	// Bottom y coords
	vertArray[5*2*3 + 4*3 + 1] = vertArray[5*2*3 + 5*3 + 1] = vertArray[5*2*3 + 6*3 + 1] = vertArray[5*2*3 + 7*3 + 1] = -0.1f;
	// Bottom z coords, reversed so winding matches when viewed from outside
	vertArray[5*2*3 + 4*3 + 2] = vertArray[5*2*3 + 5*3 + 2] = 0.1f;
	vertArray[5*2*3 + 6*3 + 2] = vertArray[5*2*3 + 7*3 + 2] = -0.1f;

	// Panel x coords
	vertArray[18*3 + 0*3 + 0] = vertArray[18*3 + 3*3 + 0] = -1.0f;
	vertArray[18*3 + 1*3 + 0] = vertArray[18*3 + 2*3 + 0] = 1.0f;
	// Panel y coords
	vertArray[18*3 + 0*3 + 1] = vertArray[18*3 + 1*3 + 1] = vertArray[18*3 + 2*3 + 1] = vertArray[18*3 + 3*3 + 1] = 0.0f;
	// Panel z coords
	vertArray[18*3 + 0*3 + 2] = vertArray[18*3 + 1*3 + 2] = -0.25f;
	vertArray[18*3 + 2*3 + 2] = vertArray[18*3 + 3*3 + 2] = 0.25f;


	for(int i = 0; i < vert_count *2; i++) {
		texArray[i] = 0.0f;
	}

	// Panel UVs
	texArray[18*2 + 0*2 + 0] = 0.0f;
	texArray[18*2 + 0*2 + 1] = 0.51f;
	texArray[18*2 + 1*2 + 0] = 1.0f;
	texArray[18*2 + 1*2 + 1] = 0.51f;
	texArray[18*2 + 2*2 + 0] = 1.0f;
	texArray[18*2 + 2*2 + 1] = 0.99f;
	texArray[18*2 + 3*2 + 0] = 0.0f;
	texArray[18*2 + 3*2 + 1] = 0.99f;

	vert_vbo_ = 0;
	glGenBuffers(1, &vert_vbo_);
	glBindBuffer (GL_ARRAY_BUFFER, vert_vbo_);
	glBufferData (GL_ARRAY_BUFFER, 3*vert_count*sizeof(float), &vertArray[0], GL_STATIC_DRAW);

	// Vertex texture uv-coordinate attribute vbo;
	tex_vbo_ = 1;
	glGenBuffers(1, &tex_vbo_);
	glBindBuffer (GL_ARRAY_BUFFER, tex_vbo_);
	glBufferData (GL_ARRAY_BUFFER, 2*vert_count*sizeof(float), &texArray[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glBindBuffer(GL_ARRAY_BUFFER, vert_vbo_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, tex_vbo_);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	// Unbind vao, (so changes to vbos don't matter ?)
	glBindVertexArray(0);
}

/*
   bool Satellite::setTexture(const std::string& tex_file_name) {
   sf::Image img_data;

   if(!img_data.loadFromFile(tex_file_name)) {
   printf("Could not load texture '%s'\n", tex_file_name.c_str());
   return false;
   }
   glGenTextures(1, &texture_handle_);
   glBindTexture(GL_TEXTURE_2D, texture_handle_);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_data.getSize().x, img_data.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data.getPixelsPtr());
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

   return true;
   }
   */

//void draw(GLfloat lat, GLfloat lon, GLfloat alt) {
void Satellite::draw(const Position& pos) {
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	glUseProgram (this->shader_program_);

	// Set model transformation matrix. For planet it's identity.
	//glm::mat4 model_mat = trans_latlonalt(lat, lon, alt);
	//glUniformMatrix4fv(model_handle, 1, GL_FALSE, &model_mat[0][0]);
	GLuint model_handle = glGetUniformLocation(this->shader_program_, "model_mat");
	glUniformMatrix4fv(model_handle, 1, GL_FALSE, &pos.trans_[0][0]);

	glBindTexture(GL_TEXTURE_2D, texture_handle_);

	glBindVertexArray(vao_);

	// Sides
	glDrawArrays (GL_QUAD_STRIP, 0, 5*2);
	// Top, bottom and panel
	glDrawArrays (GL_QUADS, 5*2, 12);

	glBindVertexArray(0);
}

Connections::Connections(const Position& start, const Position& end,
		const std::vector<Position>& satellite_positions) {

	sf::Image img_data;

	if(!img_data.loadFromFile("conn_tex.png")) {
		printf("Could not load texture for conns\n");
	}
	glGenTextures(1, &texture_handle_);
	glBindTexture(GL_TEXTURE_2D, texture_handle_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_data.getSize().x, img_data.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data.getPixelsPtr());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	std::vector<glm::vec3> vertArray;

	std::cout<<"end.pos_.x: "<<end.pos_.x<<"\n";

	vertArray.push_back(start.pos_);
	vertArray.push_back(end.pos_);

	for(auto &pos : satellite_positions) {
		vertArray.push_back(pos.pos_);
	}

	std::vector<GLuint> indices;

	// Surface to satellite links
	for(int i = 0; i < 2; i++) {
		for(int j = 2; j < vertArray.size(); j++) {
			if(glm::dot(vertArray[i], vertArray[j]-vertArray[i]) > 0) {
				indices.push_back(i);
				indices.push_back(j);
				conn_count_++;
				//std::cout<<i<<" - " <<j<<"\n";
			} else {
				//std::cout<<i<<" | " <<j<<"\n";
			}
		}
	}

	// Intersatellite links
	for(int i = 2; i < vertArray.size()-1; i++) {
		for(int j = i+1; j < vertArray.size(); j++) {
			glm::vec3 closest_point = glm::closestPointOnLine(glm::vec3(0.0f), vertArray[i], vertArray[j]);
			if(glm::distance(closest_point, glm::vec3(0.0f)) > 1.0f ||
					glm::distance(closest_point, vertArray[i]) > glm::distance(vertArray[i], vertArray[j]) ||
					glm::distance(closest_point, vertArray[j]) > glm::distance(vertArray[i], vertArray[j])) {
				indices.push_back(i);
				indices.push_back(j);
				conn_count_++;
				//std::cout<<i<<" - " <<j<<"\n";
			} else {
				//std::cout<<i<<" | " <<j<<"\n";
			}
		}
	}

	// After checking visibility, push vertices at ground stations up to the tower
	// and bring points from satellites down:
	/*
	   for(int i = 0; i < 2; i++) {
	   vertArray[i] *= 1.07f;
	   }
	   */


	vert_vbo_ = 0;
	glGenBuffers(1, &vert_vbo_);
	glBindBuffer (GL_ARRAY_BUFFER, vert_vbo_);
	glBufferData (GL_ARRAY_BUFFER, 3*vertArray.size()*sizeof(float), &vertArray[0], GL_STATIC_DRAW);

	glGenBuffers(1, &ibo_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glBindBuffer(GL_ARRAY_BUFFER, vert_vbo_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
}

void Connections::draw() {
	glUseProgram (this->shader_program_);
	glBindTexture(GL_TEXTURE_2D, texture_handle_);

	// Set model transformation matrix. For planet it's identity.
	//glm::mat4 model_mat = trans_latlonalt(lat, lon, alt);
	//glUniformMatrix4fv(model_handle, 1, GL_FALSE, &model_mat[0][0]);
	GLuint model_handle = glGetUniformLocation(this->shader_program_, "model_mat");
	glUniformMatrix4fv(model_handle, 1, GL_FALSE, &glm::mat4(1.0f)[0][0]);

	glBindVertexArray(vao_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
	glDrawElements(GL_LINES, conn_count_*2, GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);

}


Sphere::Sphere(GLuint hor_slices, GLuint ver_slices, float radius) {
	hor_slices_ = hor_slices;
	ver_slices_ = ver_slices;

	GLuint vert_count = hor_slices_ * (ver_slices_+1) * 2;

	std::vector<GLfloat> vertArray(vert_count*3);
	std::vector<GLfloat> texArray(vert_count*2);

	for(int y = 0; y < hor_slices_; y++) {
		float height1 = radius * cos(((float)y/(float)hor_slices_)*PI);
		float radius1 = radius * sin(((float)y/(float)hor_slices_)*PI);
		float height2 = radius * cos(((float)(y+1)/(float)hor_slices_)*PI);
		float radius2 = radius * sin(((float)(y+1)/(float)hor_slices_)*PI);

		//printf("ring:\n");
		for(int s = 0; s <= ver_slices_; s++) {
			float cylinderx = sin(2*PI * ((float)s/(float)ver_slices_));
			float cylindery = cos(2*PI * ((float)s/(float)ver_slices_));
			//printf("%f, %f, %f\n", height1, radius1 * cylinderx, radius1 * cylindery);
			//printf("%f, %f, %f\n", height2, radius2 * cylinderx, radius2 * cylindery);
			vertArray[y*(ver_slices_+1)*6 + s*6 + 0] = radius1 * cylinderx;
			vertArray[y*(ver_slices_+1)*6 + s*6 + 1] = height1;
			vertArray[y*(ver_slices_+1)*6 + s*6 + 2] = radius1 * cylindery;
			vertArray[y*(ver_slices_+1)*6 + s*6 + 3] = radius2 * cylinderx;
			vertArray[y*(ver_slices_+1)*6 + s*6 + 4] = height2;
			vertArray[y*(ver_slices_+1)*6 + s*6 + 5] = radius2 * cylindery;

			texArray[y*(ver_slices_+1)*4 + s*4 + 0] = (float)s/(float)ver_slices_ + 0.5f;
			//texArray[y*(ver_slices_+1)*4 + s*4 + 1] = 0.5f - height1/2.0f;
			texArray[y*(ver_slices_+1)*4 + s*4 + 1] = (float)y/(float)hor_slices_;
			texArray[y*(ver_slices_+1)*4 + s*4 + 2] = (float)s/(float)ver_slices_ + 0.5f;
			//texArray[y*(ver_slices_+1)*4 + s*4 + 3] = 0.5f - height2/2.0f;
			texArray[y*(ver_slices_+1)*4 + s*4 + 3] = (float)(y+1)/(float)hor_slices_;
		}
		/*float cylinderx = sin(2*PI);
		  float cylindery = cos(2*PI);
		  printf("%f, %f, %f\n", height1, radius1 * cylinderx, radius1 * cylindery);
		  printf("%f, %f, %f\n", height2, radius2 * cylinderx, radius2 * cylindery);
		  */
	}

	// Vertex position attribute vbo;
	vert_vbo_ = 0;
	glGenBuffers(1, &vert_vbo_);
	glBindBuffer (GL_ARRAY_BUFFER, vert_vbo_);
	glBufferData (GL_ARRAY_BUFFER, 3*vert_count*sizeof(float), &vertArray[0], GL_STATIC_DRAW);

	// Vertex texture uv-coordinate attribute vbo;
	tex_vbo_ = 1;
	glGenBuffers(1, &tex_vbo_);
	glBindBuffer (GL_ARRAY_BUFFER, tex_vbo_);
	glBufferData (GL_ARRAY_BUFFER, 2*vert_count*sizeof(float), &texArray[0], GL_STATIC_DRAW);

	// Sphere vao, attr 0 is vertex position, attr 1 is uv coordinates
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glBindBuffer(GL_ARRAY_BUFFER, vert_vbo_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, tex_vbo_);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	// Unbind vao, (so changes to vbos don't matter ?)
	glBindVertexArray(0);
}



bool Sphere::setTexture(const std::string& tex_file_name) {
	sf::Image img_data;

	GLint max_tex_size = -1;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_size);
	printf("Max tex size: %d\n", max_tex_size);

	if(!img_data.loadFromFile(tex_file_name)) {
		printf("Could not load texture '%s'\n", tex_file_name.c_str());
		return false;
	}
	GLuint h = img_data.getSize().y;
	GLuint w = img_data.getSize().x;
	printf("Earth tex mid point alpha from img_data: %d\n", img_data.getPixel(w/2, h/2).a);
	printf("Earth tex mid point alpha from buffer: %d\n", img_data.getPixelsPtr()[4*((h/2)*w + w/2) + 3]);
	glGenTextures(1, &texture_handle_);
	printf("a: %d\n", glGetError());
	glBindTexture(GL_TEXTURE_2D, texture_handle_);
	printf("b: %d\n", glGetError());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_data.getSize().x, img_data.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data.getPixelsPtr());
	printf("c: %d\n", glGetError());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return true;
}

void Sphere::draw() {
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glPolygonMode( GL_FRONT, GL_FILL );

	glUseProgram (this->shader_program_);

	// Set model transformation matrix. For planet it's identity.
	glm::mat4 model_mat = glm::mat4(1.0f);
	GLuint model_handle = glGetUniformLocation(this->shader_program_, "model_mat");
	glUniformMatrix4fv(model_handle, 1, GL_FALSE, &model_mat[0][0]);

	glBindTexture(GL_TEXTURE_2D, texture_handle_);

	glBindVertexArray(vao_);
	for(int i = 0; i < hor_slices_; i++) { // For every horizontal slice
		glDrawArrays (GL_TRIANGLE_STRIP, i*(2*(ver_slices_+1)), 2*(ver_slices_+1));
	}
	glBindVertexArray(0);
}
