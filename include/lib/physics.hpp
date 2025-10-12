////////////////////////////////////////////////////////////////////////
// Physics2D by SifuF		www.sifuf.com
//
// C++ single header 2D physics engine implementation.	
// Simple to use; Ideal for rendering, games, and other experimentation.
// 
// Usage..
//  
// in setup
//		p2d::Physics physics;
//		p2d::CircleBody circleBody(radius, x, y, mass, restitution);
//		physics.add(&circleBody);
//  
// every frame
//		physics.update(dt);
//
///////////////////////////

#ifndef PHYSICS_H
#define PHYSICS_H

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include "entt.hpp"
#include "../structs.hpp"
#include "../SceneManager.hpp"

extern SceneManager sceneManager;


namespace p2d {

	
	class Vec2f {
	public:
		float x;
		float y;

		Vec2f() : x(0.0f), y(0.0f) {}

		Vec2f(float x, float y) {
			this->x = x;
			this->y = y;
		}

		[[nodiscard]] Vec2f abs() const {
			Vec2f v;
			v.x = x;
			v.y = y;
			if (x < 0) {
				v.x = -v.x;
			}
			if (y < 0) {
				v.y = -v.y;
			}
			return v;
		}
		[[nodiscard]] float angle(const Vec2f& v) const {
			float dot = this->dot(v);
			float det = this->det(v);
			return (180.0f / 3.14159f) * atan2(det, dot);
		}

		[[nodiscard]] float dot(const Vec2f& v) const {
			return x * v.x + y * v.y;
		}

		[[nodiscard]] float det(const Vec2f& v) const {
			return x * v.y - y * v.x;
		}

		[[nodiscard]] float distance(const Vec2f& v) const {
			return sqrt((x - v.x) * (x - v.x) + (y - v.y) * (y - v.y));
		}

		[[nodiscard]] Vec2f getNormal() const {
			return Vec2f(-y, x);
		}

		[[nodiscard]] float magnitude() const {
			return sqrt(x * x + y * y);
		}
		[[nodiscard]] Vec2f normalise() const {
			float mag = sqrt(x * x + y * y);
			if (mag != 0) {
				return { x / mag, y / mag };
			}
			else {
				return { 0.0, 0.0 };
			}
		}

		[[nodiscard]] Vec2f operator*(const float& f) const {
			Vec2f v;
			v.x = this->x * f;
			v.y = this->y * f;
			return v;
		}

		[[nodiscard]] Vec2f operator/(const float& f) const {
			Vec2f v;
			v.x = this->x / f;
			v.y = this->y / f;
			return v;
		}

		[[nodiscard]] Vec2f operator=(const float& f) const {
			Vec2f v;
			v.x = f;
			v.y = f;
			return v;
		}

		[[nodiscard]] Vec2f operator+(const Vec2f& n) const {
			Vec2f v;
			v.x = this->x + n.x;
			v.y = this->y + n.y;
			return v;
		}

		[[nodiscard]] Vec2f operator-(const Vec2f& n) const {
			Vec2f v;
			v.x = this->x - n.x;
			v.y = this->y - n.y;
			return v;
		}

		bool operator==(const Vec2f& other) const {
			return (this->x == other.x) && (this->y && other.y);
		}
	};

    class Body {
    protected:

        bool m_circle;
        bool m_rect;
        bool m_isStatic;

        Vec2f pos;
        Vec2f vel;
        Vec2f tempPos;
        Vec2f tempVel;
        Vec2f acc;
        Vec2f force;
        float mass;

        float theta;
        float thetaDot;
        float thetaDotDot;
        float IthetaDotDot;
        float inertia;

        float restitution;
        float radius;
        float width;
        float height;
		

    public:
		// Custom EnTT features
		entt::entity m_entity;

		bool ignore;
		bool invert;

        Body() : m_circle(false), m_rect(false), m_isStatic(false), mass(0.0f), theta(0.0f), 
                 thetaDot(0.0f), thetaDotDot(0.0f), IthetaDotDot(0.0f), inertia(1.0f), 
                 restitution(0.0f), radius(0.0f), width(0.0f), height(0.0f) {}

        void init(float x, float y, float m, float rest, bool isStatic, entt::entity __entity) {
            pos.x = x;
            pos.y = y;
            mass = (m < 0) ? 0.0f : m;
		    restitution = std::clamp<float>(m, 0.0f, 1.0f);
            m_isStatic = isStatic;
			m_entity = __entity;
			ignore = false;
			invert = false;
        }

		void reset() { setVelocity({ 0.0f, 0.0f }); }
        void reset(const Vec2f& v) {
            setPosition(v);
            setVelocity({ 0.0f, 0.0f });
        }
        
        void setPosition(const Vec2f& v) {
            pos.x = v.x;
            pos.y = v.y;
        }

        void setTempPosition(const Vec2f& v) { tempPos = v; }
        void setVelocity(const Vec2f& v) { vel = v; }
        void setTempVelocity(const Vec2f& v) { tempVel = v; }
        void setAcceleration(const Vec2f& v) { acc = v; }
        void applyAcceleration(const Vec2f& v) { acc = acc + v; }
        void setForce(const Vec2f& v) { force = v; }
        void applyForce(const Vec2f& v) { force = force + v; }
        void setTheta(float t) { theta = t; }
        void setThetaDot(float t) { thetaDot = t; }
        void setThetaDotDot(float t) { thetaDotDot = t; }
        void setIthetaDotDot(float t) { IthetaDotDot = t; }
        void applyThetaDotDot(float t) { thetaDotDot = thetaDotDot + t; }
        void applyIthetaDotDot(float t) { IthetaDotDot = IthetaDotDot + t; }
        Vec2f getPosition() { return { pos.x, pos.y }; }
        Vec2f getTempPosition() { return tempPos; }
        Vec2f getVelocity() { return vel; }
        Vec2f getTempVelocity() { return tempVel; }
        Vec2f getAcceleration() { return acc; }
        Vec2f getForce() { return force; }
        float getTheta() { return theta; }
        float getThetaDot() { return thetaDot; }
        float getThetaDotDot() { return thetaDotDot; }
        float getIthetaDotDot() { return IthetaDotDot; }
        Vec2f getLength() { return { width, height }; }
        float getRadius() { return radius; }
        float getIntertia() { return inertia; }
		float getMass() { return mass; }
		float getRestitution() { return restitution; }
        [[nodiscard]] bool isStatic() { return m_isStatic; }
        [[nodiscard]] bool isCircle() { return m_circle; }
        [[nodiscard]] bool isRectangle() { return m_rect; }
    };

	class CircleBody : public Body {
	public:

		CircleBody() {}

		CircleBody(float rad, float x, float y, float m, float rest, bool isStatic = false, entt::entity entity = entt::null) {
			create(rad, x, y, m, rest, isStatic, entity);
		}

		void create(float rad, float x, float y, float m, float rest, bool isStatic = false, entt::entity entity = entt::null) {
			m_circle = true;
			radius = rad;
			width = radius;
			height = radius;
			init(x, y, m, rest, isStatic, entity);
		}
	};

	class RectangleBody : public Body {
	public:

		RectangleBody() {}

		RectangleBody(float w, float h, float x, float y, float m, float rest, bool isStatic = false, entt::entity entity = entt::null) {
			create(w, h, x, y, m, rest, isStatic, entity);
		}

		void create(float w, float h, float x, float y, float m, float rest, bool isStatic = false, entt::entity entity = entt::null) {
			m_rect = true;
			pos.x = x;
			pos.y = y;
			width = w;
			height = h;
			init(x, y, m, rest, isStatic, entity);
		}
	};

	struct CollisionInfo {
		Body* bodyA;
		Body* bodyB;
		Vec2f collisionPoint;
		Vec2f normal;
		float penetrationDepth;
		
		CollisionInfo(Body* a, Body* b) : 
			bodyA(a), bodyB(b), 
			collisionPoint(), normal(), 
			penetrationDepth(0.0f) {}
	};

	class Physics {
	private:
		std::vector<Body*> m_Body;
		Vec2f m_gravity;
		Vec2f m_drag;
		Vec2f m_drag_rotational;
		void (*onCollision)(CollisionInfo &);
		bool collided;
    	std::vector<CollisionInfo> m_collisions; // Store all collisions

	public:												 // -av + -bv^2
		Physics() : m_gravity({ 0.0f, 100.0f }), m_drag{ 0.05f, 0.001f }, 
					m_drag_rotational{ 0.01f, 0.01f }, onCollision{ nullptr }, collided(false) {}

		void setGravity(const Vec2f& v) {
			m_gravity = v;
		}

		void setDrag(const Vec2f& v) {
			m_drag = v;
		}
		void setDragRotational(const Vec2f& v) {
			m_drag_rotational = v;
		}

		void add(Body* ob) {
			m_Body.push_back(ob);
		}

		void remove(Body* ob) {
			auto it = std::find(m_Body.begin(), m_Body.end(), ob);
			if (it != m_Body.end()) {
				m_Body.erase(it);
			}
		}

		[[nodiscard]] bool atRest() {
			float err = 2.0f;
			for (Body* o : m_Body) {
				float v = o->getVelocity().magnitude();
				if (v > err) {
					return false;
				}
			}
			return true;
		}

		void setOnCollision(void (*fcnPtr)(CollisionInfo&)) {
			onCollision = fcnPtr;
		}
	
		const std::vector<CollisionInfo>& getCollisions() const {
			return m_collisions;
		}
		
		void clearCollisions() {
			m_collisions.clear();
		}

		[[nodiscard]] bool collisionCheck() {
			bool c = collided;
			collided = false;
			return c;
		}

		void circleCircleCollision(Body* o, Body* p) {

			//compare distance between centres with sum of radii
			float distance = p->getPosition().distance(o->getPosition());
			Vec2f radius_sum = p->getLength() + o->getLength();
			float dx = distance - radius_sum.x;

			if (dx <= 0) {

				// collided = true;
				// if (onCollision != nullptr) {
				// 	onCollision();
				// }

				Vec2f normal = p->getPosition() - o->getPosition();
				Vec2f tangent = normal.getNormal();
				Vec2f normalUnit = normal.normalise();
				Vec2f tangentUnit = tangent.normalise();

				//reverse the Body to resolve the collision 
				Vec2f reverse = normalUnit * dx;
				Vec2f reverse_i_j = { reverse.dot({ 1.0f, 0.0f }) + reverse.dot({ 1.0f, 0.0f }), reverse.dot({ 0.0f, 1.0f }) + reverse.dot({ 0.0f, 1.0f }) };
				o->setTempPosition(o->getTempPosition() + reverse_i_j);

				//velocity is uneffected parallel to tangent
				float tangent_speed = o->getVelocity().dot(tangentUnit);
				Vec2f tangent_velocity = tangentUnit * tangent_speed;

				//initial velocity along normal
				float normal_speed_o = o->getVelocity().dot(normalUnit);
				float normal_speed_p = p->getVelocity().dot(normalUnit);

				//conservation of momentum and kinetic energy - elastic collision
				float normal_speed_after_o = normal_speed_o * (o->getMass() - p->getMass()) / (o->getMass() + p->getMass()) + normal_speed_p * (2.0f * p->getMass()) / (o->getMass() + p->getMass());
				Vec2f normal_velocity = normalUnit * normal_speed_after_o;
				Vec2f normal_velocity_i_j = { tangent_velocity.dot({1.0f, 0.0f}) + normal_velocity.dot({1.0f, 0.0f}) , tangent_velocity.dot({0.0f, 1.0f}) + normal_velocity.dot({0.0f, 1.0f}) };
				o->setTempVelocity(normal_velocity_i_j);

				//avoid getting stuck
				float stuck = 0.01f;
				if ((abs(p->getPosition().x - o->getPosition().x) < stuck) && (abs(p->getPosition().y - o->getPosition().y) < stuck)) {
					//std::cout << "stuck!" << std::endl;
					o->setTempPosition({ o->getPosition().x - (o->getRadius() + 0.1f), o->getPosition().y });
					p->setTempPosition({ o->getPosition().x + (p->getRadius() + 0.1f), o->getPosition().y });
				}
			}
		}

		void circleRectangleCollision(Body* o, Body* p, bool circle) {

			Vec2f pos_o;
			Vec2f len_o;
			Vec2f pos_p;
			Vec2f len_p;

			if (circle) {
				pos_o = o->getPosition();
				len_o = o->getLength();
				pos_p = p->getPosition();
				len_p = p->getLength() / 2.0f;
			}
			else {
				pos_p = p->getPosition();
				len_p = p->getLength();
				pos_o = o->getPosition();
				len_o = o->getLength() / 2.0f;
			}

			bool condition = pos_o.x - len_o.x < pos_p.x + len_p.x && pos_o.x + len_o.x > pos_p.x - len_p.x &&
				pos_o.y - len_o.y < pos_p.y + len_p.y && pos_o.y + len_o.y > pos_p.y - len_p.y;

			if (condition) {
				
				// collided = true;
				// if (onCollision != nullptr) {
				// 	onCollision();
				// }
				
				float dx = (len_p.x + len_o.x) - abs(pos_p.x - pos_o.x);
				float dy = (len_p.y + len_o.y) - abs(pos_p.y - pos_o.y);

				Vec2f reverse_i_j;
				Vec2f normalUnit;

				//resolve towards the smallest
				if (dx <= dy) {
					if (pos_o.x < pos_p.x) {			//source coming from left
						normalUnit = { -1.0f, 0.0f };
						reverse_i_j = normalUnit * dx;
					}
					else {								//source coming from right
						normalUnit = { 1.0f, 0.0f };
						reverse_i_j = normalUnit * dx;
					}
				}
				else {
					if (pos_o.y < pos_p.y) {			//source coming from top
						normalUnit = { 0.0f, -1.0f };
						reverse_i_j = normalUnit * dy;
					}
					else {								//source coming from bottom
						normalUnit = { 0.0f, 1.0f };
						reverse_i_j = normalUnit * dy;
					}
				}
				o->setTempPosition(o->getTempPosition() + reverse_i_j);

				//velocity is uneffected parallel to tangent
				Vec2f tangentUnit = normalUnit.getNormal();
				float tangent_speed = o->getVelocity().dot(tangentUnit);
				Vec2f tangent_velocity = tangentUnit * tangent_speed;

				// initial velocity along normal
				float normal_speed_o = o->getVelocity().dot(normalUnit);
				float normal_speed_p = p->getVelocity().dot(normalUnit);

				//conservation of momentum and kinetic energy - elastic collision
				float normal_speed_after_o = normal_speed_o * (o->getMass() - p->getMass()) / (o->getMass() + p->getMass()) + normal_speed_p * (2.0f * p->getMass()) / (o->getMass() + p->getMass());
				Vec2f normal_velocity = normalUnit * normal_speed_after_o;

				Vec2f velocity_i_j = normal_velocity + tangent_velocity;
				o->setTempVelocity(velocity_i_j);

				//Temporary angular impulse
				int r = rand() % 2;
				float ang_velocity = (r == 0) ? velocity_i_j.magnitude() : -velocity_i_j.magnitude();
				o->setThetaDot(ang_velocity / 2.0f);
			}
		}

		void rectangleRectangleCollision(Body* o, Body* p) {

			Vec2f pos_o = o->getPosition();
			Vec2f len_o = o->getLength() / 2.0f;
			Vec2f pos_p = p->getPosition();
			Vec2f len_p = p->getLength() / 2.0f;

			bool condition = pos_o.x - len_o.x < pos_p.x + len_p.x && pos_o.x + len_o.x > pos_p.x - len_p.x &&
							 pos_o.y - len_o.y < pos_p.y + len_p.y && pos_o.y + len_o.y > pos_p.y - len_p.y;

			if (condition) {

				collided = true;
				
				// Check if either body should ignore the collision resolution
				if (o->ignore || p->ignore) {
					// Create collision info
					CollisionInfo info(o, p);

					// Store the collision
					m_collisions.push_back(info);

					// Call the callback if set
					if (onCollision != nullptr) {
						onCollision(info);
					}

					return; // Skip resolution
				}
				
				// Check if o or p is an Interior entity
				bool o_is_interior = sceneManager.getCurrentRegistry().all_of<Interior>(o->m_entity);

				bool p_is_interior = sceneManager.getCurrentRegistry().all_of<Interior>(p->m_entity);

				if (o_is_interior) {
					// Check if p has Inside and its .interior == o->m_entity
					if (sceneManager.getCurrentRegistry().all_of<Inside>(p->m_entity)) {
						const auto& inside = sceneManager.getCurrentRegistry().get<Inside>(p->m_entity);
						// If inside.interior == o->m_entity, set o->ignore = true
						if (inside.interior == o->m_entity) {
							return;
						}
					}
				}

				if (p_is_interior) {
					// Check if o has Inside and its .interior == p->m_entity
					if (sceneManager.getCurrentRegistry().all_of<Inside>(o->m_entity)) {
						const auto& inside = sceneManager.getCurrentRegistry().get<Inside>(o->m_entity);
						if (inside.interior == p->m_entity) {
							return;
						}
					}
				}
				
				//compare distance between centres with sum of lengths (and widths)
				float dx = (len_p.x + len_o.x) - abs(pos_p.x - pos_o.x);
				float dy = (len_p.y + len_o.y) - abs(pos_p.y - pos_o.y);

				Vec2f reverse_i_j;
				Vec2f normalUnit;

				//resolve towards the smallest
				if (dx <= dy) {
					if (pos_o.x < pos_p.x) {			//source coming from left
						normalUnit = { -1.0f, 0.0f };
						reverse_i_j = normalUnit * dx;
					}
					else {								//source coming from right
						normalUnit = { 1.0f, 0.0f };
						reverse_i_j = normalUnit * dx;
					}
				}
				else {
					if (pos_o.y < pos_p.y) {			//source coming from top
						normalUnit = { 0.0f, -1.0f };
						reverse_i_j = normalUnit * dy;
					}
					else {								//source coming from bottom
						normalUnit = { 0.0f, 1.0f };
						reverse_i_j = normalUnit * dy;
					}
				}

				if(!p->ignore)
					o->setTempPosition(o->getTempPosition() + reverse_i_j);

				//initial velocity along normal
				float normal_speed_o = o->getVelocity().dot(normalUnit);
				float normal_speed_p = p->getVelocity().dot(normalUnit);

				//conservation of momentum and kinetic energy - elastic collision
				float normal_speed_after_o = normal_speed_o * (o->getMass() - p->getMass()) / (o->getMass() + p->getMass()) + normal_speed_p * (2.0f * p->getMass()) / (o->getMass() + p->getMass());
				Vec2f normal_velocity = normalUnit * normal_speed_after_o;	// = velocity_i_j because rectangles are axis alligned

				if(!p->ignore)
					o->setTempVelocity(normal_velocity);


				// Create collision info
				CollisionInfo info(o, p);

				// Store the collision
				m_collisions.push_back(info);
			
				// Call the callback if set
				if (onCollision != nullptr) {
					onCollision(info);
				}
				
				// KEEP THIS FOR REFERENCE LATER
				// info.normal = normalUnit;
				// info.penetrationDepth = (dx <= dy) ? dx : dy;
				
				// // Calculate approximate collision point (center of overlap region)
				// Vec2f halfSizeO = o->getLength() / 2.0f;
				// Vec2f halfSizeP = p->getLength() / 2.0f;
				// Vec2f minO = o->getPosition() - halfSizeO;
				// Vec2f maxO = o->getPosition() + halfSizeO;
				// Vec2f minP = p->getPosition() - halfSizeP;
				// Vec2f maxP = p->getPosition() + halfSizeP;
				
				// Vec2f minOverlap = {
				// 	std::max(minO.x, minP.x),
				// 	std::max(minO.y, minP.y)
				// };
				
				// Vec2f maxOverlap = {
				// 	std::min(maxO.x, maxP.x),
				// 	std::min(maxO.y, maxP.y)
				// };
				
				// info.collisionPoint = {
				// 	(minOverlap.x + maxOverlap.x) / 2.0f,
				// 	(minOverlap.y + maxOverlap.y) / 2.0f
				// };

			}
		}

		void update(double d) {
			clearCollisions();

			float dt = float(d);
			
			//Apply forces
			for (auto it = m_Body.begin(); it != m_Body.end(); ) {
				Body* o = *it;
				
				if (!sceneManager.getCurrentRegistry().valid(o->m_entity)) {
					it = m_Body.erase(it);
					continue;
				}

				if (o->isStatic()) {
					++it;
					continue;
				}
				
				//gravity
				o->setAcceleration(m_gravity);

				//drag
				float speed = o->getVelocity().magnitude();
				Vec2f velocityUnit = o->getVelocity().normalise();
				o->applyForce(velocityUnit * speed * -m_drag.x);
				o->applyForce(velocityUnit * speed * speed * -m_drag.y);

				//rotational drag
				float ang_speed = abs(o->getThetaDot());
				float ang_velocityUnit = (o->getThetaDot() < 0) ? -1.0f : 1.0f;
				o->applyIthetaDotDot(ang_velocityUnit * ang_speed * -m_drag_rotational.x);
				o->applyIthetaDotDot(ang_velocityUnit * ang_speed * ang_speed * -m_drag_rotational.y);

				++it;
			}

			//Euler integrator
			for (Body* o : m_Body) {
				
				
				//Linear
				Vec2f acc = o->getAcceleration() + o->getForce() / o->getMass();
				o->setVelocity(o->getVelocity() + acc * dt);
				o->setPosition(o->getPosition() + o->getVelocity() * dt);

				o->setForce({ 0.0, 0.0 });	//reset
				o->setAcceleration({ 0.0, 0.0 });

				//Angular
				float angular_acc = o->getThetaDotDot() + o->getIthetaDotDot() / o->getIntertia();
				o->setThetaDot(o->getThetaDot() + angular_acc * dt);
				o->setTheta(o->getTheta() + o->getThetaDot() * dt);

				o->setIthetaDotDot(0.0f); //reset
				o->setThetaDotDot(0.0f);
			}

			//Collision detection
			for (Body* o : m_Body) {		//First Compute whole system on temp variables
				o->setTempVelocity(o->getVelocity());
				o->setTempPosition(o->getPosition());
			}

			for (Body* o : m_Body) {		//Source Body

				if (o->isStatic()) {		//Static Bodies can be targets (collided with and resolved from), but are not resolved themselves.
					continue;
				}

				for (Body* p : m_Body) {	//Target Body

					if (o == p) {
						continue;
					}

					if (o->isCircle() && p->isCircle()) {
						circleCircleCollision(o, p);					
					}

					else if (o->isCircle() && p->isRectangle()) {
						circleRectangleCollision(o, p, true);
					}

					else if (o->isRectangle() && p->isCircle()) {
						circleRectangleCollision(o, p, false);
					}

					else if (o->isRectangle() && p->isRectangle()) {
						rectangleRectangleCollision(o, p);	
					}
				}
			}

			for (Body* o : m_Body) {

				o->setVelocity(o->getTempVelocity());
				o->setPosition(o->getTempPosition());

				//clamp velocity to zero for slow very Bodies
				if (o->getVelocity().magnitude() < 0.01f) {
					o->setVelocity({ 0.0f, 0.0f });
				}
				if (abs(o->getThetaDot()) < 0.01f) {
					o->setThetaDot(0.0f);
				}
			}
		}
	};
}
struct PhysicsBodyRect {
    p2d::RectangleBody *body;
    // p2d::CircleBody *body;
    bool added;
    bool ignore;

	// Add destructor
    // ~PhysicsBodyRect() {
    //     if (body != nullptr) {
    //         delete body;
    //         body = nullptr;
    //     }
    // }
};
#endif // PHYSICS_H