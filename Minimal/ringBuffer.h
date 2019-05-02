#ifndef RINGBUFEER_H
#define RINGBUFEER_H

#include <iostream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct ringBuffer
{
public:
	


	ringBuffer(std::size_t cap) : buffer(cap) {}
	bool empty() const { return sz == 0; }
	bool full() const { return sz == buffer.size(); }

	void push(glm::mat4 mtx)
	{
		if (last >= buffer.size()) last = 0;
		buffer[last] = mtx;
		++last;
		if (full()) first = (first + 1) % buffer.size();
		else ++sz;
	}

	glm::mat4 pop() {
		if(empty()) throw std::logic_error("Buffer underrun");
		glm::mat4 toReturn=buffer[first];

		if (++first==buffer.size()) {
			first = 0;
			--sz;
		}
		else {
			first = (first + 1) % buffer.size();
			--sz;
		}
		
	}

	glm::mat4& operator[] (std::size_t pos)
	{
		auto p = (first + pos) % buffer.size();
		return buffer[p];
	}


private:
	std::vector<glm::mat4> buffer;
	std::size_t first = 0;
	std::size_t last = 0;
	std::size_t sz = 0;
};
#endif