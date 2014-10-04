#ifndef TYPES_H
#define TYPES_H

enum Errors {

};

typedef struct {
	float exposure;
	float zoom;
	float pos[2];
	float aspect;
	float shadows;
  float highlights;
  float contrast;
} Parameters;

#endif

