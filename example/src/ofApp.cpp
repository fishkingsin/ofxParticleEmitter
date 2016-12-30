#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetWindowTitle( "particle example" );
	ofBackground( 0, 0, 0 );
	ofSetFrameRate( 60 );
	
	if ( !m_emitter.loadFromXml( "circles.pex" ) )
	{
		ofLog( OF_LOG_ERROR, "ofApp::setup() - failed to load emitter config" );
	}
}

//--------------------------------------------------------------
void ofApp::exit()
{
	// TODO
}

//--------------------------------------------------------------
void ofApp::update()
{
	m_emitter.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
	m_emitter.draw( 0, 0 );
	
	ofSetColor( 255, 255, 255 );
	ofDrawBitmapString( "fps: " + ofToString( ofGetFrameRate(), 2 ), 20, 20 );
}


//--------------------------------------------------------------
void ofApp::keyPressed  (int key)
{
	// nothing
}

//--------------------------------------------------------------
void ofApp::keyReleased  (int key)
{
	// nothing
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y )
{
	// nothing
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
	m_emitter.sourcePosition.x = x;
	m_emitter.sourcePosition.y = y;
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
	// nothing
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
	// nothing
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
	// nothing
}
