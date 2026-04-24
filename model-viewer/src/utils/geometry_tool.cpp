#include "utils/geometry_tool.h"

#include <string>
#include <glm/glm.hpp>
#include <algorithm>

constexpr double EPS = 1e-8;

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
                       double *mua, double *mub)
{
    XYZ p13, p43, p21;
    double d1343, d4321, d1321, d4343, d2121;
    double numer, denom;

    p13.x = p1.x - p3.x;
    p13.y = p1.y - p3.y;
    p13.z = p1.z - p3.z;
    p43.x = p4.x - p3.x;
    p43.y = p4.y - p3.y;
    p43.z = p4.z - p3.z;
    if (abs(p43.x) < EPS && abs(p43.y) < EPS && abs(p43.z) < EPS)
        return (false);
    p21.x = p2.x - p1.x;
    p21.y = p2.y - p1.y;
    p21.z = p2.z - p1.z;
    if (abs(p21.x) < EPS && abs(p21.y) < EPS && abs(p21.z) < EPS)
        return (false);

    d1343 = p13.x * p43.x + p13.y * p43.y + p13.z * p43.z;
    d4321 = p43.x * p21.x + p43.y * p21.y + p43.z * p21.z;
    d1321 = p13.x * p21.x + p13.y * p21.y + p13.z * p21.z;
    d4343 = p43.x * p43.x + p43.y * p43.y + p43.z * p43.z;
    d2121 = p21.x * p21.x + p21.y * p21.y + p21.z * p21.z;

    denom = d2121 * d4343 - d4321 * d4321;
    if (abs(denom) < EPS) return (false);
    numer = d1343 * d4321 - d1321 * d4343;

    *mua = numer / denom;
    *mub = (d1343 + d4321 * (*mua)) / d4343;

    pa->x = p1.x + *mua * p21.x;
    pa->y = p1.y + *mua * p21.y;
    pa->z = p1.z + *mua * p21.z;
    pb->x = p3.x + *mub * p43.x;
    pb->y = p3.y + *mub * p43.y;
    pb->z = p3.z + *mub * p43.z;

    return (true);
}

bool LineLineIntersect(const glm::vec3 &p1, const glm::vec3 &p2,
                       const glm::vec3 &p3, const glm::vec3 &p4)
{
    XYZ a{p1.x, p1.y, p1.z};
    XYZ b{p2.x, p2.y, p2.z};
    XYZ c{p3.x, p3.y, p3.z};
    XYZ d{p4.x, p4.y, p4.z};

    XYZ pa{0.0, 0.0, 0.0};
    XYZ pb{0.0, 0.0, 0.0};

    double mua = 0.0;
    double mub = 0.0;

    bool res = LineLineIntersect(a, b, c, d, &pa, &pb, &mua, &mub);
    double square = (pa.x - pb.x) * (pa.x - pb.x) +
                    (pa.y - pb.y) * (pa.y - pb.y) +
                    (pa.z - pb.z) * (pa.z - pb.z);

    if (square < 1e-12 && abs(mua) <= 1.0 && abs(mub) <= 1.0)
    {
        return true;
    }

    return false;
}

