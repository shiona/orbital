#ifdef __MINGW32__
#define _GLIBCXX_USE_CXX11_ABI 0
#endif

//#define GL_GLEXT_PROTOTYPES
//#include <GL/glext.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <gtx/rotate_vector.hpp>
#include <gtx/closest_point.hpp>

#include <SFML/Window.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/OpenGL.hpp>

#include <iostream>
#include <fstream>

const double PI = 3.141592654;

#define WIDTH (1200)
#define HEIGHT (1000)

GLuint shader_programme;
GLuint view_handle;
GLuint model_handle;

struct Position
{
		glm::vec3 pos_;
		glm::mat4 trans_;

		Position(GLfloat lat, GLfloat lon, GLfloat alt) {
			alt = alt/6371.0f;
			trans_ = glm::scale(
				glm::translate(
					glm::rotate(
						glm::rotate(
							glm::mat4(1.0f),
							(float)(lon/180.0f*PI),
							glm::vec3(0.0f, 1.0f, 0.0f)
						),
						(float)((90.0f-lat)/180.0f*PI),
						glm::vec3(1.0f, 0.0f, 0.0f)
					),
					glm::vec3(0.0f, 0.99f+alt, 0.0f)
				),
				glm::vec3(0.1f)
			);
			double cosLat = cos(lat*M_PI/180);
			double sinLat = sin(lat*M_PI/180);
			double cosLon = cos(lon*M_PI/180);
			double sinLon = sin(lon*M_PI/180);
			double h = alt;
			pos_.z = (1+h) * cosLat * cosLon;
			pos_.x = (1+h) * cosLat * sinLon;
			pos_.y = (1+h) * sinLat;
		}
};

class Tower
{
	private:
		GLuint texture_handle_;

		GLuint vert_vbo_;
		GLuint tex_vbo_;
		GLuint vao_;

	public:

		Tower() {
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

		bool setTexture(const std::string& tex_file_name) {
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

		void draw(const Position& pos) {

			glUseProgram (shader_programme);

			glUniformMatrix4fv(model_handle, 1, GL_FALSE, &pos.trans_[0][0]);

			glBindTexture(GL_TEXTURE_2D, texture_handle_);

			glBindVertexArray(vao_);
			glDrawArrays (GL_TRIANGLES, 0, 3*4);
			glBindVertexArray(0);
		}
};

class Satellite
{
	private:
		GLuint texture_handle_;

		GLuint vert_vbo_;
		GLuint tex_vbo_;
		GLuint vao_;

	public:

		Satellite() {
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

		bool setTexture(const std::string& tex_file_name) {
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

		//void draw(GLfloat lat, GLfloat lon, GLfloat alt) {
		void draw(const Position& pos) {
			//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

			glUseProgram (shader_programme);

			// Set model transformation matrix. For planet it's identity.
			//glm::mat4 model_mat = trans_latlonalt(lat, lon, alt);
			//glUniformMatrix4fv(model_handle, 1, GL_FALSE, &model_mat[0][0]);
			glUniformMatrix4fv(model_handle, 1, GL_FALSE, &pos.trans_[0][0]);

			glBindTexture(GL_TEXTURE_2D, texture_handle_);

			glBindVertexArray(vao_);

			// Sides
			glDrawArrays (GL_QUAD_STRIP, 0, 5*2);
			// Top, bottom and panel
			glDrawArrays (GL_QUADS, 5*2, 12);

			glBindVertexArray(0);
		}
};

class Connections
{
	private:
		GLuint vert_vbo_;
		GLuint vao_;
		GLuint ibo_;

		GLuint conn_count_;

		GLuint texture_handle_;

	public:
		Connections(const Position& start, const Position& end,
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

		void draw() {
			glUseProgram (shader_programme);
			glBindTexture(GL_TEXTURE_2D, texture_handle_);

			// Set model transformation matrix. For planet it's identity.
			//glm::mat4 model_mat = trans_latlonalt(lat, lon, alt);
			//glUniformMatrix4fv(model_handle, 1, GL_FALSE, &model_mat[0][0]);
			glUniformMatrix4fv(model_handle, 1, GL_FALSE, &glm::mat4(1.0f)[0][0]);

			glBindVertexArray(vao_);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
			glDrawElements(GL_LINES, conn_count_*2, GL_UNSIGNED_INT, (void*)0);
			glBindVertexArray(0);

		}
};


class Sphere
{
	private:
		GLuint texture_handle_;

		GLuint vert_vbo_;
		GLuint tex_vbo_;
		GLuint vao_;

		GLuint ver_slices_;
		GLuint hor_slices_;

	public:

		Sphere(GLuint hor_slices, GLuint ver_slices, float radius) {
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


		bool setTexture(const std::string& tex_file_name) {
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

		void draw() {
			//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			glPolygonMode( GL_FRONT, GL_FILL );

			glUseProgram (shader_programme);

			// Set model transformation matrix. For planet it's identity.
			glm::mat4 model_mat = glm::mat4(1.0f);
			glUniformMatrix4fv(model_handle, 1, GL_FALSE, &model_mat[0][0]);

			glBindTexture(GL_TEXTURE_2D, texture_handle_);

			glBindVertexArray(vao_);
			for(int i = 0; i < hor_slices_; i++) { // For every horizontal slice
				glDrawArrays (GL_TRIANGLE_STRIP, i*(2*(ver_slices_+1)), 2*(ver_slices_+1));
			}
			glBindVertexArray(0);
		}
};

bool init()
{
	glewInit();

	glEnable(GL_MULTISAMPLE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glClearColor(0.01f,0.05f,0.1,1.0f);

	const char* vertex_shader =
		"#version 130\n"
		//"in vec3 vp;"
		"uniform mat4 proj_mat;"
		"uniform mat4 model_mat;"
		"uniform mat4 view_mat;"
		//"layout(location = 0) in vec3 vp;"
		//"layout(location = 1) in vec2 tex;"
		"in vec3 vp;"
		"in vec2 tex;"
		"out vec2 tex_coord;"
		"void main () {"
		"  tex_coord = tex;"
		"  gl_Position = proj_mat * view_mat * model_mat * vec4 (vp, 1.0);"
		//"  gl_Position = mat4(1.0) * vec4 (vp, 1.0);"
		//"  gl_Position = vec4 (vp, 1.0);"
		//"  gl_Position = vec4 (MVP * vp, 1.0);"
		//"  gl_Position = vec4 (vp, 1.0);"
		"}";

	const char* fragment_shader =
		"#version 130\n"
		"in vec2 tex_coord;"
		"out vec4 frag_colour;"
		"uniform sampler2D tex_sampler;"
		"void main () {"
		"  vec4 texel = texture(tex_sampler, tex_coord);"
		"  if(texel.a < 0.5) { discard; } "
		"  frag_colour = texel; "
		"}";

	GLuint vs = glCreateShader (GL_VERTEX_SHADER);
	glShaderSource (vs, 1, &vertex_shader, NULL);
	glCompileShader (vs);
	GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (fs, 1, &fragment_shader, NULL);
	glCompileShader (fs);

	GLint compileStatus;
	GLchar errorLog[512];
	glGetObjectParameterivARB(vs,GL_OBJECT_COMPILE_STATUS_ARB,&compileStatus);
	if (!compileStatus){
		fprintf(stderr, "VS Compilation error\n");
		glGetShaderInfoLog(vs, 512, NULL, errorLog);
		printf("%s\n", errorLog);
	}
	glGetObjectParameterivARB(fs,GL_OBJECT_COMPILE_STATUS_ARB,&compileStatus);
	if (!compileStatus){
		fprintf(stderr, "FS Compilation error\n");
		glGetShaderInfoLog(fs, 512, NULL, errorLog);
		printf("%s\n", errorLog);
	}

	shader_programme = glCreateProgram ();
	glAttachShader (shader_programme, fs);
	glAttachShader (shader_programme, vs);
	glLinkProgram (shader_programme);

	glGetObjectParameterivARB(shader_programme,GL_OBJECT_LINK_STATUS_ARB,&compileStatus);
	if (!compileStatus){
		fprintf(stderr, "Link error\n");
	}

	GLuint proj_handle = glGetUniformLocation(shader_programme, "proj_mat");
	view_handle = glGetUniformLocation(shader_programme, "view_mat");
	model_handle = glGetUniformLocation(shader_programme, "model_mat");
	if(proj_handle == -1 || view_handle == -1) {
		printf("Illegal uniform:\n");
	}
	glUseProgram (shader_programme);
	// Set projection matrix. View matrix is set in main loop and model matrices on every object draw.
	glm::mat4 proj_mat = glm::perspective(glm::radians(30.0f), (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);
	glUniformMatrix4fv(proj_handle, 1, GL_FALSE, &proj_mat[0][0]);
}

int main()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 8;
	settings.majorVersion = 3;
	settings.minorVersion = 0;

	GLuint window_width = WIDTH;
	GLuint window_height = HEIGHT;

	sf::Window window (sf::VideoMode(window_width, window_height), "Orbital Challenge", sf::Style::Default, settings);

	settings = window.getSettings();
	window.setVerticalSyncEnabled(true);

	std::cout << "depth bits:" << settings.depthBits << std::endl;
	std::cout << "stencil bits:" << settings.stencilBits << std::endl;
	std::cout << "antialiasing level:" << settings.antialiasingLevel << std::endl;
	std::cout << "version:" << settings.majorVersion << "." << settings.minorVersion << std::endl;

	init();

	bool running = true;

	GLfloat theta = 0.0f;
	GLfloat sigma = 0.0f;


	std::vector<Position> satellite_positions;
	std::vector<Position> ground_station_positions;

	{
		std::cout<<"Opening file\n";
		std::ifstream datafile("origdata");
		std::string tmp;
		getline(datafile, tmp);

		window.setTitle("Orbital Challenge, seed: " + std::string(tmp.c_str()+7));

		GLfloat lat, lon, alt;

		while( getline(datafile, tmp) ) {
			if(tmp.compare(0, 3, "SAT") == 0) {
				std::size_t comma_pos = tmp.find_first_of(',');
				//std::cout<<"##"<<tmp.substr(comma_pos+1)<<"##\n";
				lat = std::stof(tmp.substr(comma_pos+1));

				comma_pos = tmp.find_first_of(',',comma_pos+1);
				//std::cout<<"##"<<tmp.substr(comma_pos+1)<<"##\n";
				lon = std::stof(tmp.substr(comma_pos+1));

				comma_pos = tmp.find_first_of(',',comma_pos+1);
				//std::cout<<"##"<<tmp.substr(comma_pos+1)<<"##\n";
				alt = std::stof(tmp.substr(comma_pos+1));
				//std::cout<<lat << " " << lon << " " << alt <<"\n";

				satellite_positions.push_back(Position(lat, lon, alt));
			} else if(tmp.compare(0,5, "ROUTE") == 0) {
				std::size_t comma_pos = tmp.find_first_of(',');
				//std::cout<<"##"<<tmp.substr(comma_pos+1)<<"##\n";
				lat = std::stof(tmp.substr(comma_pos+1));

				comma_pos = tmp.find_first_of(',',comma_pos+1);
				//std::cout<<"##"<<tmp.substr(comma_pos+1)<<"##\n";
				lon = std::stof(tmp.substr(comma_pos+1));

				ground_station_positions.push_back(Position(lat,lon,0));

				comma_pos = tmp.find_first_of(',',comma_pos+1);
				//std::cout<<"##"<<tmp.substr(comma_pos+1)<<"##\n";
				lat = std::stof(tmp.substr(comma_pos+1));

				comma_pos = tmp.find_first_of(',',comma_pos+1);
				//std::cout<<"##"<<tmp.substr(comma_pos+1)<<"##\n";
				lon = std::stof(tmp.substr(comma_pos+1));

				ground_station_positions.push_back(Position(lat,lon,0));
			}
		}
	}

	Sphere earth = Sphere(36, 56, 1.0f);
	earth.setTexture("earth_tex.jpg");
	std::cout<<"Planet done\n";

	Tower tower;
	tower.setTexture("tower_tex_transparent.png");
	std::cout<<"Tower done\n";

	Satellite sat;
	sat.setTexture("satellite_tex_transparent.png");
	std::cout<<"Satellite done\n";

	Connections conns = Connections(ground_station_positions[0], ground_station_positions[1], satellite_positions);

	sf::Clock fps_clock;
	GLuint frame_counter = 0;

	while (running)
	{
		// handle events
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				// end the program
				running = false;
			}
			else if (event.type == sf::Event::Resized)
			{
				window_width = event.size.width;
				window_height = event.size.height;
				// adjust the viewport when the window is resized
				glViewport(0, 0, window_width, window_height);

				// Reset projection matrix to keep aspect ratio.
				glUseProgram (shader_programme);
				GLuint proj_handle = glGetUniformLocation(shader_programme, "proj_mat");
				glm::mat4 proj_mat = glm::perspective(glm::radians(30.0f), (float) window_width / (float) window_height, 0.1f, 100.0f);
				glUniformMatrix4fv(proj_handle, 1, GL_FALSE, &proj_mat[0][0]);
			}
			else if (event.type == sf::Event::KeyPressed) {
				if(event.key.code == sf::Keyboard::Q) {
					running = false;
				}
			}
		}
		GLfloat moveSpeed = 0.01f;
		if(sf::Keyboard::isKeyPressed( sf::Keyboard::LShift )) {
			moveSpeed *= 5.0f;
		} else if(sf::Keyboard::isKeyPressed( sf::Keyboard::LControl )) {
			moveSpeed /= 5.0f;
		}

		if(sf::Keyboard::isKeyPressed( sf::Keyboard::W )) {
			sigma += moveSpeed;
		} if(sf::Keyboard::isKeyPressed( sf::Keyboard::S )) {
			sigma -= moveSpeed;
		} if(sf::Keyboard::isKeyPressed( sf::Keyboard::D )) {
			theta += moveSpeed;
		} if(sf::Keyboard::isKeyPressed( sf::Keyboard::A )) {
			theta -= moveSpeed;
		}

		glm::vec3 lookatpos(0.0f, 0.0f, 0.0f);

		GLfloat zoomfactor = 1.0f;

		for(int i = 0; i < sf::Joystick::Count; i++) {
			if(sf::Joystick::isConnected(i)) {
				float y = - sf::Joystick::getAxisPosition(0, sf::Joystick::Y);
				float x = sf::Joystick::getAxisPosition(0, sf::Joystick::X);
				float u = - sf::Joystick::getAxisPosition(0, sf::Joystick::U);
				float r = sf::Joystick::getAxisPosition(0, sf::Joystick::R);
				float z = sf::Joystick::getAxisPosition(0, sf::Joystick::Z);
				printf("Joy %i, x: %0.3f, y: %0.3f, u: %0.3f, r: %0.3f, z: %0.3f\n", i, x, y, u, r, z);
				const float DEADZONE = 5.0f;
				if( x < DEADZONE && x > -DEADZONE ) { x = 0; }
				if( y < DEADZONE && y > -DEADZONE ) { y = 0; }
				/*if( u < DEADZONE && u > -DEADZONE ) { u = 0; }
				if( r < DEADZONE && r > -DEADZONE ) { r = 0; }*/
				if(z > 0) { z = 0; }
				sigma += y*0.0002f;
				theta += x*0.0002f;
				lookatpos[0] = -u*0.01;
				lookatpos[1] = -r*0.01;

				zoomfactor = 1 + z*0.0075;
			}
		}
		if(sigma > 1.4) { sigma = 1.4f; }
		if(sigma < -1.4) { sigma = -1.4f; }

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram (shader_programme);

		glm::vec3 campos = glm::vec3(5*sin(theta)*cos(sigma),5*sin(sigma),5*cos(theta)*cos(sigma));

		//glm::vec3 lookatpos = glm::vec3(0.0f, 1.0f, 0.0f);

		lookatpos = glm::rotateX(lookatpos, (float)(-sigma));
		lookatpos = glm::rotateY(lookatpos, (float)(theta));

		glm::mat4 view_mat = glm::lookAt(
			//glm::vec3(5*sin(theta)*cos(sigma),5*sin(sigma),5*cos(theta)*cos(sigma)),
			//glm::vec3(0,0,0), // and looks at the origin
			campos,
			lookatpos,
			glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
		);
		glUniformMatrix4fv(view_handle, 1, GL_FALSE, &view_mat[0][0]);

		GLuint proj_handle = glGetUniformLocation(shader_programme, "proj_mat");
		glm::mat4 proj_mat = glm::perspective(glm::radians(zoomfactor*30.0f), (float) window_width / (float) window_height, 0.1f, 100.0f);
		glUniformMatrix4fv(proj_handle, 1, GL_FALSE, &proj_mat[0][0]);

		glLineWidth(1/zoomfactor);

		earth.draw();

		for(auto &sat_pos : satellite_positions) {
			sat.draw(sat_pos);
		}

		conns.draw();

		tower.draw(ground_station_positions[0]);
		tower.draw(ground_station_positions[1]);

		window.display();

		if(++frame_counter == 10) {
			printf("FPS: %f\n", 10000.0f / fps_clock.getElapsedTime().asMilliseconds());
			frame_counter = 0;
			fps_clock.restart();
		}

		//sf::sleep(sf::milliseconds(1));
	}
	return 0;
}

