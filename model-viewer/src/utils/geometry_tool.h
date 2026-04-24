#ifndef GEOMETRY_TOOL_H
#define GEOMETRY_TOOL_H

#include <string>
#include <glm/glm.hpp>
#include <algorithm>

typedef struct {
    double x, y, z;
} XYZ;

// https://stackoverflow.com/questions/2316490/the-algorithm-to-find-the-point-of-intersection-of-two-3d-line-segment
// https://paulbourke.net/geometry/pointlineplane/

/*
   Calculate the line segment PaPb that is the shortest route between
   two lines P1P2 and P3P4. Calculate also the values of mua and mub where
      Pa = P1 + mua (P2 - P1)
      Pb = P3 + mub (P4 - P3)
   Return false if no solution exists.
*/
bool LineLineIntersect(XYZ p1, XYZ p2, XYZ p3, XYZ p4, XYZ *pa, XYZ *pb,
                       double *mua, double *mub);

bool LineLineIntersect(const glm::vec3 &p1, const glm::vec3 &p2,
                       const glm::vec3 &p3, const glm::vec3 &p4);

#endif
