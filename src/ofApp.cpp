#include "ofApp.h"

ofPlanePrimitive plane;

//--------------------------------------------------------------
void ofApp::setup(){
  // MARK: - Initialise socket connection
  isConnected = false;
  address = "http://localhost:3000";
  status = "not connected";
  socketIO.setup(address);
  ofAddListener(socketIO.notifyEvent, this, &ofApp::gotEvent);
  ofAddListener(socketIO.connectionEvent, this, &ofApp::onConnection);

  // MARK: - Initialise Kinect connection
  kinect.init();
  kinect.open();
	ofSetFrameRate(60);
	easyCam.enableMouseInput();
	pointCloud.setMode(OF_PRIMITIVE_POINTS);
	
	//ofSetWindowShape(640, 480);
	
  // MARK: - Initialise debug interface
  gui.setup();
  gui.add(kinectDistanceSlider.setup("Kinect distance", 2850, 80, 3000));
	gui.add(kinectXSlider.setup("Kinect Scale X", 1.6, -2, 2));
	gui.add(kinectYSlider.setup("Kinect Scale Y", -1.6, -2, 2));
	gui.add(kinectZSlider.setup("Kinect Scale Z", -1, -2, 2));
	gui.add(kinectAngleSlider.setup("Kinect Angle", 0, -1, 1));
	
	
	numPointsInRegion = 0;
	scaleFactorHoop = 200;
  
  // MARK: - Load textures & setup plane
  texture_plain.load("texture_plain.png");
  texture_glowing.load("texture_glowing.png");
  plane.set(hoopScale, hoopScale);
  plane.setPosition(xPos, yPos, kinectDistanceSlider-220);
}

void ofApp::update() {
  // MARK: - Generate pointcloud from Kinect data
    kinect.update();
		numPointsInRegion = 0;
    if(kinect.isFrameNewDepth()) {
      pointCloud.clear();
      for(int y = 0; y < kinect.height; y++) {
        for(int x= 0; x < kinect.width; x++) {
          int z = kinect.getDistanceAt(x, y);
          if(z > 0) {
            pointCloud.addColor(kinect.getColorAt(x, y));
            ofVec3f pt = kinect.getWorldCoordinateAt(x, y);
            pointCloud.addVertex(pt);
						
						int halfRegionSize = (hoopScale) / 2;
						if((pt.x > xPos-halfRegionSize && pt.x < xPos + halfRegionSize)
							 && (pt.y > yPos-halfRegionSize && pt.y < yPos + halfRegionSize)
							 && (pt.z > kinectDistanceSlider-halfRegionSize && pt.z < kinectDistanceSlider + halfRegionSize)){
							numPointsInRegion += 1;
          }
				}
			}
		}
	}
}

void ofApp::draw() {
  ofBackground(0);
  
  // MARK: - Display the pointcloud & start easyCam
	easyCam.begin();
	ofPushMatrix();
	ofScale(1, -1, kinectZSlider);
	ofTranslate(0, 0, -1000);
	
	ofEnableDepthTest();
	pointCloud.drawVertices();
	ofDisableDepthTest();
	
	if(numPointsInRegion > 100) {
		ofFill();
		ofLogNotice("DETECTED");
		std::string hoopHitEventName = "detectHit";
		std::string param = "param";
		socketIO.emit(hoopHitEventName, param);
	} else {
		ofNoFill();
	}
  
  texture_plain.bind();
  plane.draw();
  texture_plain.unbind();
  
	ofDrawBox(xPos, yPos, kinectDistanceSlider-220, hoopScale, hoopScale, 400);
	ofPopMatrix();
	
	easyCam.end();
	
  // MARK: - Draw debug interface
  if(debugMode) {
    gui.draw();
    kinect.setCameraTiltAngle(kinectAngleSlider);
  }
}

// MARK: - #BIND_ON_CONNECT
void ofApp::onConnection () {
  isConnected = true;
  bindEvents();
}

// MARK: - #BIND_EVENTS_TO_CALLBACKS
void ofApp::bindEvents () {
  std::string hoopPlacedEventName = "drawHoop";
  socketIO.bindEvent(hoopPlacedEvent, hoopPlacedEventName);
  ofAddListener(hoopPlacedEvent, this, &ofApp::drawHoop);
}

 // MARK: - #PROPAGATE_EVENT
void ofApp::gotEvent(string& name) {
  ofLogNotice("ofxSocketIO[gotEvent]", name);
  status = name;
}

// MARK: - #EVENTS
void ofApp::drawHoop (ofxSocketIOData& data) {
  // MARK: - Log data to console
  ofLogNotice("ofxSocketIO[pageX]", ofToString(data.getFloatValue("relX")));
  ofLogNotice("ofxSocketIO[pageY]", ofToString(data.getFloatValue("relY")));
  ofLogNotice("ofxSocketIO[scale]", ofToString(data.getFloatValue("scale")));

  // MARK: - Assign data values to global variables
  xPos = (data.getFloatValue("relX"))* ofGetWidth();
  yPos = (data.getFloatValue("relY"))* ofGetHeight();
  hoopScale = (data.getFloatValue("scale"))* scaleFactorHoop;
}
