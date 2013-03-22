#ifndef __Simulator_h_
#define __Simulator_h_
#include <OgreSceneManager.h>
#include <OgreEntity.h>
#include "btBulletDynamicsCommon.h"

class Simulator
{
	public:
		Simulator();
		virtual ~Simulator(void);
		bool updatePaddle(Ogre::Vector3 location);
		void setupSimulator(void);
		void runSimulator(bool sim);
		btVector3 getBallLocation(void);
		void handleCollisions(void);

	protected:
		btVector3 paddleLocation;
		btRigidBody* ballBody;
		btRigidBody* paddleBody;
		bool simulate;
		btDiscreteDynamicsWorld* world;

};
#endif // #ifndef __Simulator_h_
