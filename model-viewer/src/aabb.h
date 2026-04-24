#ifndef AABB_H
#define AABB_H

#include <stdint.h>
#include <numbers>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct AABB {
    glm::vec3 m_min;
    glm::vec3 m_max;
    AABB()
    {
        m_min = glm::vec3(0xFFFFFFF, 0xFFFFFFF, 0xFFFFFFF);
        m_max = glm::vec3(-0xFFFFFFF, -0xFFFFFFF, -0xFFFFFFF);
    }
    void push(glm::vec3 point)
    {
        m_min.x = min(m_min.x, point.x);
        m_min.y = min(m_min.y, point.y);
        m_min.z = min(m_min.z, point.z);

        m_max.x = max(m_max.x, point.x);
        m_max.y = max(m_max.y, point.y);
        m_max.z = max(m_max.z, point.z);
    }

    bool empty()
    {
        return glm::any(glm::greaterThan(m_min, m_max));
    }

    void push(const AABB& box)
    {

        if (empty()) {
            m_min = box.m_min;
            m_max = box.m_max;
        } else {
            m_min = glm::min(m_min, box.m_min);
            m_max = glm::max(m_max, box.m_max);
        }
    }

    bool contain(const glm::vec3& p)
    {
        return (p.x >= m_min.x && p.x <= m_max.x && p.y >= m_min.y &&
                p.y <= m_max.y && p.z >= m_min.z && p.z <= m_max.z);
    }

    float max_extent()
    {
        glm::vec3 sub = m_max - m_min;
        return max(max(sub.x, sub.y), sub.z);
    }

    float bounding_radius()
    {
        return glm::distance(center(), m_max);
    }

    glm::vec3 center() { return (m_min + m_max) / 2.f; }
};

#endif
