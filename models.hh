#pragma once

const double PI = 3.141592654;

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

class Model
{
	public:
		bool setTexture(const std::string& tex_file_name);
		void setShader(GLuint shader_program);

	protected:
		GLuint texture_handle_;

		GLuint vert_vbo_;
		GLuint tex_vbo_;
		GLuint vao_;

		GLuint shader_program_;
};

class Tower : public Model
{

	public:

		Tower();

		//bool setTexture(const std::string& tex_file_name);

		void draw(const Position& pos);
};

class Satellite : public Model
{
	public:

		Satellite();

		//bool setTexture(const std::string& tex_file_name);

		void draw(const Position& pos);
};

class Connections : public Model
{
	private:
		GLuint vert_vbo_;
		GLuint vao_;
		GLuint ibo_;

		GLuint conn_count_;

		GLuint texture_handle_;

	public:
		Connections(const Position& start, const Position& end,
				const std::vector<Position>& satellite_positions);

		void draw();
};


class Sphere : public Model
{
	private:
		GLuint texture_handle_;

		GLuint vert_vbo_;
		GLuint tex_vbo_;
		GLuint vao_;

		GLuint ver_slices_;
		GLuint hor_slices_;

	public:

		Sphere(GLuint hor_slices, GLuint ver_slices, float radius);

		bool setTexture(const std::string& tex_file_name);

		void draw();
};

