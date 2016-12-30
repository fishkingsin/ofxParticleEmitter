//
// ofxParticleEmitter.cpp
//
// Copyright (c) 2010 71Squared, ported to Openframeworks by Shawn Roske
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "ofxParticleEmitter.h"

// ------------------------------------------------------------------------
// Lifecycle
// ------------------------------------------------------------------------

ofxParticleEmitter::ofxParticleEmitter()
{
	settings = NULL;
	
	emitterType = kParticleTypeGravity;
	texture = NULL;
	sourcePosition.x = sourcePosition.y = 0.0f;
	sourcePositionVariance.x = sourcePositionVariance.y = 0.0f;
	angle = angleVariance = 0.0f;								
	speed = speedVariance = 0.0f;	
	radialAcceleration = tangentialAcceleration = 0.0f;
	radialAccelVariance = tangentialAccelVariance = 0.0f;
	gravity.x = gravity.y = 0.0f;
	particleLifespan = particleLifespanVariance = 0.0f;			
	startColor.r = startColor.g = startColor.b = startColor.a = 1.0f;
	startColorVariance.r = startColorVariance.g = startColorVariance.b = startColorVariance.a = 1.0f;
	finishColor.r = finishColor.g = finishColor.b = finishColor.a = 1.0f;
	finishColorVariance.r = finishColorVariance.g = finishColorVariance.b = finishColorVariance.a = 1.0f;
	startParticleSize = startParticleSizeVariance = 0.0f;
	finishParticleSize = finishParticleSizeVariance = 0.0f;
	maxParticles = 0.0f;
	particleCount = 0;
	emissionRate = 0.0f;
	emitCounter = 0.0f;	
	elapsedTime = 0.0f;
	duration = -1;
	lastUpdateMillis = 0;
    
	blendFuncSource = blendFuncDestination = 0;

	maxRadius = maxRadiusVariance = radiusSpeed = minRadius = 0.0f;
	rotatePerSecond = rotatePerSecondVariance = 0.0f;
	
	active = useTexture = false;
	particleIndex = 0;

	verticesID = 0;
	particles = NULL;
	vertices = NULL;
}

ofxParticleEmitter::~ofxParticleEmitter()
{
	exit();
}

void ofxParticleEmitter::exit()
{	
	if ( texture != NULL )
		delete texture;
	texture = NULL;
	
	if ( particles != NULL )
		delete particles;
	particles = NULL;
	
	if ( vertices != NULL )
		delete vertices;
	vertices = NULL;
	
	glDeleteBuffers( 1, &verticesID );
}

bool ofxParticleEmitter::loadFromXml( const std::string& filename )
{
	bool ok = false;
	
	settings = new ofxXmlSettings();
	
	ok = settings->loadFile( filename );
	if ( ok )
	{
		parseParticleConfig();
		setupArrays();
		
		ok = active = true;
	}

	delete settings;
	settings = NULL;
	
	return ok;
}

void ofxParticleEmitter::parseParticleConfig()
{
	if ( settings == NULL )
	{
		ofLog( OF_LOG_ERROR, "ofxParticleEmitter::parseParticleConfig() - XML settings is invalid!" );
		return;
	}
	
	settings->pushTag( "particleEmitterConfig" );

	std::string imageFilename	= settings->getAttribute( "texture", "name", "" );
	std::string imageData		= settings->getAttribute( "texture", "data", "" );
	
	if ( imageFilename != "" )
	{
		ofLog( OF_LOG_WARNING, "ofxParticleEmitter::parseParticleConfig() - loading image file " + imageFilename);
		
		texture = new ofImage();
		texture->load( imageFilename );
		texture->setUseTexture( true );
		texture->setAnchorPercent( 0.5f, 0.5f );
		
		textureData = texture->getTexture().getTextureData();
	}
	else if ( imageData != "" )
	{
		// TODO
		
		ofLog( OF_LOG_ERROR, "ofxParticleEmitter::parseParticleConfig() - image data found but not yet implemented!" );
		return;
	}

    emitterType					= settings->getAttribute( "emitterType", "value", emitterType );
	
	sourcePosition.x			= settings->getAttribute( "sourcePosition", "x", sourcePosition.x );
	sourcePosition.y			= settings->getAttribute( "sourcePosition", "y", sourcePosition.y );
	
	speed						= settings->getAttribute( "speed", "value", speed );
	speedVariance				= settings->getAttribute( "speedVariance", "value", speedVariance );
	particleLifespan			= settings->getAttribute( "particleLifespan", "value", particleLifespan );
	particleLifespanVariance	= settings->getAttribute( "particleLifespanVariance", "value", particleLifespanVariance );
	angle						= settings->getAttribute( "angle", "value", angle );
	angleVariance				= settings->getAttribute( "angleVariance", "value", angleVariance );
	
	gravity.x					= settings->getAttribute( "gravity", "x", gravity.x );
	gravity.y					= settings->getAttribute( "gravity", "y", gravity.y );
	
	radialAcceleration			= settings->getAttribute( "radialAcceleration", "value", radialAcceleration );
	tangentialAcceleration		= settings->getAttribute( "tangentialAcceleration", "value", tangentialAcceleration );
	
	startColor.r				= settings->getAttribute( "startColor", "red", startColor.r );
	startColor.g			= settings->getAttribute( "startColor", "green", startColor.g );
	startColor.b				= settings->getAttribute( "startColor", "blue", startColor.b );
	startColor.a			= settings->getAttribute( "startColor", "alpha", startColor.a );
	
	startColorVariance.r		= settings->getAttribute( "startColorVariance", "red", startColorVariance.r );
	startColorVariance.g	= settings->getAttribute( "startColorVariance", "green", startColorVariance.g );
	startColorVariance.b		= settings->getAttribute( "startColorVariance", "blue", startColorVariance.b );
	startColorVariance.a	= settings->getAttribute( "startColorVariance", "alpha", startColorVariance.a );
	
	finishColor.r				= settings->getAttribute( "finishColor", "red", finishColor.r );
	finishColor.g			= settings->getAttribute( "finishColor", "green", finishColor.g );
	finishColor.b			= settings->getAttribute( "finishColor", "blue", finishColor.b );
	finishColor.a			= settings->getAttribute( "finishColor", "alpha", finishColor.a );
	
	finishColorVariance.r		= settings->getAttribute( "finishColorVariance", "red", finishColorVariance.r );
	finishColorVariance.g	= settings->getAttribute( "finishColorVariance", "green", finishColorVariance.g );
	finishColorVariance.b	= settings->getAttribute( "finishColorVariance", "blue", finishColorVariance.b );
	finishColorVariance.a	= settings->getAttribute( "finishColorVariance", "alpha", finishColorVariance.a );
	
	maxParticles				= settings->getAttribute( "maxParticles", "value", maxParticles );
	startParticleSize			= settings->getAttribute( "startParticleSize", "value", startParticleSize );
	startParticleSizeVariance	= settings->getAttribute( "startParticleSizeVariance", "value", startParticleSizeVariance );
	finishParticleSize			= settings->getAttribute( "finishParticleSize", "value", finishParticleSize );
	finishParticleSizeVariance	= settings->getAttribute( "finishParticleSizeVariance", "value", finishParticleSizeVariance );
	duration					= settings->getAttribute( "duration", "value", duration );
	blendFuncSource				= settings->getAttribute( "blendFuncSource", "value", blendFuncSource );
	blendFuncDestination		= settings->getAttribute( "blendFuncDestination", "value", blendFuncDestination );
	
	maxRadius					= settings->getAttribute( "maxRadius", "value", maxRadius );
	maxRadiusVariance			= settings->getAttribute( "maxRadiusVariance", "value", maxRadiusVariance );
	radiusSpeed					= settings->getAttribute( "radiusSpeed", "value", radiusSpeed );
	minRadius					= settings->getAttribute( "minRadius", "value", minRadius );
	
	rotatePerSecond				= settings->getAttribute( "rotatePerSecond", "value", rotatePerSecond );
	rotatePerSecondVariance		= settings->getAttribute( "rotatePerSecondVariance", "value", rotatePerSecondVariance );
}

void ofxParticleEmitter::setupArrays()
{
	// Allocate the memory necessary for the particle emitter arrays
	particles = (Particle*)malloc( sizeof( Particle ) * maxParticles );
	vertices = (PointSprite*)malloc( sizeof( PointSprite ) * maxParticles );
	
	// If one of the arrays cannot be allocated throw an assertion as this is bad
	assert( particles && vertices );
	
	// Generate the vertices VBO
	glGenBuffers( 1, &verticesID );
	
	// Set the particle count to zero
	particleCount = 0;
	
	// Reset the elapsed time
	elapsedTime = 0;
}

// ------------------------------------------------------------------------
// Particle Management
// ------------------------------------------------------------------------

bool ofxParticleEmitter::addParticle()
{
	// If we have already reached the maximum number of particles then do nothing
	if(particleCount == maxParticles)
		return false;
	
	// Take the next particle out of the particle pool we have created and initialize it
	Particle *particle = &particles[particleCount];
	initParticle( particle );
	
	// Increment the particle count
	particleCount++;
	
	// Return true to show that a particle has been created
	return true;
}

void ofxParticleEmitter::initParticle( Particle* particle )
{
	// Init the position of the particle.  This is based on the source position of the particle emitter
	// plus a configured variance.  The RANDOM_MINUS_1_TO_1 macro allows the number to be both positive
	// and negative
	particle->position.x = sourcePosition.x + sourcePositionVariance.x * RANDOM_MINUS_1_TO_1();
	particle->position.y = sourcePosition.y + sourcePositionVariance.y * RANDOM_MINUS_1_TO_1();
    particle->startPos.x = sourcePosition.x;
    particle->startPos.y = sourcePosition.y;
	
	// Init the direction of the particle.  The newAngle is calculated using the angle passed in and the
	// angle variance.
	float newAngle = (GLfloat)DEGREES_TO_RADIANS(angle + angleVariance * RANDOM_MINUS_1_TO_1());
	
	// Create a new Vector2f using the newAngle
	Vector2f vector = Vector2fMake(cosf(newAngle), sinf(newAngle));
	
	// Calculate the vectorSpeed using the speed and speedVariance which has been passed in
	float vectorSpeed = speed + speedVariance * RANDOM_MINUS_1_TO_1();
	
	// The particles direction vector is calculated by taking the vector calculated above and
	// multiplying that by the speed
	particle->direction = Vector2fMultiply(vector, vectorSpeed);
	
	// Set the default diameter of the particle from the source position
	particle->radius = maxRadius + maxRadiusVariance * RANDOM_MINUS_1_TO_1();
	particle->radiusDelta = (maxRadius / particleLifespan) * (1.0 / MAXIMUM_UPDATE_RATE);
	particle->angle = DEGREES_TO_RADIANS(angle + angleVariance * RANDOM_MINUS_1_TO_1());
	particle->degreesPerSecond = DEGREES_TO_RADIANS(rotatePerSecond + rotatePerSecondVariance * RANDOM_MINUS_1_TO_1());
    
    particle->radialAcceleration = radialAcceleration;
    particle->tangentialAcceleration = tangentialAcceleration;
	
	// Calculate the particles life span using the life span and variance passed in
	particle->timeToLive = MAX(0, particleLifespan + particleLifespanVariance * RANDOM_MINUS_1_TO_1());
	
	// Calculate the particle size using the start and finish particle sizes
	GLfloat particleStartSize = startParticleSize + startParticleSizeVariance * RANDOM_MINUS_1_TO_1();
	GLfloat particleFinishSize = finishParticleSize + finishParticleSizeVariance * RANDOM_MINUS_1_TO_1();
	particle->particleSizeDelta = ((particleFinishSize - particleStartSize) / particle->timeToLive) * (1.0 / MAXIMUM_UPDATE_RATE);
	particle->particleSize = MAX(0, particleStartSize);
	
	// Calculate the color the particle should have when it starts its life.  All the elements
	// of the start color passed in along with the variance are used to calculate the star color
	ofFloatColor start = {0, 0, 0, 0};
	start.r = startColor.r + startColorVariance.r * RANDOM_MINUS_1_TO_1();
	start.g = startColor.g + startColorVariance.g * RANDOM_MINUS_1_TO_1();
	start.b = startColor.b + startColorVariance.b * RANDOM_MINUS_1_TO_1();
	start.a = startColor.a + startColorVariance.a * RANDOM_MINUS_1_TO_1();
	
	// Calculate the color the particle should be when its life is over.  This is done the same
	// way as the start color above
	ofFloatColor end = {0, 0, 0, 0};
	end.r = finishColor.r + finishColorVariance.r * RANDOM_MINUS_1_TO_1();
	end.g = finishColor.g + finishColorVariance.g * RANDOM_MINUS_1_TO_1();
	end.b = finishColor.b + finishColorVariance.b * RANDOM_MINUS_1_TO_1();
	end.a = finishColor.a + finishColorVariance.a * RANDOM_MINUS_1_TO_1();
	
	// Calculate the delta which is to be applied to the particles color during each cycle of its
	// life.  The delta calculation uses the life span of the particle to make sure that the 
	// particles color will transition from the start to end color during its life time.  As the game
	// loop is using a fixed delta value we can calculate the delta color once saving cycles in the 
	// update method
	particle->color = start;
	particle->deltaColor.r = ((end.r - start.r) / particle->timeToLive) * (1.0 / MAXIMUM_UPDATE_RATE);
	particle->deltaColor.g = ((end.g - start.g) / particle->timeToLive)  * (1.0 / MAXIMUM_UPDATE_RATE);
	particle->deltaColor.b = ((end.b - start.b) / particle->timeToLive)  * (1.0 / MAXIMUM_UPDATE_RATE);
	particle->deltaColor.a = ((end.a - start.a) / particle->timeToLive)  * (1.0 / MAXIMUM_UPDATE_RATE);
}

void ofxParticleEmitter::stopParticleEmitter()
{
	active = false;
	elapsedTime = 0;
	emitCounter = 0;
}

// ------------------------------------------------------------------------
// Update
// ------------------------------------------------------------------------

void ofxParticleEmitter::update()
{
	if ( !active ) return;

	// Calculate the emission rate
	emissionRate = maxParticles / particleLifespan;

	GLfloat aDelta = (ofGetElapsedTimeMillis()-lastUpdateMillis)/1000.0f;
	
	// If the emitter is active and the emission rate is greater than zero then emit
	// particles
	if(active && emissionRate) {
		float rate = 1.0f/emissionRate;
		emitCounter += aDelta;
		while(particleCount < maxParticles && emitCounter > rate) {
			addParticle();
			emitCounter -= rate;
		}
		
		elapsedTime += aDelta;
		if(duration != -1 && duration < elapsedTime)
			stopParticleEmitter();
	}
	
	// Reset the particle index before updating the particles in this emitter
	particleIndex = 0;
	
	// Loop through all the particles updating their location and color
	while(particleIndex < particleCount) {
		
		// Get the particle for the current particle index
		Particle *currentParticle = &particles[particleIndex];
        
        // FIX 1
        // Reduce the life span of the particle
        currentParticle->timeToLive -= aDelta;
		
		// If the current particle is alive then update it
		if(currentParticle->timeToLive > 0) {
			
			// If maxRadius is greater than 0 then the particles are going to spin otherwise
			// they are effected by speed and gravity
			if (emitterType == kParticleTypeRadial) {
				
                // FIX 2
                // Update the angle of the particle from the sourcePosition and the radius.  This is only
				// done of the particles are rotating
				currentParticle->angle += currentParticle->degreesPerSecond * aDelta;
				currentParticle->radius -= currentParticle->radiusDelta;
                
				Vector2f tmp;
				tmp.x = sourcePosition.x - cosf(currentParticle->angle) * currentParticle->radius;
				tmp.y = sourcePosition.y - sinf(currentParticle->angle) * currentParticle->radius;
				currentParticle->position = tmp;
				
				if (currentParticle->radius < minRadius)
					currentParticle->timeToLive = 0;
			} else {
				Vector2f tmp, radial, tangential;
                
                radial = Vector2fZero;
                Vector2f diff = Vector2fSub(currentParticle->startPos, Vector2fZero);
                
                currentParticle->position = Vector2fSub(currentParticle->position, diff);
                
                if (currentParticle->position.x || currentParticle->position.y)
                    radial = Vector2fNormalize(currentParticle->position);
                
                tangential.x = radial.x;
                tangential.y = radial.y;
                radial = Vector2fMultiply(radial, currentParticle->radialAcceleration);
                
                GLfloat newy = tangential.x;
                tangential.x = -tangential.y;
                tangential.y = newy;
                tangential = Vector2fMultiply(tangential, currentParticle->tangentialAcceleration);
                
				tmp = Vector2fAdd( Vector2fAdd(radial, tangential), gravity);
                tmp = Vector2fMultiply(tmp, aDelta);
				currentParticle->direction = Vector2fAdd(currentParticle->direction, tmp);
				tmp = Vector2fMultiply(currentParticle->direction, aDelta);
				currentParticle->position = Vector2fAdd(currentParticle->position, tmp);
                currentParticle->position = Vector2fAdd(currentParticle->position, diff);
			}
			
			// Update the particles color
			currentParticle->color.r += currentParticle->deltaColor.r;
			currentParticle->color.g += currentParticle->deltaColor.g;
			currentParticle->color.b += currentParticle->deltaColor.b;
			currentParticle->color.a += currentParticle->deltaColor.a;
			
			// Place the position of the current particle into the vertices array
			vertices[particleIndex].x = currentParticle->position.x;
			vertices[particleIndex].y = currentParticle->position.y;
			
			// Place the size of the current particle in the size array
			currentParticle->particleSize += currentParticle->particleSizeDelta;
			vertices[particleIndex].size = MAX(0, currentParticle->particleSize);
			
			// Place the color of the current particle into the color array
			vertices[particleIndex].color = currentParticle->color;
			
			// Update the particle counter
			particleIndex++;
		} else {
			
			// As the particle is not alive anymore replace it with the last active particle 
			// in the array and reduce the count of particles by one.  This causes all active particles
			// to be packed together at the start of the array so that a particle which has run out of
			// life will only drop into this clause once
			if(particleIndex != particleCount - 1)
				particles[particleIndex] = particles[particleCount - 1];
			particleCount--;
		}
	}

	lastUpdateMillis = ofGetElapsedTimeMillis();
}

// ------------------------------------------------------------------------
// Render
// ------------------------------------------------------------------------

void ofxParticleEmitter::draw(int x /* = 0 */, int y /* = 0 */)
{
	if ( !active ) return;
	
	glPushMatrix();
	glTranslatef( x, y, 0.0f );
	
#ifdef TARGET_OF_IPHONE
	
	drawPointsOES();
	
#else
	
	drawTextures();
	//drawPoints();
	
#endif
	
	glPopMatrix();
}

void ofxParticleEmitter::drawTextures()
{
	glEnable(GL_BLEND);
	glBlendFunc(blendFuncSource, blendFuncDestination);
	
	for( int i = 0; i < particleCount; i++ )
	{
		PointSprite* ps = &vertices[i];
		ofSetColor( ps->color.r*255.0f, ps->color.g*255.0f, 
				   ps->color.b*255.0f, ps->color.a*255.0f );
		texture->draw( ps->x, ps->y, ps->size, ps->size );
	}
	
	glDisable(GL_BLEND);
}

// this doesn't yet work, it is an attempt to port over the point sprite logic
// from opengles. It draws the point sprites but it doesn't replace the point
// size or color values. I left it here in case anyone wants to fix it :)
void ofxParticleEmitter::drawPoints()
{
	// Disable the texture coord array so that texture information is not copied over when rendering
	// the point sprites.
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	// Bind to the verticesID VBO and popuate it with the necessary vertex & color informaiton
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PointSprite) * maxParticles, vertices, GL_DYNAMIC_DRAW);
	
	// Configure the vertex pointer which will use the currently bound VBO for its data
	glVertexPointer(2, GL_FLOAT, sizeof(PointSprite), 0);
	glColorPointer(4,GL_FLOAT,sizeof(PointSprite),(GLvoid*) (sizeof(GLfloat)*3));
	
	// Bind to the particles texture
	glBindTexture(GL_TEXTURE_2D, (GLuint)textureData.textureID);
	
	// Enable the point size array
	
	
	//glEnableClientState(GL_POINT_SIZE_ARRAY_OES);
	
	
	// Configure the point size pointer which will use the currently bound VBO.  PointSprite contains
	// both the location of the point as well as its size, so the config below tells the point size
	// pointer where in the currently bound VBO it can find the size for each point
	
	
	//glPointSizePointerOES(GL_FLOAT,sizeof(PointSprite),(GLvoid*) (sizeof(GL_FLOAT)*2));
	
	
	// Change the blend function used if blendAdditive has been set
	
    // Set the blend function based on the configuration
    glBlendFunc(blendFuncSource, blendFuncDestination);
	
	// Enable and configure point sprites which we are going to use for our particles
	glEnable(GL_POINT_SPRITE);
	glTexEnvi( GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE );
	
	// Now that all of the VBOs have been used to configure the vertices, pointer size and color
	// use glDrawArrays to draw the points
	glDrawArrays(GL_POINTS, 0, particleIndex);
	
	// Unbind the current VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// Disable the client states which have been used incase the next draw function does 
	// not need or use them
	
	
	//glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
	
	
	glDisable(GL_POINT_SPRITE);
	
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// Re-enable the texture coordinates as we use them elsewhere in the game and it is expected that
	// its on
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void ofxParticleEmitter::drawPointsOES()
{
#ifdef TARGET_OF_IPHONE
	
	// Disable the texture coord array so that texture information is not copied over when rendering
	// the point sprites.
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	// Bind to the verticesID VBO and popuate it with the necessary vertex & color informaiton
	glBindBuffer(GL_ARRAY_BUFFER, verticesID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PointSprite) * maxParticles, vertices, GL_DYNAMIC_DRAW);
	
	// Configure the vertex pointer which will use the currently bound VBO for its data
	glVertexPointer(2, GL_FLOAT, sizeof(PointSprite), 0);
	glColorPointer(4,GL_FLOAT,sizeof(PointSprite),(GLvoid*) (sizeof(GLfloat)*3));
	
	// Bind to the particles texture
	glBindTexture(GL_TEXTURE_2D, (GLuint)textureData.textureID);
	
	// Enable the point size array
	glEnableClientState(GL_POINT_SIZE_ARRAY_OES);
	
	// Configure the point size pointer which will use the currently bound VBO.  PointSprite contains
	// both the location of the point as well as its size, so the config below tells the point size
	// pointer where in the currently bound VBO it can find the size for each point
	glPointSizePointerOES(GL_FLOAT,sizeof(PointSprite),(GLvoid*) (sizeof(GL_FLOAT)*2));
	
	// Change the blend function used if blendAdditive has been set
	
    // Set the blend function based on the configuration
    glBlendFunc(blendFuncSource, blendFuncDestination);
	
	// Enable and configure point sprites which we are going to use for our particles
	glEnable(GL_POINT_SPRITE_OES);
	glTexEnvi( GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE );
	
	// Now that all of the VBOs have been used to configure the vertices, pointer size and color
	// use glDrawArrays to draw the points
	glDrawArrays(GL_POINTS, 0, particleIndex);
	
	// Unbind the current VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	// Disable the client states which have been used incase the next draw function does 
	// not need or use them
	glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
	glDisable(GL_POINT_SPRITE_OES);
	
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// Re-enable the texture coordinates as we use them elsewhere in the game and it is expected that
	// its on
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
#endif
}

void ofxParticleEmitter::changeTexture(string path){
    texture = new ofImage();
    texture->load( path );
    texture->setUseTexture( true );
    texture->setAnchorPercent( 0.5f, 0.5f );
    
    textureData = texture->getTexture().getTextureData();
    textureName = path;
}


string ofxParticleEmitter::getTextureName(){
    return textureName;
}

